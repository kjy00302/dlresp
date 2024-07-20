#include "descriptors.h"

// BEGIN Descriptors and strings
// NOTE: SETUP EVENT REQUIRES FUNCTIONFS_ALL_CTRL_RECIP FLAG!
struct usb_descs descriptors = {
    .header = {
        .magic = FUNCTIONFS_DESCRIPTORS_MAGIC_V2,
        .flags = FUNCTIONFS_HAS_FS_DESC | FUNCTIONFS_HAS_HS_DESC | FUNCTIONFS_ALL_CTRL_RECIP,
        .length = sizeof(descriptors),
    },
    .fs_count = 3,
    .fs_descs = {
        .intf = {
            .bLength = sizeof(descriptors.fs_descs.intf),
            .bDescriptorType = USB_DT_INTERFACE,
            .bNumEndpoints = 2,
            .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
            .iInterface = 1,
        },
        .bulk_source = {
            .bLength = sizeof(descriptors.fs_descs.bulk_source),
            .bDescriptorType = USB_DT_ENDPOINT,
            .bEndpointAddress = 1 | USB_DIR_OUT,
            .bmAttributes = USB_ENDPOINT_XFER_BULK,
        },
        .bulk_sink = {
            .bLength = sizeof(descriptors.fs_descs.bulk_source),
            .bDescriptorType = USB_DT_ENDPOINT,
            .bEndpointAddress = 2 | USB_DIR_IN,
            .bmAttributes = USB_ENDPOINT_XFER_BULK,
        },
    },
    .hs_count = 3,
    .hs_descs = {
        .intf = {
            .bLength = sizeof(descriptors.hs_descs.intf),
            .bDescriptorType = USB_DT_INTERFACE,
            .bNumEndpoints = 2,
            .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
            .iInterface = 1,
        },
        .bulk_source = {
            .bLength = sizeof(descriptors.hs_descs.bulk_source),
            .bDescriptorType = USB_DT_ENDPOINT,
            .bEndpointAddress = 1 | USB_DIR_OUT,
            .bmAttributes = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize = 512,
        },
        .bulk_sink = {
            .bLength = sizeof(descriptors.hs_descs.bulk_source),
            .bDescriptorType = USB_DT_ENDPOINT,
            .bEndpointAddress = 2 | USB_DIR_IN,
            .bmAttributes = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize = 512,
        },
    }
};

struct usb_strs strings = {
    .header = {
        .magic = FUNCTIONFS_STRINGS_MAGIC,
        .length = sizeof(strings),
        .str_count = 1,
        .lang_count = 1,
    },
    .lang0 = {
        0x0409,
        STR_INTERFACE,
    }
};

// END Descriptors and strings 
