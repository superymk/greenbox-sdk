/* SPDX-License-Identifier: GPL-2.0-only OR Proprietary */
#include "hid-report.h"
#include <string.h>

#define HID_GLOBAL_STACK_CAPACITY 16u
#define HID_COLLECTION_STACK_CAPACITY 32u
#define HID_LOCAL_USAGE_RANGE_CAPACITY 64u
/* Extended usages store the usage page in the upper 16 bits. */
#define HID_USAGE(usage_page, usage_id) (((uint32_t)(usage_page) << 16) | (usage_id))

/* Main-item data flags: clear bits mean Data, Array, and Absolute. */
#define MAIN_ITEM_CONSTANT 1u
#define MAIN_ITEM_VARIABLE 2u
#define MAIN_ITEM_RELATIVE 4u
#define MAIN_ITEM_NO_PREFERRED_STATE 32u
#define MAIN_ITEM_NULL_STATE 64u

enum hid_item_type
{
    HID_ITEM_MAIN = 0, HID_ITEM_GLOBAL = 1, HID_ITEM_LOCAL = 2
};

enum hid_main_item_tag
{
    HID_MAIN_INPUT = 8, HID_MAIN_OUTPUT = 9, HID_MAIN_COLLECTION = 10,
    HID_MAIN_FEATURE = 11, HID_MAIN_END_COLLECTION = 12
};

enum hid_global_item_tag
{
    HID_GLOBAL_USAGE_PAGE = 0, HID_GLOBAL_LOGICAL_MINIMUM = 1,
    HID_GLOBAL_LOGICAL_MAXIMUM = 2, HID_GLOBAL_PHYSICAL_MINIMUM = 3,
    HID_GLOBAL_PHYSICAL_MAXIMUM = 4, HID_GLOBAL_UNIT_EXPONENT = 5,
    HID_GLOBAL_UNIT = 6, HID_GLOBAL_REPORT_SIZE = 7, HID_GLOBAL_REPORT_ID = 8,
    HID_GLOBAL_REPORT_COUNT = 9, HID_GLOBAL_PUSH = 10, HID_GLOBAL_POP = 11
};

enum hid_local_item_tag
{
    HID_LOCAL_USAGE = 0, HID_LOCAL_USAGE_MINIMUM = 1, HID_LOCAL_USAGE_MAXIMUM = 2,
    HID_LOCAL_DESIGNATOR_INDEX = 3, HID_LOCAL_DESIGNATOR_MINIMUM = 4,
    HID_LOCAL_DESIGNATOR_MAXIMUM = 5, HID_LOCAL_STRING_INDEX = 7,
    HID_LOCAL_STRING_MINIMUM = 8, HID_LOCAL_STRING_MAXIMUM = 9,
    HID_LOCAL_DELIMITER = 10
};

static uint32_t read_item_unsigned_le(const uint8_t *item_bytes, unsigned byte_count)
{
    uint32_t value = 0;
    unsigned byte_index;

    FOREACH_S(byte_index, byte_count, sizeof(value), 0, 1)
    {
        value |= (uint32_t)item_bytes[byte_index] << (byte_index * 8u);
    }

    return value;
}

static int32_t sign_extend_bits(uint32_t raw_value, unsigned bit_count)
{
    if (bit_count && (raw_value & ((uint32_t)1 << (bit_count - 1u))))
    {
        return (int32_t)((int64_t)raw_value - ((int64_t)1 << bit_count));
    }

    return (int32_t)raw_value;
}

/* Arrays map selectors from Logical Minimum into this ordered usage list.
 * Variable items repeat the final usage when Report Count exceeds the list. */
static uint32_t lookup_usage(const uint32_t *first_usage, const uint32_t *last_usage,
                            unsigned usage_range_count, uint32_t usage_index,
                            int repeat_last_usage)
{
    unsigned range_index;

    FOREACH_S(range_index, usage_range_count, HID_REPORT_MAX_USAGE_RANGES, 0, 1)
    {
        uint32_t range_length = last_usage[range_index] - first_usage[range_index] + 1u;

        if (usage_index < range_length)
        {
            return first_usage[range_index] + usage_index;
        }

        usage_index -= range_length;
    }

    return repeat_last_usage && usage_range_count ? last_usage[usage_range_count - 1u] : 0;
}

static unsigned supported_usage_input_type(uint32_t usage, unsigned input_type)
{
    if (input_type == HID_REPORT_KEYBOARD && (usage >> 16) == 7 &&
        (usage & 0xffffu) <= 255u)
    {
        return HID_REPORT_KEYBOARD;
    }

    if (input_type == HID_REPORT_MOUSE &&
        ((usage >= HID_USAGE(9,1) && usage <= HID_USAGE(9,8)) ||
         usage == HID_USAGE(1,0x30) || usage == HID_USAGE(1,0x31) ||
         usage == HID_USAGE(1,0x38) || usage == HID_USAGE(0x0c,0x238)))
    {
        return HID_REPORT_MOUSE;
    }

    return 0;
}

static int find_or_add_report_index(struct hid_report_descriptor *descriptor, uint8_t report_id)
{
    unsigned report_index;

    FOREACH_S(report_index, descriptor->report_id_count, HID_REPORT_MAX_IDS, 0, 1)
    {
        if (descriptor->report_ids[report_index] == report_id)
        {
            return (int)report_index;
        }
    }

    if (descriptor->report_id_count == HID_REPORT_MAX_IDS)
    {
        return -1;
    }

    report_index = descriptor->report_id_count++;
    descriptor->report_ids[report_index] = report_id;

    return (int)report_index;
}

static int append_report_field(struct hid_report_descriptor *descriptor,
                               uint32_t element_size_bits, uint32_t element_count,
                               int32_t logical_minimum, int32_t logical_maximum,
                               const uint32_t *first_usage, const uint32_t *last_usage,
                               unsigned usage_range_count, unsigned report_index,
                               uint32_t report_bit_offset, unsigned input_type,
                               uint32_t main_item_flags, unsigned is_output_field)
{
    unsigned field_index;

    if (descriptor->field_count == HID_REPORT_MAX_FIELDS ||
        usage_range_count > HID_REPORT_MAX_USAGE_RANGES - descriptor->usage_range_count)
    {
        return HID_PARSE_UNSUPPORTED;
    }

    field_index = descriptor->field_count++;
    descriptor->field_bit_offset[field_index] = report_bit_offset;
    descriptor->field_logical_minimum[field_index] = logical_minimum;
    descriptor->field_logical_maximum[field_index] = logical_maximum;
    descriptor->field_element_count[field_index] = (uint16_t)element_count;
    descriptor->field_first_usage_range[field_index] = (uint16_t)descriptor->usage_range_count;
    descriptor->field_usage_range_count[field_index] = (uint16_t)usage_range_count;
    descriptor->field_main_item_flags[field_index] = (uint16_t)main_item_flags;
    descriptor->field_element_size_bits[field_index] = (uint8_t)element_size_bits;
    descriptor->field_report_index[field_index] = (uint8_t)report_index;
    descriptor->field_input_type[field_index] = (uint8_t)input_type;
    descriptor->field_is_output[field_index] = (uint8_t)is_output_field;

    memcpy(&descriptor->first_usage[descriptor->usage_range_count], first_usage,
           usage_range_count * sizeof(first_usage[0]));
    memcpy(&descriptor->last_usage[descriptor->usage_range_count], last_usage,
           usage_range_count * sizeof(last_usage[0]));
    descriptor->usage_range_count += usage_range_count;

    return HID_PARSE_OK;
}

static int parse_data_main_item(struct hid_report_descriptor *descriptor,
                                uint32_t element_size_bits, uint32_t element_count,
                                int32_t logical_minimum, uint32_t logical_maximum_raw,
                                unsigned logical_maximum_size_bytes, uint8_t report_id,
                                const uint32_t *first_usage, const uint32_t *last_usage,
                                unsigned usage_range_count, unsigned input_type,
                                unsigned main_item_tag, uint32_t main_item_flags)
{
    uint32_t *report_size_bits, report_bit_offset, max_report_bits, usage_index;
    int report_index, encoding_supported = 1, has_supported_usage = 0, only_led_fields = 1;
    int64_t logical_maximum;

    if (!element_size_bits || !element_count)
    {
        return HID_PARSE_INVALID;
    }

    /* Count every Input/Output/Feature, including constant and vendor data. */
    max_report_bits = (HID_REPORT_MAX_PACKET - (descriptor->has_report_ids ? 1u : 0u)) * 8u;
    if (element_size_bits > max_report_bits || element_count > max_report_bits / element_size_bits)
    {
        return HID_PARSE_INVALID;
    }

    if (descriptor->has_report_ids && !report_id)
    {
        return HID_PARSE_INVALID;
    }

    report_index = find_or_add_report_index(descriptor, report_id);
    if (report_index < 0)
    {
        return HID_PARSE_UNSUPPORTED;
    }

    report_size_bits = main_item_tag == HID_MAIN_INPUT ? &descriptor->input_size_bits[report_index] :
                       main_item_tag == HID_MAIN_OUTPUT ? &descriptor->output_size_bits[report_index] :
                       &descriptor->feature_size_bits[report_index];
    if (*report_size_bits > max_report_bits - element_size_bits * element_count)
    {
        return HID_PARSE_INVALID;
    }

    report_bit_offset = *report_size_bits;
    *report_size_bits += element_size_bits * element_count;
    if (main_item_tag == HID_MAIN_FEATURE || (main_item_flags & MAIN_ITEM_CONSTANT))
    {
        return HID_PARSE_OK;
    }

    logical_maximum = logical_minimum < 0 ?
        sign_extend_bits(logical_maximum_raw, logical_maximum_size_bytes * 8u) :
        (int64_t)logical_maximum_raw;
    if (logical_maximum < logical_minimum)
    {
        return HID_PARSE_INVALID;
    }

    if (element_size_bits > 32u || logical_maximum > INT32_MAX ||
        (main_item_flags & ~(MAIN_ITEM_VARIABLE | MAIN_ITEM_RELATIVE |
                             MAIN_ITEM_NULL_STATE | MAIN_ITEM_NO_PREFERRED_STATE)))
    {
        encoding_supported = 0;
    }

    if (!(main_item_flags & MAIN_ITEM_VARIABLE))
    {
        /* Only keyboard selector arrays are implemented. */

        FOREACH_S(usage_index, usage_range_count, HID_LOCAL_USAGE_RANGE_CAPACITY, 0, 1)
        {
            if (supported_usage_input_type(first_usage[usage_index], input_type) ||
                supported_usage_input_type(last_usage[usage_index], input_type))
            {
                has_supported_usage = 1;
            }
        }

        if (input_type != HID_REPORT_KEYBOARD || (main_item_flags & MAIN_ITEM_RELATIVE))
        {
            encoding_supported = 0;
        }

        only_led_fields = 0;
    }
    else
    {
        FOREACH_S(usage_index, element_count, HID_REPORT_MAX_PACKET * 8u, 0, 1)
        {
            uint32_t usage = lookup_usage(first_usage, last_usage, usage_range_count, usage_index, 1);

            if (supported_usage_input_type(usage, input_type))
            {
                has_supported_usage = 1;
                if (input_type == HID_REPORT_MOUSE && (usage >> 16) != 9)
                {
                    if (!(main_item_flags & MAIN_ITEM_RELATIVE))
                    {
                        encoding_supported = 0;
                    }
                }
                else if (logical_minimum != 0 || logical_maximum != 1 ||
                           (main_item_flags & MAIN_ITEM_RELATIVE))
                {
                    encoding_supported = 0;
                }
            }

            if (input_type != HID_REPORT_KEYBOARD || usage < HID_USAGE(8,1) ||
                usage > HID_USAGE(8,5) || logical_minimum != 0 || logical_maximum != 1 ||
                (main_item_flags & MAIN_ITEM_RELATIVE))
            {
                only_led_fields = 0;
            }
        }
    }

    if (main_item_tag == HID_MAIN_OUTPUT)
    {
        if (!only_led_fields || !encoding_supported)
        {
            descriptor->has_unsupported_output_fields[report_index] = 1;

            return HID_PARSE_OK;
        }

        descriptor->has_led_fields[report_index] = 1;

        return append_report_field(descriptor, element_size_bits, element_count,
            logical_minimum, (int32_t)logical_maximum, first_usage, last_usage,
            usage_range_count, (unsigned)report_index, report_bit_offset,
            input_type, main_item_flags, 1);
    }

    if (!has_supported_usage)
    {
        return HID_PARSE_OK;
    }

    if (!encoding_supported)
    {
        return HID_PARSE_UNSUPPORTED;
    }

    descriptor->report_input_types[report_index] |= input_type;
    descriptor->input_types |= input_type;

    return append_report_field(descriptor, element_size_bits, element_count,
        logical_minimum, (int32_t)logical_maximum, first_usage, last_usage,
        usage_range_count, (unsigned)report_index, report_bit_offset,
        input_type, main_item_flags, 0);
}

int hid_report_parse(const uint8_t *descriptor, size_t descriptor_size_bytes,
                     struct hid_report_descriptor *parsed_descriptor)
{
    /* Index global_depth is the active set; Push copies each global item. */
    uint32_t usage_page_stack[HID_GLOBAL_STACK_CAPACITY + 1u] = {0};
    uint32_t element_size_stack[HID_GLOBAL_STACK_CAPACITY + 1u] = {0};
    uint32_t element_count_stack[HID_GLOBAL_STACK_CAPACITY + 1u] = {0};
    int32_t logical_minimum_stack[HID_GLOBAL_STACK_CAPACITY + 1u] = {0};
    uint32_t logical_maximum_stack[HID_GLOBAL_STACK_CAPACITY + 1u] = {0};
    unsigned logical_maximum_size_stack[HID_GLOBAL_STACK_CAPACITY + 1u] = {0};
    uint8_t report_id_stack[HID_GLOBAL_STACK_CAPACITY + 1u] = {0};
    uint32_t first_usage[HID_LOCAL_USAGE_RANGE_CAPACITY], last_usage[HID_LOCAL_USAGE_RANGE_CAPACITY];
    unsigned usage_range_count = 0, awaiting_usage_maximum = 0;
    unsigned delimiter_set_open = 0, has_usage_aliases = 0;
    unsigned collection_input_types[HID_COLLECTION_STACK_CAPACITY];
    unsigned collection_inside_application[HID_COLLECTION_STACK_CAPACITY];
    unsigned application_input_type = 0, inside_application_collection = 0;
    unsigned global_depth = 0, collection_depth = 0, has_unsupported_items = 0;
    size_t descriptor_offset = 0;

    if (!parsed_descriptor)
    {
        return HID_PARSE_INVALID;
    }

    memset(parsed_descriptor, 0, sizeof(*parsed_descriptor));
    if (!descriptor || !descriptor_size_bytes || descriptor_size_bytes > HID_REPORT_MAX_DESCRIPTOR)
    {
        return HID_PARSE_INVALID;
    }

    while (descriptor_offset < descriptor_size_bytes)
    {
        unsigned item_prefix = descriptor[descriptor_offset++], item_size_bytes = item_prefix & 3u;
        unsigned item_type = (item_prefix >> 2) & 3u, item_tag = item_prefix >> 4;
        uint32_t item_value;
        int parse_result;

        if (item_prefix == 0xfe)
        {
            if (descriptor_size_bytes - descriptor_offset < 2 || (size_t)descriptor[descriptor_offset] > descriptor_size_bytes - descriptor_offset - 2)
            {
                return HID_PARSE_INVALID;
            }

            descriptor_offset += (size_t)descriptor[descriptor_offset] + 2;
            has_unsupported_items = 1;
            continue;
        }

        if (item_size_bytes == 3)
        {
            item_size_bytes = 4;
        }

        if (descriptor_size_bytes - descriptor_offset < item_size_bytes)
        {
            return HID_PARSE_INVALID;
        }

        item_value = read_item_unsigned_le(&descriptor[descriptor_offset], item_size_bytes);
        descriptor_offset += item_size_bytes;

        if (item_type == HID_ITEM_GLOBAL)
        {
            if (item_tag == HID_GLOBAL_PUSH || item_tag == HID_GLOBAL_POP)
            {
                if (item_size_bytes)
                {
                    return HID_PARSE_INVALID;
                }

                if (item_tag == HID_GLOBAL_PUSH)
                {
                    if (global_depth == HID_GLOBAL_STACK_CAPACITY)
                    {
                        return HID_PARSE_INVALID;
                    }

                    usage_page_stack[global_depth + 1u] = usage_page_stack[global_depth];
                    element_size_stack[global_depth + 1u] = element_size_stack[global_depth];
                    element_count_stack[global_depth + 1u] = element_count_stack[global_depth];
                    logical_minimum_stack[global_depth + 1u] = logical_minimum_stack[global_depth];
                    logical_maximum_stack[global_depth + 1u] = logical_maximum_stack[global_depth];
                    logical_maximum_size_stack[global_depth + 1u] = logical_maximum_size_stack[global_depth];
                    report_id_stack[global_depth + 1u] = report_id_stack[global_depth];
                    ++global_depth;
                }
                else
                {
                    if (!global_depth)
                    {
                        return HID_PARSE_INVALID;
                    }

                    --global_depth;
                }

                continue;
            }

            if (!item_size_bytes)
            {
                return HID_PARSE_INVALID;
            }

            switch (item_tag)
            {
            case HID_GLOBAL_USAGE_PAGE:
                if (item_value > 0xffffu)
                {
                    return HID_PARSE_INVALID;
                }

                usage_page_stack[global_depth] = item_value;
                break;
            case HID_GLOBAL_LOGICAL_MINIMUM:
                logical_minimum_stack[global_depth] = sign_extend_bits(item_value, item_size_bytes * 8u);
                break;
            case HID_GLOBAL_LOGICAL_MAXIMUM:
                logical_maximum_stack[global_depth] = item_value;
                logical_maximum_size_stack[global_depth] = item_size_bytes;
                break;
            case HID_GLOBAL_PHYSICAL_MINIMUM:
            case HID_GLOBAL_PHYSICAL_MAXIMUM:
            case HID_GLOBAL_UNIT_EXPONENT:
            case HID_GLOBAL_UNIT:
                /* Physical-unit metadata does not change the encoded bit layout. */
                break;
            case HID_GLOBAL_REPORT_SIZE:
                element_size_stack[global_depth] = item_value;
                break;
            case HID_GLOBAL_REPORT_ID:
                if (item_size_bytes != 1 || !item_value || item_value > 255u)
                {
                    return HID_PARSE_INVALID;
                }

                if (!parsed_descriptor->has_report_ids && parsed_descriptor->report_id_count)
                {
                    return HID_PARSE_INVALID;
                }

                parsed_descriptor->has_report_ids = 1;
                report_id_stack[global_depth] = (uint8_t)item_value;
                break;
            case HID_GLOBAL_REPORT_COUNT:
                element_count_stack[global_depth] = item_value;
                break;
            default:
                has_unsupported_items = 1;
                break;
            }
        }
        else if (item_type == HID_ITEM_LOCAL)
        {
            if (!item_size_bytes)
            {
                return HID_PARSE_INVALID;
            }

            if (item_tag <= HID_LOCAL_USAGE_MAXIMUM)
            {
                uint32_t usage = item_size_bytes == 4 ? item_value : HID_USAGE(usage_page_stack[global_depth],item_value);

                if (item_tag == HID_LOCAL_USAGE_MAXIMUM)
                {
                    if (!awaiting_usage_maximum)
                    {
                        return HID_PARSE_INVALID;
                    }

                    if ((usage >> 16) != (first_usage[usage_range_count - 1u] >> 16) || usage < first_usage[usage_range_count - 1u])
                    {
                        return HID_PARSE_INVALID;
                    }

                    last_usage[usage_range_count - 1u] = usage;
                    awaiting_usage_maximum = 0;
                }
                else
                {
                    if (awaiting_usage_maximum)
                    {
                        return HID_PARSE_INVALID;
                    }

                    if (usage_range_count == HID_LOCAL_USAGE_RANGE_CAPACITY)
                    {
                        /* Stop safely without allowing an unchecked suffix
                         * to turn a malformed descriptor into BOOT fallback. */
                        return HID_PARSE_INVALID;
                    }

                    first_usage[usage_range_count] = usage;
                    last_usage[usage_range_count++] = usage;
                    awaiting_usage_maximum = item_tag == HID_LOCAL_USAGE_MINIMUM;
                }
            }
            else if (item_tag == HID_LOCAL_DELIMITER)
            {
                if (item_value > 1 || (item_value && delimiter_set_open) ||
                    (!item_value && !delimiter_set_open) || awaiting_usage_maximum)
                {
                    return HID_PARSE_INVALID;
                }

                delimiter_set_open = item_value;
                has_usage_aliases = 1;

                /* Syntax is checked, but selecting among usage aliases is unsupported. */
                has_unsupported_items = 1;
            }
            else if (item_tag != HID_LOCAL_DESIGNATOR_INDEX &&
                       item_tag != HID_LOCAL_DESIGNATOR_MINIMUM &&
                       item_tag != HID_LOCAL_DESIGNATOR_MAXIMUM &&
                       item_tag != HID_LOCAL_STRING_INDEX &&
                       item_tag != HID_LOCAL_STRING_MINIMUM &&
                       item_tag != HID_LOCAL_STRING_MAXIMUM)
            {
                has_unsupported_items = 1;
            }
        }
        else if (item_type == HID_ITEM_MAIN)
        {
            if (awaiting_usage_maximum || delimiter_set_open)
            {
                return HID_PARSE_INVALID;
            }

            /* HID 1.11 section 6.2.2.8 forbids aliases on Application
             * collections and Array items, even when sets are balanced. */
            if (has_usage_aliases &&
                ((item_tag == HID_MAIN_COLLECTION && item_value == 1) ||
                 ((item_tag == HID_MAIN_INPUT || item_tag == HID_MAIN_OUTPUT || item_tag == HID_MAIN_FEATURE) &&
                  !(item_value & MAIN_ITEM_VARIABLE))))
            {
                return HID_PARSE_INVALID;
            }

            if (item_tag == HID_MAIN_COLLECTION)
            {
                if (item_size_bytes != 1 || collection_depth == HID_COLLECTION_STACK_CAPACITY)
                {
                    return HID_PARSE_INVALID;
                }

                collection_input_types[collection_depth] = application_input_type;
                collection_inside_application[collection_depth++] = inside_application_collection;
                if (item_value == 1 && !inside_application_collection)
                {
                    uint32_t usage = lookup_usage(first_usage, last_usage, usage_range_count, 0, 0);

                    inside_application_collection = 1;
                    application_input_type =
                        usage == HID_USAGE(1,6) ? HID_REPORT_KEYBOARD :
                        usage == HID_USAGE(1,2) ? HID_REPORT_MOUSE : 0;
                }
            }
            else if (item_tag == HID_MAIN_END_COLLECTION)
            {
                if (item_size_bytes || !collection_depth)
                {
                    return HID_PARSE_INVALID;
                }

                --collection_depth;
                application_input_type = collection_input_types[collection_depth];
                inside_application_collection = collection_inside_application[collection_depth];
            }
            else if (item_tag == HID_MAIN_INPUT || item_tag == HID_MAIN_OUTPUT || item_tag == HID_MAIN_FEATURE)
            {
                if (!item_size_bytes)
                {
                    return HID_PARSE_INVALID;
                }

                parse_result = parse_data_main_item(parsed_descriptor,
                    element_size_stack[global_depth], element_count_stack[global_depth],
                    logical_minimum_stack[global_depth], logical_maximum_stack[global_depth],
                    logical_maximum_size_stack[global_depth], report_id_stack[global_depth],
                    first_usage, last_usage, usage_range_count, application_input_type,
                    item_tag, item_value);
                if (parse_result == HID_PARSE_INVALID)
                {
                    return parse_result;
                }

                if (parse_result == HID_PARSE_UNSUPPORTED)
                {
                    has_unsupported_items = 1;
                }
            }
            else
            {
                has_unsupported_items = 1;
            }

            usage_range_count = 0;
            awaiting_usage_maximum = 0;
            delimiter_set_open = 0;
            has_usage_aliases = 0;
        }
        else
        {
            has_unsupported_items = 1;
        }
    }

    if (collection_depth || global_depth || awaiting_usage_maximum || delimiter_set_open)
    {
        return HID_PARSE_INVALID;
    }

    if (has_unsupported_items || !parsed_descriptor->input_types)
    {
        return HID_PARSE_UNSUPPORTED;
    }

    {
        unsigned report_index;

        FOREACH_S(report_index, parsed_descriptor->report_id_count, HID_REPORT_MAX_IDS, 0, 1)
        {
            size_t input_size_bytes = (parsed_descriptor->input_size_bits[report_index] + 7u) / 8u;

            if (input_size_bytes && parsed_descriptor->has_report_ids)
            {
                ++input_size_bytes;
            }

            if (input_size_bytes > parsed_descriptor->max_input_bytes)
            {
                parsed_descriptor->max_input_bytes = input_size_bytes;
            }
        }
    }

    return HID_PARSE_OK;
}

static uint32_t read_report_bits(const uint8_t *report_bytes, uint32_t report_bit_offset, unsigned bit_count)
{
    uint32_t value = 0;
    unsigned bit_index;

    FOREACH_S(bit_index, bit_count, sizeof(value) * 8u, 0, 1)
    {
        value |= (uint32_t)((report_bytes[(report_bit_offset + bit_index) / 8u] >> ((report_bit_offset + bit_index) % 8u)) & 1u) << bit_index;
    }

    return value;
}

static int accumulate_axis_delta(int32_t *axis_delta, int32_t element_delta)
{
    int64_t combined_delta = (int64_t)*axis_delta + element_delta;

    if (combined_delta < INT32_MIN || combined_delta > INT32_MAX)
    {
        return HID_PARSE_INVALID;
    }

    *axis_delta = (int32_t)combined_delta;

    return HID_PARSE_OK;
}

/* Verify the descriptor field before any packet access. */
static int validate_report_field(const struct hid_report_descriptor *descriptor,
                                 unsigned field_index, uint32_t report_size_bits)
{
    unsigned element_size_bits = descriptor->field_element_size_bits[field_index];
    unsigned first_range = descriptor->field_first_usage_range[field_index];

    if (!element_size_bits || element_size_bits > 32u ||
        descriptor->field_bit_offset[field_index] > report_size_bits ||
        descriptor->field_element_count[field_index] >
            (report_size_bits - descriptor->field_bit_offset[field_index]) / element_size_bits ||
        first_range > descriptor->usage_range_count ||
        descriptor->field_usage_range_count[field_index] > descriptor->usage_range_count - first_range ||
        descriptor->field_logical_minimum[field_index] > descriptor->field_logical_maximum[field_index])
    {
        return HID_PARSE_INVALID;
    }

    return HID_PARSE_OK;
}

static int decode_input_report(const struct hid_report_descriptor *descriptor,
                                const uint8_t *report_bytes, size_t report_size_bytes,
                                unsigned input_type, uint8_t pressed_keys[32],
                                uint8_t *pressed_buttons, uint8_t *button_mask,
                                int32_t relative_axes[HID_MOUSE_AXIS_COUNT],
                                unsigned *decoded_report_index)
{
    uint8_t keys[32] = {0}, buttons = 0, supplied_buttons = 0;
    int32_t axes[HID_MOUSE_AXIS_COUNT] = {0};
    unsigned report_index, field_index;
    uint8_t report_id = 0;
    uint32_t input_size_bits;

    if (!descriptor || !report_bytes || !report_size_bytes || report_size_bytes > HID_REPORT_MAX_PACKET ||
        !descriptor->input_types || descriptor->report_id_count > HID_REPORT_MAX_IDS ||
        descriptor->field_count > HID_REPORT_MAX_FIELDS ||
        descriptor->usage_range_count > HID_REPORT_MAX_USAGE_RANGES)
    {
        return HID_PARSE_INVALID;
    }

    if (descriptor->has_report_ids)
    {
        report_id = *report_bytes++;
        --report_size_bytes;
    }

    FOREACH_S(report_index, descriptor->report_id_count, HID_REPORT_MAX_IDS, 0, 1)
    {
        if (descriptor->report_ids[report_index] == report_id)
        {
            break;
        }
    }

    if (report_index == descriptor->report_id_count)
    {
        return HID_PARSE_UNSUPPORTED;
    }

    input_size_bits = descriptor->input_size_bits[report_index];
    if (input_size_bits > (HID_REPORT_MAX_PACKET - (descriptor->has_report_ids ? 1u : 0u)) * 8u ||
        report_size_bytes < (input_size_bits + 7u) / 8u)
    {
        return HID_PARSE_INVALID;
    }

    if (!(descriptor->report_input_types[report_index] & input_type))
    {
        return HID_PARSE_UNSUPPORTED;
    }

    FOREACH_S(field_index, descriptor->field_count, HID_REPORT_MAX_FIELDS, 0, 1)
    {
        unsigned element_index, first_range, range_count, element_size_bits, main_item_flags;
        int32_t logical_minimum, logical_maximum;

        if (descriptor->field_is_output[field_index] ||
            descriptor->field_report_index[field_index] != report_index ||
            descriptor->field_input_type[field_index] != input_type)
        {
            continue;
        }

        if (validate_report_field(descriptor, field_index, input_size_bits) != HID_PARSE_OK)
        {
            return HID_PARSE_INVALID;
        }

        first_range = descriptor->field_first_usage_range[field_index];
        range_count = descriptor->field_usage_range_count[field_index];
        element_size_bits = descriptor->field_element_size_bits[field_index];
        main_item_flags = descriptor->field_main_item_flags[field_index];
        logical_minimum = descriptor->field_logical_minimum[field_index];
        logical_maximum = descriptor->field_logical_maximum[field_index];

        FOREACH_S(element_index, descriptor->field_element_count[field_index], HID_REPORT_MAX_PACKET * 8u, 0, 1)
        {
            uint32_t raw_value, usage = 0;
            int64_t element_value;

            if (main_item_flags & MAIN_ITEM_VARIABLE)
            {
                usage = lookup_usage(&descriptor->first_usage[first_range],
                    &descriptor->last_usage[first_range], range_count, element_index, 1);

                /* Unknown controls share offsets, but their values are not
                 * interpreted or validated as keyboard/mouse controls. */
                if (!supported_usage_input_type(usage, input_type))
                {
                    continue;
                }
            }

            raw_value = read_report_bits(report_bytes,
                descriptor->field_bit_offset[field_index] + element_index * element_size_bits,
                element_size_bits);
            element_value = logical_minimum < 0 ?
                sign_extend_bits(raw_value, element_size_bits) : (int64_t)raw_value;
            if (element_value < logical_minimum || element_value > logical_maximum)
            {
                if (!(main_item_flags & MAIN_ITEM_VARIABLE) || (main_item_flags & MAIN_ITEM_NULL_STATE))
                {
                    continue;
                }

                return HID_PARSE_INVALID;
            }

            if (!(main_item_flags & MAIN_ITEM_VARIABLE))
            {
                usage = lookup_usage(&descriptor->first_usage[first_range],
                    &descriptor->last_usage[first_range], range_count,
                    (uint32_t)(element_value - logical_minimum), 0);
            }

            if (!supported_usage_input_type(usage, input_type))
            {
                continue;
            }

            if (input_type == HID_REPORT_KEYBOARD)
            {
                unsigned key_usage = usage & 0xffffu;

                if ((main_item_flags & MAIN_ITEM_VARIABLE) && !element_value)
                {
                    continue;
                }

                if (key_usage >= 1 && key_usage <= 3)
                {
                    return HID_PARSE_INVALID; /* ErrorRollOver/POSTFail/ErrorUndefined. */
                }

                if (key_usage)
                {
                    keys[key_usage / 8u] |= (uint8_t)(1u << (key_usage % 8u));
                }
            }
            else if ((usage >> 16) == 9)
            {
                unsigned button_index = (usage & 0xffffu) - 1u;

                supplied_buttons |= (uint8_t)(1u << button_index);
                if (element_value)
                {
                    buttons |= (uint8_t)(1u << button_index);
                }
            }
            else
            {
                unsigned axis_index = usage == HID_USAGE(1,0x30) ? HID_MOUSE_AXIS_X :
                    usage == HID_USAGE(1,0x31) ? HID_MOUSE_AXIS_Y :
                    usage == HID_USAGE(1,0x38) ? HID_MOUSE_AXIS_WHEEL : HID_MOUSE_AXIS_PAN;

                if (accumulate_axis_delta(&axes[axis_index], (int32_t)element_value) != HID_PARSE_OK)
                {
                    return HID_PARSE_INVALID;
                }
            }
        }
    }

    if (input_type == HID_REPORT_KEYBOARD)
    {
        memcpy(pressed_keys, keys, sizeof(keys));
    }
    else
    {
        *pressed_buttons = buttons;
        *button_mask = supplied_buttons;
        memcpy(relative_axes, axes, sizeof(axes));
    }

    *decoded_report_index = report_index;

    return HID_PARSE_OK;
}

int hid_report_decode_keyboard(const struct hid_report_descriptor *descriptor,
                               const uint8_t *report_bytes, size_t report_size_bytes,
                               uint8_t pressed_keys[32], unsigned *report_index)
{
    if (!pressed_keys || !report_index)
    {
        return HID_PARSE_INVALID;
    }

    return decode_input_report(descriptor, report_bytes, report_size_bytes,
        HID_REPORT_KEYBOARD, pressed_keys, NULL, NULL, NULL, report_index);
}

int hid_report_decode_mouse(const struct hid_report_descriptor *descriptor,
                            const uint8_t *report_bytes, size_t report_size_bytes,
                            uint8_t *pressed_buttons, uint8_t *button_mask,
                            int32_t relative_axes[HID_MOUSE_AXIS_COUNT],
                            unsigned *report_index)
{
    if (!pressed_buttons || !button_mask || !relative_axes || !report_index)
    {
        return HID_PARSE_INVALID;
    }

    return decode_input_report(descriptor, report_bytes, report_size_bytes,
        HID_REPORT_MOUSE, NULL, pressed_buttons, button_mask, relative_axes, report_index);
}

int hid_report_encode_leds(const struct hid_report_descriptor *descriptor,
                           uint8_t led_states, unsigned report_index,
                           uint8_t *report_buffer, size_t buffer_capacity,
                           size_t *encoded_size_bytes, uint8_t *encoded_report_id)
{
    uint8_t *report_payload;
    size_t report_size_bytes;
    uint32_t output_size_bits;
    unsigned field_index;

    if (!descriptor || !report_buffer || !encoded_size_bytes || !encoded_report_id ||
        descriptor->report_id_count > HID_REPORT_MAX_IDS ||
        descriptor->field_count > HID_REPORT_MAX_FIELDS ||
        descriptor->usage_range_count > HID_REPORT_MAX_USAGE_RANGES)
    {
        return HID_PARSE_INVALID;
    }

    if (report_index >= descriptor->report_id_count)
    {
        return HID_PARSE_UNSUPPORTED;
    }

    if (!descriptor->has_led_fields[report_index] || descriptor->has_unsupported_output_fields[report_index])
    {
        return HID_PARSE_UNSUPPORTED;
    }

    output_size_bits = descriptor->output_size_bits[report_index];
    if (output_size_bits > (HID_REPORT_MAX_PACKET - (descriptor->has_report_ids ? 1u : 0u)) * 8u)
    {
        return HID_PARSE_INVALID;
    }

    report_size_bytes = (output_size_bits + 7u) / 8u + (descriptor->has_report_ids ? 1u : 0u);
    if (report_size_bytes > buffer_capacity)
    {
        return HID_PARSE_INVALID;
    }

    FOREACH_S(field_index, descriptor->field_count, HID_REPORT_MAX_FIELDS, 0, 1)
    {
        if (descriptor->field_is_output[field_index] &&
            descriptor->field_report_index[field_index] == report_index &&
            validate_report_field(descriptor, field_index, output_size_bits) != HID_PARSE_OK)
        {
            return HID_PARSE_INVALID;
        }
    }

    memset(report_buffer, 0, report_size_bytes);
    report_payload = report_buffer;

    if (descriptor->has_report_ids)
    {
        *report_payload++ = descriptor->report_ids[report_index];
    }

    FOREACH_S(field_index, descriptor->field_count, HID_REPORT_MAX_FIELDS, 0, 1)
    {
        unsigned element_index, first_range;

        if (!descriptor->field_is_output[field_index] || descriptor->field_report_index[field_index] != report_index)
        {
            continue;
        }

        first_range = descriptor->field_first_usage_range[field_index];

        FOREACH_S(element_index, descriptor->field_element_count[field_index], HID_REPORT_MAX_PACKET * 8u, 0, 1)
        {
            uint32_t usage = lookup_usage(&descriptor->first_usage[first_range],
                &descriptor->last_usage[first_range], descriptor->field_usage_range_count[field_index],
                element_index, 1);

            if (usage >= HID_USAGE(8,1) && usage <= HID_USAGE(8,5) &&
                (led_states & (1u << ((usage & 0xffffu) - 1u))))
            {
                uint32_t report_bit_offset = descriptor->field_bit_offset[field_index] +
                    element_index * descriptor->field_element_size_bits[field_index];

                report_payload[report_bit_offset / 8u] |= (uint8_t)(1u << (report_bit_offset % 8u));
            }
        }
    }

    *encoded_size_bytes = report_size_bytes;
    *encoded_report_id = descriptor->report_ids[report_index];

    return HID_PARSE_OK;
}
