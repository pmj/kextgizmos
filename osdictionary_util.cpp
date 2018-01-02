#include <libkern/c++/OSDictionary.h>

#define KLog(...) ({ IOLog(__VA_ARGS__); kprintf(__VA_ARGS); })

void log_dictionary_contents(OSDictionary* table)
{
	KLog("MCTTrigger6USB::matchPropertyTable:\n");
	if (table == nullptr)
	{
		KLog("[NULL dictionary]\n");
	}
	else
	{
		OSCollectionIterator* iter = OSCollectionIterator::withCollection(table);
		
		while (OSObject* obj = iter->getNextObject())
		{
			OSString* key = OSDynamicCast(OSString, obj);
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
				else
				{
					KLog("'%s' -> [%s @ %p]\n", key->getCStringNoCopy(), obj->getMetaClass()->getClassName(), obj);
				}
			}
			else
			{
				KLog("[%s @ %p]\n", obj->getMetaClass()->getClassName(), obj);
			}
		}
		OSSafeReleaseNULL(iter);
	}
}
