/* SPDX-License-Identifier: GPL-2.0-only OR Proprietary */
#ifndef HID_REPORT_H
#define HID_REPORT_H

#include <stddef.h>
#include <stdint.h>

/* Driver builds use the SDK's secure iteration macro. Keep the parser usable
 * by standalone callers that do not include the SDK headers. */
#ifndef FOREACH_S
#define FOREACH_S(var_name, m1, m2, initial, step) \
    for (var_name = (initial); (var_name < (m1)) && (var_name < (m2)); var_name += (step))
#endif

#define HID_REPORT_MAX_DESCRIPTOR 4096u
#define HID_REPORT_MAX_PACKET 1024u
#define HID_REPORT_MAX_IDS 16u
#define HID_REPORT_MAX_FIELDS 128u
#define HID_REPORT_MAX_USAGE_RANGES 512u

#define HID_REPORT_KEYBOARD 1u
#define HID_REPORT_MOUSE 2u
#define HID_MOUSE_AXIS_X 0u
#define HID_MOUSE_AXIS_Y 1u
#define HID_MOUSE_AXIS_WHEEL 2u
#define HID_MOUSE_AXIS_PAN 3u
#define HID_MOUSE_AXIS_COUNT 4u

enum hid_parse_result
{
    HID_PARSE_OK = 0,
    HID_PARSE_UNSUPPORTED = 1,
    HID_PARSE_INVALID = 2
};

/* Parsed descriptor data only. Each group of parallel arrays is indexed by
 * report, field, or usage-range index; no transfer or input state is retained. */
struct hid_report_descriptor
{
    /* HID_REPORT_KEYBOARD/HID_REPORT_MOUSE flags across all Input reports. */
    unsigned input_types;
    /* Largest complete Input report in bytes, including an optional ID byte. */
    size_t max_input_bytes;
    unsigned report_id_count;
    unsigned field_count;
    unsigned usage_range_count;
    /* Boolean: every report begins with a report-ID byte when nonzero. */
    unsigned has_report_ids;

    /* Report arrays share report_index in [0, report_id_count). ID 0 denotes
     * an unnumbered report. Input/Output/Feature bit counts exclude the ID byte. */
    uint8_t report_ids[HID_REPORT_MAX_IDS];
    uint32_t input_size_bits[HID_REPORT_MAX_IDS];
    uint32_t output_size_bits[HID_REPORT_MAX_IDS];
    uint32_t feature_size_bits[HID_REPORT_MAX_IDS];
    /* HID_REPORT_KEYBOARD/HID_REPORT_MOUSE flags for each Input report. */
    unsigned report_input_types[HID_REPORT_MAX_IDS];
    /* Boolean per report: supported LED fields are present when nonzero. */
    uint8_t has_led_fields[HID_REPORT_MAX_IDS];
    /* Boolean per report: nonzero prohibits LED writes because other output
     * controls or an unsupported output encoding would be overwritten. */
    uint8_t has_unsupported_output_fields[HID_REPORT_MAX_IDS];

    /* Field arrays share field_index in [0, field_count). Each field describes
     * one retained Input/Output Main item. Payload offsets exclude the ID byte. */
    uint32_t field_bit_offset[HID_REPORT_MAX_FIELDS];
    int32_t field_logical_minimum[HID_REPORT_MAX_FIELDS];
    int32_t field_logical_maximum[HID_REPORT_MAX_FIELDS];
    uint16_t field_element_count[HID_REPORT_MAX_FIELDS];
    /* First index and count of this field's slice of the usage-range arrays. */
    uint16_t field_first_usage_range[HID_REPORT_MAX_FIELDS];
    uint16_t field_usage_range_count[HID_REPORT_MAX_FIELDS];
    /* HID Main-item flags, including Variable, Relative, and Null State. */
    uint16_t field_main_item_flags[HID_REPORT_MAX_FIELDS];
    uint8_t field_element_size_bits[HID_REPORT_MAX_FIELDS];
    uint8_t field_report_index[HID_REPORT_MAX_FIELDS];
    /* Exactly one selected bit: HID_REPORT_KEYBOARD or HID_REPORT_MOUSE. */
    uint8_t field_input_type[HID_REPORT_MAX_FIELDS];
    /* Boolean: nonzero identifies Output; zero identifies Input. */
    uint8_t field_is_output[HID_REPORT_MAX_FIELDS];

    /* Usage arrays share range_index in [0, usage_range_count). Inclusive
     * extended usages hold the page in upper 16 bits and usage in lower 16. */
    uint32_t first_usage[HID_REPORT_MAX_USAGE_RANGES];
    uint32_t last_usage[HID_REPORT_MAX_USAGE_RANGES];
};

/* Use parsed_descriptor only after HID_PARSE_OK. No allocation, I/O, or
 * pointer into descriptor_bytes is retained. */
int hid_report_parse(const uint8_t *descriptor_bytes, size_t descriptor_size_bytes,
                     struct hid_report_descriptor *parsed_descriptor);

/* Reports include their optional ID byte. Each successful call supplies one
 * report's complete state and its descriptor index. All outputs remain
 * unchanged on failure, including short packets and keyboard error usages.
 * Keys use a 256-bit usage bitmap; modifiers occupy usages E0..E7. */
int hid_report_decode_keyboard(const struct hid_report_descriptor *descriptor,
                               const uint8_t *report_bytes, size_t report_size_bytes,
                               uint8_t pressed_keys[32], unsigned *report_index);
/* Button 1 is bit 0; button_mask identifies supplied button bits. Axes contain
 * relative X/Y/Wheel/Pan counts and are zero for axes absent from this report. */
int hid_report_decode_mouse(const struct hid_report_descriptor *descriptor,
                            const uint8_t *report_bytes, size_t report_size_bytes,
                            uint8_t *pressed_buttons, uint8_t *button_mask,
                            int32_t relative_axes[HID_MOUSE_AXIS_COUNT],
                            unsigned *report_index);

/* Enumerate report_index < descriptor->report_id_count. Unsupported means the
 * report has no LEDs or also contains output controls the encoder cannot supply.
 * led_states bits 0..4 select Num/Caps/Scroll/Compose/Kana. Encoded bytes include
 * the optional ID prefix; encoded_report_id also supplies SET_REPORT's ID. */
int hid_report_encode_leds(const struct hid_report_descriptor *descriptor,
                           uint8_t led_states, unsigned report_index,
                           uint8_t *report_buffer, size_t buffer_capacity,
                           size_t *encoded_size_bytes, uint8_t *encoded_report_id);

#endif
