// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIB_IOSM_USB_OPS_H
#define LIB_IOSM_USB_OPS_H

#define USB_TRANSFER_DEFAULT_TIMEOUT_MS        (1000)

#define USB_LOG         LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__
#include <iosm_usb3_sp_intfs.h>

#define USB_DEBUG(format, ...) \
    do{    \
        if((USB_LOG >= LOG_DEBUG) && (SAP_LOG != LOG_NONE)) {  \
        printf("[Lib-IOSM-USB][Debug] "format, ## __VA_ARGS__); } }while(0)

#define USB_INFO(format, ...) \
    do{    \
        if((USB_LOG >= LOG_INFO) && (SAP_LOG != LOG_NONE)) {  \
        printf("[Lib-IOSM-USB] "format, ## __VA_ARGS__); } }while(0)

#define USB_ERR(format, ...) \
    do{    \
        if((USB_LOG >= LOG_ERR) && (SAP_LOG != LOG_NONE)) {  \
        printf("[Lib-IOSM-USB][ERROR] "format, ## __VA_ARGS__); } }while(0)

typedef enum
{
    USB_TYPE_INVALID = 0,
    USB2 = 1,
    USB3 = 2,
} USB_BUS_TYPE;

// Maximum of 15 data pipes (exclude the default control pipe)
#define USBPDEV_MAX_DATA_ENDPOINTS_NUM    (15)

// Maximum of 15 data interfaces (i.e., Interfaces containing data endpoints). Each interface has one or more endpoints.
#define USBPDEV_MAX_DATA_INTERFACES_NUM (USBPDEV_MAX_DATA_ENDPOINTS_NUM)

struct usb_hid_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdHID;
    uint8_t  bCountryCode;
    uint8_t  bNumDescriptors;
    uint8_t  bClassDescriptorType;
    uint16_t wClassDescriptorLength;
} PACKED;

struct usbpdev_endpoint_descriptor
{
    // Interface descriptor that owns this endpoint and associated descriptors (e.g., HID descriptor)
    struct usb_interface_descriptor* iface;
    struct usb_hid_descriptor* hid_desc; // HID descriptor, if any

    // USB endpoint descriptor
    struct usb_endpoint_descriptor* epdesc;
};

struct sp_usbpdev_s
{
    void* defpipe;
    struct usb_device_descriptor *ddesc;
    struct usb_config_descriptor *config;
    struct usb_interface_descriptor *data_iface[USBPDEV_MAX_DATA_INTERFACES_NUM]; // Interfaces containing data endpoints
    uint32_t data_ifaces_num;
    struct usbpdev_endpoint_descriptor data_eps[USBPDEV_MAX_DATA_ENDPOINTS_NUM]; // Data endpoints (exclude the default control pipe)
    uint32_t data_eps_num; // Number of data endpoints
    int imax;
    uint8_t speed;

    USBPDEV_ADDR devaddr;
    USB_BUS_TYPE bus_type;

    uint32_t usbpdev_slot;
};

#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIG                   0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05
#define USB_DT_DEVICE_QUALIFIER         0x06
#define USB_DT_OTHER_SPEED_CONFIG       0x07
#define USB_DT_ENDPOINT_HID             0x21
#define USB_DT_ENDPOINT_COMPANION       0x30

#define USB_ENDPOINT_DIR_MASK           0x80

#define USB_ENDPOINT_XFERTYPE_MASK      0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_INT           3

/// @brief Struct for USB data pipes.
/// [NOTE] The type of device default pipes are either <usb2_pipe> or <usb3_pipe>, not <struct sp_usbpdev_pipe>, see 
/// the <defpipe> field of <struct sp_usbpdev_s>.
struct sp_usbpdev_pipe
{
    USB_BUS_TYPE bus_type;

    uint8_t eptype;
    
    void* pipe;

    struct sp_usbpdev_s* dev; // Owner device of the pipe
};

extern SAP_STATUS iosm_usb_init(spid_t in_iosm_usb2_spid, size_t iosm_usb2_usbbuf_4k_pages, spid_t in_iosm_usb3_spid, size_t iosm_usb3_usbbuf_4k_pages);
extern void iosm_usb_fini(void);

/// @brief Activate an ephemeral USB host controller (eHC).
/// @param bus_type 
/// @param out_eHC_slot 
/// @return 
extern SAP_STATUS iosm_usb_activate_eHC(USB_BUS_TYPE bus_type, uint32_t* out_eHC_slot);

/// @brief Deactivate an ephemeral USB host controller (eHC).
/// @param bus_type 
/// @param eHC_slot 
/// @return 
extern SAP_STATUS iosm_usb_deactivate_eHC(USB_BUS_TYPE bus_type, uint32_t eHC_slot);

/********* Functions - USB Peripheral Devices (USBPDevs) *********/
/// @brief Init the general USB configuration of the given USBPDev <dev_addr>.
extern struct sp_usbpdev_s* iosm_usb_init_usbpdev(USB_BUS_TYPE bus_type, USBPDEV_ADDR dev_addr);
extern SAP_STATUS iosm_usb_fini_usbpdev(struct sp_usbpdev_s* dev);

extern SAP_STATUS libusb2_send_bulk(void *pipe_fl, int dir, void *data, int datasize, uint32_t timeout_ms);


/// @brief
/// @param dev
/// @param req Must be allocated using <usbbuf_alloc(USBBUF_MEM_DMA, size)>
/// @param data Must be allocated using <usbbuf_alloc(USBBUF_MEM_DMA, size)>
extern SAP_STATUS iosm_usb_send_default_control(struct sp_usbpdev_s* dev, const struct usb_device_request *req, void *data, uint32_t timeout_ms);

extern struct usb_endpoint_descriptor* libusb2_findEndPointDesc(struct sp_usbpdev_s* usbdev, int type, int dir);

extern struct sp_usbpdev_pipe* iosm_usb_usbpdev_alloc_intr_pipe(
    struct sp_usbpdev_s* usbdev,
    struct usb_endpoint_descriptor* epdesc
);

extern SAP_STATUS iosm_usb_poll_intr(struct sp_usbpdev_pipe* pipe, void *data, size_t data_sz);

extern struct usb_new_pipe* libusb2_usbpdev_alloc_bulk_pipe(struct sp_usbpdev_s* usbdev, struct usb_endpoint_descriptor* epdesc);

/// @brief Free a given pipe
extern SAP_STATUS iosm_usb_usbpdev_free_pipe(struct sp_usbpdev_pipe* pipe);

/// @brief Activate the given USB peripheral device <usbpdev_addr>
extern SAP_STATUS iosm_usb_activate_usbpdev(USB_BUS_TYPE bus_type, USBPDEV_ADDR usbpdev_addr);

/// @brief Deactivate the given USB peripheral device <usbpdev_addr>
extern SAP_STATUS iosm_usb_deactivate_usbpdev(USB_BUS_TYPE bus_type, USBPDEV_ADDR usbpdev_addr);

extern bool usbpdev_addr_equal(USBPDEV_ADDR* addr1, USBPDEV_ADDR* addr2);
extern void usbpdev_addr_copy(USBPDEV_ADDR* to, USBPDEV_ADDR* from);

/********* Debug Functions *********/
extern void iosm_usb_enable_debug(void);
extern void iosm_usb_disable_debug(void);

#endif // __ASSEMBLY__
#endif // LIB_IOSM_USB_OPS_H