#ifndef DESCRIPTORS_H_
#define DESCRIPTORS_H_

#include <linux/usb/functionfs.h>
#include <linux/usb/ch9.h>

extern struct usb_descs {
    struct usb_functionfs_descs_head_v2 header;
    __le32 fs_count;
    __le32 hs_count;
    struct {
        struct usb_interface_descriptor intf;
        struct usb_endpoint_descriptor_no_audio bulk_source;
        struct usb_endpoint_descriptor_no_audio bulk_sink;
    } __attribute__ ((__packed__)) fs_descs, hs_descs;
} __attribute__ ((__packed__)) descriptors;

#define STR_INTERFACE "displaylink-responder"

extern struct usb_strs {
    struct usb_functionfs_strings_head header;
    struct {
        __le16 code;
        const char str1[sizeof(STR_INTERFACE)];
    } __attribute__ ((__packed__)) lang0;
} __attribute__ ((__packed__)) strings;

#endif
