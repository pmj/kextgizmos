/* kextgizmos IOPCIDevice helpers

Dual-licensed under the MIT and zLib licenses.


Copyright 2018 Phillip & Laura Dennis-Jordan

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



Copyright (c) 2018 Phillip & Laura Dennis-Jordan

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

class IOPCIDevice;

uint8_t djt_iopcidevice_first_capability_offset(IOPCIDevice* dev)  __attribute__((visibility("hidden")));
uint8_t djt_iopcidevice_next_capability_offset(IOPCIDevice* dev, uint8_t prev_capability_offset)  __attribute__((visibility("hidden")));
int djt_iopcidevice_memory_range_type(IOPCIDevice* dev, uint8_t reg)  __attribute__((visibility("hidden")));
uint8_t djt_iopcidevice_register_for_range_index(uint8_t index) __attribute__((visibility("hidden")));

struct djt_pci_interrupt_index_ranges
{
	int irq_pin_start;
	int irq_pin_end; // exclusive (count = end - start)
	int msi_start;
	int msi_end; // exclusive
};

djt_pci_interrupt_index_ranges djt_iopcidevice_find_interrupt_ranges(IOPCIDevice* pci_device) __attribute__((visibility("hidden")));

void djt_kprint_pci_config_space(IOPCIDevice* pci_dev, const char* line_prefix_str);
