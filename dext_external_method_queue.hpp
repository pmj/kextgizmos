/*
kextgizmos dext_external_method_queue


Dual-licensed under the MIT and zLib licenses.


Copyright 2023 Phillip Dennis-Jordan

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



Copyright (c) 2023 Phillip Dennis-Jordan

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

#include <TargetConditionals.h>

#if TARGET_OS_DRIVERKIT
class IOUserClient;

/// macOS 10.15/11 compatibility fixup for DriverKit 21+ SDK
/** This function should be called during init or start of every IOUserClient
 * subclass unless an ExternalMethod dispatch queue is explicitly set.
 * If the class overrides its default queue, this fixup must be done after the
 * default queue has been set.
 * Dexts built with DriverKit 21 SDK or newer expect the queue named in
 * kIOUserClientQueueNameExternalMethod, but the DriverKit runtime in macOS 11
 * and older (DriverKit 19/20) doesn't know about this, so the dext crashes in
 * OSMetaClassBase::QueueForObject when an ExternalMethod is called from user
 * space. */
bool DJTUserClientExternalMethodQueueFixup(IOUserClient* uc);
#endif
