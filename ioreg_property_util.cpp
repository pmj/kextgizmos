#include "ioreg_property_util.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

// Key for dictionary property below which all properties exported by dexts are published on macOS 10.15.x & 11.x
#define kIOGizmo_IOUserServicePropertiesKey "IOUserServiceProperties"

static void safe_cf_release(CFTypeRef obj [[clang::cf_consumed]]);
static void safe_cf_release(CFTypeRef obj)
{
	if (obj != nullptr)
		CFRelease(obj);
}

static CFDictionaryRef registry_entry_create_user_property_dictionary(io_registry_entry_t reg_entry, CFStringRef property_key)
{
	CFTypeRef user_properties_obj = IORegistryEntryCreateCFProperty(reg_entry, CFSTR(kIOGizmo_IOUserServicePropertiesKey), kCFAllocatorDefault, 0 /* no options exist */);
	if (user_properties_obj == nullptr || CFGetTypeID(user_properties_obj) != CFDictionaryGetTypeID())
	{
		safe_cf_release(user_properties_obj);
		return nullptr;
	}
	
	return static_cast<CFDictionaryRef>(user_properties_obj);
}

bool iogizmo_registry_entry_get_int_user_property(io_registry_entry_t reg_entry, CFStringRef property_key, int* out_value, int default_value)
{
	*out_value = default_value;

	CFDictionaryRef user_properties = registry_entry_create_user_property_dictionary(reg_entry, property_key);
	if (user_properties == nullptr)
		return false;
	
	CFTypeRef property_obj = CFDictionaryGetValue(user_properties, property_key);
	bool ok =
		property_obj != nullptr
		&& CFGetTypeID(property_obj) == CFNumberGetTypeID();
	if (ok)
		ok = CFNumberGetValue(static_cast<CFNumberRef>(property_obj), kCFNumberIntType, out_value);
		
	safe_cf_release(user_properties);
	return ok;
}

bool iogizmo_registry_entry_get_boolean_user_property(io_registry_entry_t reg_entry, CFStringRef property_key, bool* out_value, bool default_value)
{
	*out_value = default_value;

	CFDictionaryRef user_properties = registry_entry_create_user_property_dictionary(reg_entry, property_key);
	if (user_properties == nullptr)
		return false;
	
	CFTypeRef property_obj = CFDictionaryGetValue(user_properties, property_key);
	bool ok =
		property_obj != nullptr
		&& CFGetTypeID(property_obj) == CFBooleanGetTypeID();
	if (ok)
		*out_value = CFBooleanGetValue(static_cast<CFBooleanRef>(property_obj));
		
	safe_cf_release(user_properties);
	return ok;
}
