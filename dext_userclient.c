//
//  dext_userclient.c
//  MCTTriggerDriver
//
//  Created by Phil Dennis-Jordan on 14.06.20.
//  Copyright © 2020 MCT. All rights reserved.
//

#include "dext_userclient.h"
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CFBundle.h>

// Creates IOKit matching dictionary for locating driverkit-based service objects
// (Corresponds to kext services' IOServiceMatching())
CFMutableDictionaryRef djt_user_service_matching(CFStringRef driverkit_classname, CFStringRef driverkit_server_bundle_id) CF_RETURNS_RETAINED
{
	CFMutableDictionaryRef match = IOServiceMatching("IOUserService");
	CFTypeRef match_property_keys[]   = { CFSTR("IOUserClass"), kCFBundleIdentifierKey };
	CFTypeRef match_property_values[] = { driverkit_classname,  driverkit_server_bundle_id };
	CFDictionaryRef match_properties = CFDictionaryCreate(
		kCFAllocatorDefault,
		match_property_keys, match_property_values, 2,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionaryAddValue(match, CFSTR(kIOPropertyMatchKey), match_properties);
	CFRelease(match_properties);
	return match;
}
