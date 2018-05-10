/*
kextgizmos osobject_retaincount.h


Dual-licensed under the MIT and zLib licenses.


Copyright 2016-2018 Phillip & Laura Dennis-Jordan

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



Copyright (c) 2016-2018 Phillip & Laura Dennis-Jordan

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

#include <libkern/c++/OSObject.h>
#include <libkern/OSDebug.h>

#define DJ_OSOBJECT_RETAINCOUNT(CLASS, SUPERCLASS) \
	virtual void taggedRelease(const void * tag, const int freeWhen) const override \
	{ \
		int count = this->getRetainCount(); \
		void* bt[10] = { 0 }; \
		OSBacktrace(bt, 10); \
		kprintf(#CLASS " %p release: %2d -> %2d (freeWhen = %d, tag = %p) trace: %p  %p  %p  %p  %p  %p  %p  %p  %p  %p\n", \
			this, count, count - 1, freeWhen, tag, \
			bt[0], bt[1], bt[2], bt[3], bt[4], bt[5], \
			bt[6], bt[7], bt[8], bt[9]); \
		OSReportWithBacktrace(#CLASS " %p release: %2d -> %2d (freeWhen = %d, tag = %p)\n", this, count, count - 1, freeWhen, tag); \
		this->SUPERCLASS::taggedRelease(tag, freeWhen); \
	} \
	virtual void taggedRetain(const void * tag = 0) const override \
	{ \
		int count = this->getRetainCount(); \
		void* bt[10] = { 0 }; \
		OSBacktrace(bt, 10); \
		kprintf(#CLASS" %p retain:  %2d -> %2d (tag = %p) trace: %p  %p  %p  %p  %p  %p  %p  %p  %p  %p\n", \
			this, count, count + 1, tag, \
			bt[0], bt[1], bt[2], bt[3], bt[4], bt[5], \
			bt[6], bt[7], bt[8], bt[9]); \
		OSReportWithBacktrace(#CLASS " %p retain:  %2d -> %2d (tag = %p)\n", this, count, count + 1, tag); \
		this->SUPERCLASS::taggedRetain(tag); \
	}

__private_extern__ kmod_info_t kmod_info;

#define DJ_KEXTGIZMO_STRINGIFY2(MACRONAME) #MACRONAME
#define DJ_KEXTGIZMO_STRINGIFY(MACRONAME) DJ_KEXTGIZMO_STRINGIFY2(MACRONAME)

struct DJKextgizmoKextAddressDump
{
	DJKextgizmoKextAddressDump()
	{
		kmod_info_t* info = &kmod_info;
		kprintf("Kext '%." DJ_KEXTGIZMO_STRINGIFY(KMOD_MAX_NAME) "s' start address: 0x%16lx, end address: 0x%16lx. kmod info: %p\n", info->name, info->address, info->address + info->size - 1, info);
	}
};

static DJKextgizmoKextAddressDump address_dump;
