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

#pragma once

#include <stdint.h>
#include <CoreFoundation/CFDictionary.h>

#ifdef __cplusplus
extern "C" {
#define DJT_ARG_DEFAULT(expr) = expr
#else
#define DJT_ARG_DEFAULT(expr)
#endif

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDMatch(
    uint16_t vendor_id, uint16_t product_id,
    uint8_t interface_number, uint8_t configuration_value,
    bool modern_iousbhost_classnames DJT_ARG_DEFAULT(true));
CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDRevisionMatch(
    uint16_t vendor_id, uint16_t product_id,
    uint8_t interface_number, uint8_t configuration_value,
    uint8_t revision_bcd_device, bool modern_iousbhost_classnames DJT_ARG_DEFAULT(true));

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDsMatch(
    uint16_t vendor_id, const uint16_t product_ids[], size_t num_product_ids,
    uint8_t interface_number, uint8_t configuration_value, bool modern_iousbhost_classnames DJT_ARG_DEFAULT(true));

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDPIDsRevisionMatch(
    uint16_t vendor_id, const uint16_t product_ids[], size_t num_product_ids,
    uint8_t interface_number, uint8_t configuration_value,
    uint8_t revision_bcd_device, bool modern_iousbhost_classnames DJT_ARG_DEFAULT(true));

CFMutableDictionaryRef DJTCreateIOUSBInterfaceVIDVendorSpecificSubclassProtocolMatch(
	uint16_t vendor_id, uint8_t interface_subclass, uint8_t interface_protocol,
	bool modern_iousbhost_classnames DJT_ARG_DEFAULT(true));


#ifdef __cplusplus
} // extern "C" block
#endif
#undef DJT_ARG_DEFAULT
