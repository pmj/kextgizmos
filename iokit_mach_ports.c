/*
kextgizmos' iokit_mach_ports - helper functions for dealing with I/O Kit Mach
and notification sports.


Dual-licensed under the MIT and zLib licenses.


Copyright 2023 Phillip & Laura Dennis-Jordan

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



Copyright (c) 2018-2020 Phillip & Laura Dennis-Jordan

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

#include "iokit_mach_ports.h"
#include <TargetConditionals.h>
#include <IOKit/IOKitLib.h>

mach_port_t djt_iokit_main_port_default(void)
{
#if (TARGET_OS_MAC || TARGET_OS_MACCATALYST) && (!defined(MAC_OS_VERSION_12_0) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_VERSION_12_0)
	_Pragma("clang diagnostic push")
	_Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"");
	return kIOMasterPortDefault;
		_Pragma("clang diagnostic pop")
#else
	return kIOMainPortDefault;
#endif
}
