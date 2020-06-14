//
//  dext_userclient.h
//  MCTTriggerDriver
//
//  Created by Phil Dennis-Jordan on 14.06.20.
//  Copyright © 2020 MCT. All rights reserved.
//

#pragma once

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>

#ifdef __cplusplus
extern "C"
#endif
CFMutableDictionaryRef djt_user_service_matching(CFStringRef driverkit_classname, CFStringRef driverkit_server_bundle_id);

