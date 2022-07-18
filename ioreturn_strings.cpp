/*
kextgizmos' ioreturn_strings helper for obtaining string representations of
IOReturn codes.


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

#include "ioreturn_strings.h"


#if KERNEL

#include <libkern/version.h>
#if VERSION_MAJOR >= 15
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <IOKit/usb/IOUSBHostInterface.h>
#include <IOKit/usb/USB.h>

#pragma clang diagnostic pop

#else

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <IOKit/usb/USB.h>
#pragma clang diagnostic pop

#endif

#elif TARGET_OS_DRIVERKIT
#include <USBDriverKit/IOUSBHostFamilyDefinitions.h>
#elif TARGET_OS_IOS
#else
#include <IOKit/usb/USB.h>
#include <IOKit/usb/IOUSBHostFamilyDefinitions.h>
#endif

#define RET_CASE(E) case E: return #E

const char* djt_ioreturn_string(IOReturn r)
{
	if (r == kIOReturnSuccess)
		return "kIOReturnSuccess";

	unsigned sys = system_emask & r;
	if (sys != sys_iokit)
		return "NON-IOKIT";
	
	unsigned sub = sub_emask & r;
	switch (sub)
	{
	case sub_iokit_common:
		switch (r)
		{
		// Generic IOKit IOReturns:
		RET_CASE(kIOReturnError);
		RET_CASE(kIOReturnNoMemory);
		RET_CASE(kIOReturnNoResources);
		RET_CASE(kIOReturnIPCError);
		RET_CASE(kIOReturnNoDevice);
		RET_CASE(kIOReturnNotPrivileged);
		RET_CASE(kIOReturnBadArgument);
		RET_CASE(kIOReturnLockedRead);
		RET_CASE(kIOReturnLockedWrite);
		RET_CASE(kIOReturnExclusiveAccess);
		RET_CASE(kIOReturnBadMessageID);
		RET_CASE(kIOReturnUnsupported);
		RET_CASE(kIOReturnVMError);
		RET_CASE(kIOReturnInternalError);
		RET_CASE(kIOReturnIOError);
		RET_CASE(kIOReturnCannotLock);
		RET_CASE(kIOReturnNotOpen);
		RET_CASE(kIOReturnNotReadable);
		RET_CASE(kIOReturnNotWritable);
		RET_CASE(kIOReturnNotAligned);
		RET_CASE(kIOReturnBadMedia);
		RET_CASE(kIOReturnStillOpen);
		RET_CASE(kIOReturnRLDError);
		RET_CASE(kIOReturnDMAError);
		RET_CASE(kIOReturnBusy);
		RET_CASE(kIOReturnTimeout);
		RET_CASE(kIOReturnOffline);
		RET_CASE(kIOReturnNotReady);
		RET_CASE(kIOReturnNotAttached);
		RET_CASE(kIOReturnNoChannels);
		RET_CASE(kIOReturnNoSpace);
		RET_CASE(kIOReturnPortExists);
		RET_CASE(kIOReturnCannotWire);
		RET_CASE(kIOReturnNoInterrupt);
		RET_CASE(kIOReturnNoFrames);
		RET_CASE(kIOReturnMessageTooLarge);
		RET_CASE(kIOReturnNotPermitted);
		RET_CASE(kIOReturnNoPower);
		RET_CASE(kIOReturnNoMedia);
		RET_CASE(kIOReturnUnformattedMedia);
		RET_CASE(kIOReturnUnsupportedMode);
		RET_CASE(kIOReturnUnderrun);
		RET_CASE(kIOReturnOverrun);
		RET_CASE(kIOReturnDeviceError);
		RET_CASE(kIOReturnNoCompletion);
		RET_CASE(kIOReturnAborted);
		RET_CASE(kIOReturnNoBandwidth);
		RET_CASE(kIOReturnNotResponding);
		RET_CASE(kIOReturnIsoTooOld);
		RET_CASE(kIOReturnIsoTooNew);
		RET_CASE(kIOReturnNotFound);
		RET_CASE(kIOReturnInvalid);
		default:
			return "IOKIT-COMMON-UNKNOWN";
		};
	case sub_iokit_usb:
		switch (r)
		{
		// USB specific return values:
#if !TARGET_OS_DRIVERKIT && !TARGET_OS_IOS
		RET_CASE(kIOUSBUnknownPipeErr);
		RET_CASE(kIOUSBTooManyPipesErr);
		RET_CASE(kIOUSBNoAsyncPortErr);
		RET_CASE(kIOUSBNotEnoughPipesErr);
		RET_CASE(kIOUSBNotEnoughPowerErr);
		RET_CASE(kIOUSBEndpointNotFound);
		RET_CASE(kIOUSBConfigNotFound);
		RET_CASE(kIOUSBTransactionTimeout);
		RET_CASE(kIOUSBTransactionReturned);
		RET_CASE(kIOUSBPipeStalled);
		RET_CASE(kIOUSBInterfaceNotFound);
		RET_CASE(kIOUSBLowLatencyBufferNotPreviouslyAllocated);
		RET_CASE(kIOUSBLowLatencyFrameListNotPreviouslyAllocated);
		RET_CASE(kIOUSBHighSpeedSplitError);
		RET_CASE(kIOUSBSyncRequestOnWLThread);
		RET_CASE(kIOUSBClearPipeStallNotRecursive);
#if VERSION_MAJOR > 10
#ifdef kIOUSBDeviceTransferredToCompanion
		RET_CASE(kIOUSBDeviceTransferredToCompanion);
#endif
#ifdef kIOUSBEndpointCountExceeded
		RET_CASE(kIOUSBEndpointCountExceeded);
#endif
#ifdef kIOUSBDeviceCountExceeded
		RET_CASE(kIOUSBDeviceCountExceeded);
#endif
#ifdef kIOUSBStreamsNotSupported
		RET_CASE(kIOUSBStreamsNotSupported);
#endif
#ifdef kIOUSBInvalidSSEndpoint
		RET_CASE(kIOUSBInvalidSSEndpoint);
#endif
#ifdef kIOUSBTooManyTransactionsPending
		RET_CASE(kIOUSBTooManyTransactionsPending);
#endif
#endif
		RET_CASE(kIOUSBLinkErr);
		RET_CASE(kIOUSBNotSent2Err);
		RET_CASE(kIOUSBNotSent1Err);
		RET_CASE(kIOUSBBufferUnderrunErr);
		RET_CASE(kIOUSBBufferOverrunErr);
		RET_CASE(kIOUSBReserved2Err);
		RET_CASE(kIOUSBReserved1Err);
		RET_CASE(kIOUSBWrongPIDErr);
		RET_CASE(kIOUSBPIDCheckErr);
		RET_CASE(kIOUSBDataToggleErr);
		RET_CASE(kIOUSBBitstufErr);
		RET_CASE(kIOUSBCRCErr);

/*
		// USB message specific return values:
		RET_CASE(kIOUSBMessageHubResetPort);
		RET_CASE(kIOUSBMessageHubSuspendPort);
		RET_CASE(kIOUSBMessageHubResumePort);
		RET_CASE(kIOUSBMessageHubIsDeviceConnected);
		RET_CASE(kIOUSBMessageHubIsPortEnabled);
		RET_CASE(kIOUSBMessageHubReEnumeratePort);
		RET_CASE(kIOUSBMessagePortHasBeenReset);
		RET_CASE(kIOUSBMessagePortHasBeenResumed);
		RET_CASE(kIOUSBMessageHubPortClearTT);
		RET_CASE(kIOUSBMessagePortHasBeenSuspended);
		RET_CASE(kIOUSBMessageFromThirdParty);
		RET_CASE(kIOUSBMessagePortWasNotSuspended);
		RET_CASE(kIOUSBMessageExpressCardCantWake);
		RET_CASE(kIOUSBMessageCompositeDriverReconfigured);
		RET_CASE(kIOUSBMessageHubSetPortRecoveryTime);
		RET_CASE(kIOUSBMessageOvercurrentCondition);
		RET_CASE(kIOUSBMessageNotEnoughPower);
		RET_CASE(kIOUSBMessageController);
		RET_CASE(kIOUSBMessageRootHubWakeEvent);
		RET_CASE(kIOUSBMessageReleaseExtraCurrent);
		RET_CASE(kIOUSBMessageReallocateExtraCurrent);
		RET_CASE(kIOUSBMessageEndpointCountExceeded);
		RET_CASE(kIOUSBMessageDeviceCountExceeded);
		RET_CASE(kIOUSBMessageHubPortDeviceDisconnected);
		RET_CASE(kIOUSBMessageUnsupportedConfiguration);
		RET_CASE(kIOUSBMessageHubCountExceeded);
		RET_CASE(kIOUSBMessageTDMLowBattery);
*/
#endif
		
#if !TARGET_OS_IOS
		static_assert((kUSBHostReturnPipeStalled & system_emask) == sys_iokit, "");
		static_assert((kUSBHostReturnPipeStalled & sub_emask) == sub_iokit_usb, "");
		
		RET_CASE(kUSBHostReturnPipeStalled);
		RET_CASE(kUSBHostReturnNoPower);
#endif

		default:
			// This one is weird, in some SDK versions, it's equal to kIOUSBTransactionReturned so we can't have a case: for it
#if !TARGET_OS_DRIVERKIT && !TARGET_OS_IOS
			if (r == kIOUSBDevicePortWasNotSuspended)
				return "kIOUSBDevicePortWasNotSuspended";
#endif
			return "IOKIT-USB-UNKNOWN";
		}
	}
	return "IOKIT-UNKNOWN-UNKNOWN";
}





