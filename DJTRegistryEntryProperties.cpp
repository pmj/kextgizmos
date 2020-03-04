#include "DJTRegistryEntryProperties.hpp"
#include <IOKit/IORegistryEntry.h>

djt_number_value djt_get_registry_entry_property_number(IORegistryEntry* object, const char* property_name)
{
	djt_number_value prop_val = { 0, false };
	OSObject* prop_obj = object->copyProperty(property_name);
	if (prop_obj != nullptr)
	{
		OSNumber* prop_num = OSDynamicCast(OSNumber, prop_obj);
		if (prop_num != nullptr)
		{
			prop_val.value = prop_num->unsigned64BitValue();
			prop_val.success = true;
		}
		prop_obj->release();
	}
	return prop_val;
}
