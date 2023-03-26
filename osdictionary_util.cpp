/*
kextgizmos osdictionary_util


Dual-licensed under the MIT and zLib licenses.


Copyright 2018-2020 Phillip & Laura Dennis-Jordan

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

#include "osdictionary_util.hpp"
#include <TargetConditionals.h>

#if TARGET_OS_DRIVERKIT
// DEXT
#include <DriverKit/OSNumber.h>
#include <DriverKit/OSBoolean.h>
#include <DriverKit/OSString.h>
#include <DriverKit/OSDictionary.h>
#include <DriverKit/IOLib.h>

#else
// KEXT
#include <libkern/c++/OSDictionary.h>
#include <libkern/c++/OSCollectionIterator.h>
#include <libkern/c++/OSString.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSBoolean.h>
#include <libkern/c++/OSData.h>
#include <pexpert/pexpert.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <IOKit/IOLib.h>
#pragma clang diagnostic pop

#if __has_builtin(__builtin_alloca)
#define alloca(s) __builtin_alloca(s)
#else
extern void* alloca(size_t);
#endif

#define KLog(...) ({ IOLog(__VA_ARGS__); kprintf(__VA_ARGS__); })
#endif // end platform specific


#if defined (__cplusplus) && __cplusplus < 201103L && !defined(nullptr)

class DJTNullptr
{
public:
	template <typename T> operator T*() const
	{
		return 0;
	}
};

#define nullptr (DJTNullptr())
#endif

#if !TARGET_OS_DRIVERKIT

void log_dictionary_contents(OSDictionary* table, bool recurse, unsigned indent)
{
	if (table == nullptr)
	{
		KLog("%*s[NULL dictionary]\n", indent, "");
	}
	else
	{
		OSCollectionIterator* iter = OSCollectionIterator::withCollection(table);
		
		while (OSObject* key_obj = iter->getNextObject())
		{
			OSString* key = OSDynamicCast(OSString, key_obj);
			if (key != nullptr)
			{
				OSObject* obj = table->getObject(key);
				OSNumber* num = OSDynamicCast(OSNumber, obj);
				OSBoolean* boolean = OSDynamicCast(OSBoolean, obj);
				OSString* str = OSDynamicCast(OSString, obj);
				if (num != nullptr)
				{
					KLog("'%s' -> %llu (0x%llx, %lld)\n", key->getCStringNoCopy(), num->unsigned64BitValue(), num->unsigned64BitValue(), num->unsigned64BitValue());
				}
				else if (boolean != nullptr)
				{
					KLog("'%s' -> %s\n", key->getCStringNoCopy(), boolean->getValue() ? "true" : "false");
				}
				else if (str != nullptr)
				{
					KLog("'%s' -> '%s'\n", key->getCStringNoCopy(), str->getCStringNoCopy());
				}
				else if (obj == nullptr)
				{
					KLog("'%s' -> NULL\n", key->getCStringNoCopy());
				}
				else if (OSData* data = OSDynamicCast(OSData, obj))
				{
					unsigned bytes_remain = data->getLength();
					KLog("'%s' -> [OSData, %u bytes] <\n", key->getCStringNoCopy(), bytes_remain);
					const uint8_t* bytes = static_cast<const uint8_t*>(data->getBytesNoCopy());
					while (bytes_remain >= 16)
					{
						KLog("    %02x %02x %02x %02x  %02x %02x %02x %02x    %02x %02x %02x %02x  %02x %02x %02x %02x\n",
							bytes[0], bytes[1], bytes[2],  bytes[3],   bytes[4],  bytes[5],  bytes[6],  bytes[7],
							bytes[8], bytes[9], bytes[10], bytes[11],  bytes[12], bytes[13], bytes[14], bytes[15]);
						bytes_remain -= 16;
						bytes += 16;
					}
					
					char buffer[100] = {};
					char* buffer_pos = buffer;
					size_t buffer_remain = sizeof(buffer);
					int written = snprintf(buffer_pos, buffer_remain, "    ");
					buffer_remain -= written;
					buffer_pos += written;
					unsigned index = 0;
					while (bytes_remain > 0 && buffer_remain > 0)
					{
						written = snprintf(buffer_pos, buffer_remain, "%02x %*s", bytes[0], ((index % 4 == 0) ? 1 : 0) + ((index % 8 == 0) ? 2 : 0), "");
						buffer_remain -= written;
						buffer_pos += written;
						bytes++;
						bytes_remain--;
					}
					written = snprintf(buffer_pos, buffer_remain, ">\n");
					KLog("%s", buffer);
				}
				else
				{
					KLog("'%s' -> [%s @ %p]\n", key->getCStringNoCopy(), obj->getMetaClass()->getClassName(), obj);
				}
			}
			else
			{
				KLog("[%s @ %p]\n", key_obj->getMetaClass()->getClassName(), key_obj);
			}
		}
		OSSafeReleaseNULL(iter);
	}
}
#endif

bool DJTDictionarySetNumber(OSDictionary* dictionary, const char* key, uint64_t value)
{
	OSNumber* number = OSNumber::withNumber(value, 64);
	bool ok = dictionary->setObject(key, number);
	OSSafeReleaseNULL(number);
	return ok;
}

bool DJTDictionarySetBoolean(OSDictionary* dictionary, const char* key, bool value)
{
	return dictionary->setObject(key, value ? kOSBooleanTrue : kOSBooleanFalse);
}

bool DJTDictionarySetString(OSDictionary* dictionary, const char* key, const char* string, uint32_t string_max_len)
{
	size_t length = strnlen(string, string_max_len);
	char* terminated_string = static_cast<char*>(alloca(length + 1));
	memcpy(terminated_string, string, length);
	terminated_string[length] = '\0';
	OSString* string_obj = OSString::withCString(terminated_string);
	bool ok = dictionary->setObject(key, string_obj);
	OSSafeReleaseNULL(string_obj);
	return ok;
}

OSDictionary* djt_osdictionary_create_merged(OSDictionary* dictionary, OSDictionary* into)
{
	if (into == nullptr)
		return OSDictionary::withDictionary(dictionary, dictionary->getCount());
	
	OSDictionary* created = OSDictionary::withDictionary(into, into->getCount() + (dictionary ? dictionary->getCount() : 0));
	if (dictionary == nullptr)
		return created;
	
	created->merge(dictionary);
	return created;
}
