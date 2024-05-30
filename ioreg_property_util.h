#pragma once

#include <IOKit/IOTypes.h>
#include <CoreFoundation/CFBase.h>

#ifdef __cplusplus
#define IOGIZMO_CPPONLY(x) x
extern "C" {
#else
#define IOGIZMO_CPPONLY(x)
#endif

bool iogizmo_registry_entry_get_boolean_user_property(
	io_registry_entry_t reg_entry, CFStringRef property_key, bool* out_value, bool default_value IOGIZMO_CPPONLY(= false));
bool iogizmo_registry_entry_get_int_user_property(
	io_registry_entry_t reg_entry, CFStringRef property_key, int* out_value, int default_value IOGIZMO_CPPONLY(= 0));

#ifdef __cplusplus
}
#endif
#undef IOGIZMO_CPPONLY
