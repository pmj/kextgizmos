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

#include "iopcidevice_helpers.hpp"

#if DEBUG
#include "ioreturn_strings.h"
#endif

#include <IOKit/pci/IOPCIDevice.h>


// TODO: move these to a logging gizmo

#define ANSI_ESCAPE_RESET "\x1b[0m"
#define ANSI_ESCAPE_DARKGREY "\x1b[90m"
#define ANSI_ESCAPE_RED "\x1b[31m"

#define LogWithLocation(fmt, ...) ({ kprintf( ANSI_ESCAPE_DARKGREY "%s:" ANSI_ESCAPE_RESET " " fmt , /*__PRETTY_FUNCTION__*/ __func__ , ## __VA_ARGS__); })
#define LogWarning(fmt, ...) LogWithLocation(ANSI_ESCAPE_RED "Warning: " ANSI_ESCAPE_RESET fmt, ## __VA_ARGS__)
#if DEBUG
#define LogVerbose(fmt, ...) LogWithLocation(fmt, ## __VA_ARGS__)
#endif

// Returns 0 if the device does not support PCI capabilities, or if the config space layout is bad
uint8_t djt_iopcidevice_first_capability_offset(IOPCIDevice* dev)
{
	uint16_t status_reg = dev->configRead16(kIOPCIConfigStatus);
	if (0 == (status_reg & kIOPCIStatusCapabilities))
		return 0;
	
	uint8_t first_capability_offset = dev->configRead8(kIOPCIConfigCapabilitiesPtr);
	if (first_capability_offset < 0x40 || (first_capability_offset % 4 != 0))
	{
		LogVerbose("Bad value 0x%02x found in capabilities pointer register (should be 0x40-0xfc)\n", first_capability_offset);
		first_capability_offset = 0;
	}
	return first_capability_offset;
}

// Returns 0 if the previous capability was the last one in the chain. NB does not defend against malformed infinite-loop lists.
uint8_t djt_iopcidevice_next_capability_offset(IOPCIDevice* dev, uint8_t prev_capability_offset)
{
	assert(prev_capability_offset >= 0x40);
	return dev->configRead8(prev_capability_offset + 1);
}

/* reg can be in range kIOPCIConfigBaseAddress0..kIOPCIConfigBaseAddress5 inclusive
 * Returns one of:
 *  kIOPCIIOSpace
 *  kIOPCI32BitMemorySpace
 *  kIOPCI64BitMemorySpace
 *  -1 (error)
 */
int djt_iopcidevice_memory_range_type(IOPCIDevice* dev, UInt8 reg)
{
	if (reg < kIOPCIConfigBaseAddress0 || reg > kIOPCIConfigBaseAddress5 || reg % 4 != 0)
		return -1;
	
	if (reg != kIOPCIConfigBaseAddress0)
	{
		// registers other than 0 can be second half of 64-bit BAR, so check that's not the case
		uint32_t even_bar_val = dev->configRead32(reg - 4);
		if ((even_bar_val & 0x1) == 0 && (even_bar_val & 0x6) == 0x02)
			return -1;
	}

	uint32_t bar_val = dev->configRead32(reg);
	if ((bar_val & 0x1) != 0)
		return kIOPCIIOSpace;
	else
	{
		uint32_t bar_mem_type = (bar_val & 0x6) >> 1;
		switch (bar_mem_type)
		{
		case 0x00:
			return kIOPCI32BitMemorySpace;
		case 0x02:
			if (reg == kIOPCIConfigBaseAddress5)
				return -1; // 64-bit in BAR5 does not make any sense
			return kIOPCI64BitMemorySpace;
		}
		return -1;
	}
}

uint8_t djt_iopcidevice_register_for_range_index(uint8_t index)
{
	assert(index <= 5);
	return kIOPCIConfigBaseAddress0 + (index * (kIOPCIConfigBaseAddress1 - kIOPCIConfigBaseAddress0));
}


djt_pci_interrupt_index_ranges djt_iopcidevice_find_interrupt_ranges(IOPCIDevice* pci_device)
{
	djt_pci_interrupt_index_ranges ranges = { -1, -1, -1, -1 };
	bool pin_started = false;
	bool pin_done = false;
	bool msi_started = false;
	bool msi_done = false;
	
	int32_t intr_index = 0;
	// keep trying interrupt source indices until we run out
	while (intr_index <= INT32_MAX)
	{
		int intr_type = 0;
		IOReturn ret = pci_device->getInterruptType(intr_index, &intr_type);
		LogVerbose("Index %d type: 0x%x -> 0x%x (%s)\n", intr_index, intr_type, ret, djt_ioreturn_string(ret));
		if (ret != kIOReturnSuccess)
			break;
		
		if (intr_type & kIOInterruptTypePCIMessaged)
		{
			// found MSI interrupt source
			if (msi_done)
			{
				LogWarning("New unexpected MSI range found starting at index %d, existing range %d..%d (inclusive)\n",
					intr_index, ranges.msi_start, ranges.msi_end);
			}
			else
			{
				if (!msi_started)
				{
					if (pin_started)
					{
						pin_started = false;
						pin_done = true;
					}
					msi_started = true;
					ranges.msi_start = intr_index;
				}
				ranges.msi_end = intr_index + 1;
			}
		}
		else
		{
			// found traditional IRQ interrupt source
			if (pin_done)
			{
				LogWarning("New unexpected IRQ range found starting at index %d, existing range %d..%d (inclusive)\n",
					intr_index, ranges.irq_pin_start, ranges.irq_pin_end);
			}
			else
			{
				if (!pin_started)
				{
					if (msi_started)
					{
						msi_started = false;
						msi_done = true;
					}
					pin_started = true;
					ranges.irq_pin_start = intr_index;
				}
				ranges.irq_pin_end = intr_index + 1;
			}
		}
		++intr_index;
	}
	
	return ranges;
}


void djt_kprint_pci_config_space(IOPCIDevice* pci_dev, const char* line_prefix_str)
{
	uint8_t config_space[256];
	
	for (unsigned i = 0; i < 256; ++i)
	{
		config_space[i] = pci_dev->configRead8(i);
	}
	
	for (unsigned i = 0; i < 16; ++i)
	{
		unsigned offset = i * 16;
		const uint8_t* b = config_space + offset;
		kprintf("%s[%3u]: %02x %02x %02x %02x  %02x %02x %02x %02x    %02x %02x %02x %02x  %02x %02x %02x %02x\n",
			line_prefix_str, offset,
			b[0], b[1], b[2], b[3],  b[4], b[5], b[6], b[7],  b[8], b[9], b[10], b[11],  b[12], b[13], b[14], b[15]);
	}
}

