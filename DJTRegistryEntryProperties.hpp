#pragma once

#include <stdint.h>

class IORegistryEntry;

struct djt_number_value
{
	uint64_t value;
	bool success;
};

djt_number_value djt_get_registry_entry_property_number(IORegistryEntry* object, const char* property_name);
