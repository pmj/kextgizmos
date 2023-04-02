/*
kextgizmos iokit-matching-usb

Helper functions for generating I/O Kit matching dictionaries for USB devices,
interfaces, etc.
These are based on the matching rules documented in Apple's Tech Q&A 1076.
https://developer.apple.com/library/archive/qa/qa1076/_index.html
Deviating from these matching patterns will typically cause matching to fail!

This is intended to be used in user space, and requires linking against
IOKit.framework. Header can be #included from C, C++, Objective-C, and
Objective-C++ sources.


Dual-licensed under the MIT and zLib licenses.


Copyright 2022 Phillip Dennis-Jordan

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.



Copyright (c) 2022 Phillip Dennis-Jordan

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use
of this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject
to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
that you wrote the original software. If you use this software in a product,
an acknowledgment in the product documentation would be appreciated but is not
required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#include "iokit-matching-usb.h"
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CFNumber.h>
#include <cassert>

#if TARGET_OS_IOS
// iOS/iPadOS is missing the USB header files and legacy class names are irrelevant.
#define kIOUSBHostInterfaceClassName "IOUSBHostInterface"
#define kUSBHostMatchingPropertyVendorID "idVendor"
#define kUSBHostMatchingPropertyProductID "idProduct"
#define kUSBHostMatchingPropertyProductIDArray "idProductArray"
#define kUSBHostMatchingPropertyInterfaceNumber "bInterfaceNumber"
#define kUSBHostMatchingPropertyConfigurationValue "bConfigurationValue"
#define kUSBHostMatchingPropertyDeviceReleaseNumber "bcdDevice"

#define non_macos_assert(expr) assert(expr)

#else
#include <IOKit/usb/IOUSBHostFamilyDefinitions.h>
#include <IOKit/usb/IOUSBLib.h>
#define non_macos_assert(expr) ({})
#endif

static inline const char* usb_interface_classname(bool modern_iousbhost)
{
#if TARGET_OS_IOS
	return kIOUSBHostInterfaceClassName;
#else
	return modern_iousbhost ? kIOUSBHostInterfaceClassName :
#ifdef kIOUSBInterfaceClass
		kIOUSBInterfaceClass;
#else
		kIOUSBInterfaceClassName;
#endif
#endif
}

static void dictionary_set_number_value(CFMutableDictionaryRef dict, CFStringRef key, int32_t val)
{
    CFNumberRef num = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &val);
    CFDictionarySetValue(dict, key, num);
    CFRelease(num);
}

static void dictionary_set_vid_iface_config(
    CFMutableDictionaryRef dict, uint16_t vendor_id,
    uint8_t interface_number, uint8_t configuration_value)
{
    dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyVendorID), vendor_id);
    dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyInterfaceNumber), interface_number);
    dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyConfigurationValue), configuration_value);
}

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDMatch(
    uint16_t vendor_id, uint16_t product_id,
    uint8_t interface_number, uint8_t configuration_value, bool modern_iousbhost_classnames)
{
    non_macos_assert(modern_iousbhost_classnames);
    CFMutableDictionaryRef dict = IOServiceMatching(usb_interface_classname(modern_iousbhost_classnames));
    dictionary_set_vid_iface_config(dict, vendor_id, interface_number, configuration_value);
    dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyProductID), product_id);
    return dict;
}

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDRevisionMatch(
    uint16_t vendor_id, uint16_t product_id,
    uint8_t interface_number, uint8_t configuration_value, uint8_t revision_bcd_device, bool modern_iousbhost_classnames)
{
    CFMutableDictionaryRef dict = DJTCreateIOUSBInterfaceVIDPIDMatch(vendor_id, product_id, interface_number, configuration_value, modern_iousbhost_classnames);
    dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyDeviceReleaseNumber), revision_bcd_device);
    return dict;
}

CFArrayRef create_cfnumber_array(const uint16_t* nums, size_t num_nums)
{
    CFMutableArrayRef num_array = CFArrayCreateMutable(kCFAllocatorDefault, num_nums, &kCFTypeArrayCallBacks);
    for (size_t i = 0; i < num_nums; ++i)
    {
        int32_t val = nums[i];
        CFNumberRef cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &val);
        CFArrayAppendValue(num_array, cfnum);
        CFRelease(cfnum);
    }
    
    return num_array;
}

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDsMatch(
    uint16_t vendor_id, const uint16_t product_ids[], size_t num_product_ids,
    uint8_t interface_number, uint8_t configuration_value, bool modern_iousbhost_classnames)
{
    non_macos_assert(modern_iousbhost_classnames);
    CFMutableDictionaryRef dict = IOServiceMatching(usb_interface_classname(modern_iousbhost_classnames));
    dictionary_set_vid_iface_config(dict, vendor_id, interface_number, configuration_value);
    CFArrayRef pid_array = create_cfnumber_array(product_ids, num_product_ids);
    CFDictionarySetValue(dict, CFSTR(kUSBHostMatchingPropertyProductIDArray), pid_array);
    CFRelease(pid_array);
    return dict;
}

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDsRevisionMatch(
    uint16_t vendor_id, const uint16_t product_ids[], size_t num_product_ids,
    uint8_t interface_number, uint8_t configuration_value,
    uint8_t revision_bcd_device, bool modern_iousbhost_classnames)
{
    CFMutableDictionaryRef dict = DJTCreateIOUSBInterfaceVIDPIDsMatch(vendor_id, product_ids, num_product_ids, interface_number, configuration_value, modern_iousbhost_classnames);
    dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyDeviceReleaseNumber), revision_bcd_device);
    return dict;
}

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDVendorSpecificSubclassProtocolMatch(
	uint16_t vendor_id, uint8_t interface_subclass, uint8_t interface_protocol,
	bool modern_iousbhost_classnames)
{
	non_macos_assert(modern_iousbhost_classnames);
	CFMutableDictionaryRef dict = IOServiceMatching(usb_interface_classname(modern_iousbhost_classnames));
	dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyVendorID), vendor_id);
	dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyInterfaceSubClass), interface_subclass);
	dictionary_set_number_value(dict, CFSTR(kUSBHostMatchingPropertyInterfaceProtocol), interface_protocol);
	return dict;
}
