#include "ioreturn_strings.h"
#include <IOKit/usb/USB.h>

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
		
		// USB specific return values:
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
		RET_CASE(kIOUSBDevicePortWasNotSuspended);
		RET_CASE(kIOUSBDeviceTransferredToCompanion);
		RET_CASE(kIOUSBEndpointCountExceeded);
		RET_CASE(kIOUSBDeviceCountExceeded);
		RET_CASE(kIOUSBStreamsNotSupported);
		RET_CASE(kIOUSBInvalidSSEndpoint);
		RET_CASE(kIOUSBTooManyTransactionsPending);
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
		default:
			return "IOKIT-COMMON-UNKNOWN";
		}
	}
	return "IOKIT-UNKNOWN-UNKNOWN";
}





