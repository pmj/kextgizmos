/* Kext profiling helpers.
 * Copyright 2012-2016 Phillip Dennis-Jordan. All rights reserved.*/

#include "profiling.h"

#ifdef DJ_PROFILE_ENABLE

#include <kern/clock.h>
#include <libkern/OSAtomic.h>
#include <IOKit/IOUserClient.h>

uint64_t dj_absolute_nanoseconds()
{
	uint64_t ns;
	absolutetime_to_nanoseconds(mach_absolute_time(), &ns);
	return ns;
}

void dj_profile_sample(dj_profile_probe_t* probe, uint64_t start_ns, uint64_t end_ns)
{
	OSIncrementAtomic64(&probe->num_samples_1);
	uint64_t delta = end_ns - start_ns;
	
	OSAddAtomic64(delta, &probe->sum_ns);
	
	uint64_t min = probe->min_ns;
	while (min > delta)
	{
		if (OSCompareAndSwap64(min, delta, &probe->min_ns))
			break;
		min = probe->min_ns;
	}
	uint64_t max = probe->max_ns;
	while (max < delta)
	{
		if (OSCompareAndSwap64(max, delta, &probe->max_ns))
			break;
		max = probe->max_ns;
	}
	
	__uint128_t delta_sq = static_cast<__uint128_t>(delta) * delta;
	uint64_t delta_sq_lo = delta_sq;
	uint64_t delta_sq_hi = delta_sq >> 64;
	uint64_t prev_lo = OSAddAtomic64(delta_sq_lo, &probe->sum_sq_ns_lo);
	uint64_t carry = 0;
	__builtin_addcll(delta_sq_lo, prev_lo, carry, &carry);
	delta_sq_hi += carry;
	OSAddAtomic64(delta_sq_hi, &probe->sum_sq_ns_hi);
	
	OSIncrementAtomic64(&probe->num_samples_2);
}

IOReturn dj_profile_iouc_export(const volatile dj_profile_probe_t probes[], unsigned num_probes, IOExternalMethodArguments* arguments)
{
	if (arguments->scalarOutputCount != 1
	    || (arguments->structureOutputSize > 0 && arguments->structureOutput == nullptr))
		return kIOReturnBadArgument;

	arguments->scalarOutput[0] = num_probes;
	
	dj_profile_probe_t* export_probes = static_cast<dj_profile_probe_t*>(arguments->structureOutput);
	
	if (arguments->structureOutputSize / sizeof(dj_profile_probe_t) < num_probes)
		num_probes = arguments->structureOutputSize / sizeof(dj_profile_probe_t);
	for (unsigned i = 0; i < num_probes; ++i)
	{
		unsigned tries = 0;
		dj_profile_probe_t probe_copy = {};
		do
		{
			probe_copy.num_samples_1 = probes[i].num_samples_1;
			
			probe_copy.sum_ns = probes[i].sum_ns;
			probe_copy.sum_sq_ns = probes[i].sum_sq_ns;
			probe_copy.min_ns = probes[i].min_ns;
			probe_copy.max_ns = probes[i].max_ns;

			probe_copy.num_samples_2 = probes[i].num_samples_2;

			++tries;
		} while (probe_copy.num_samples_1 != probe_copy.num_samples_2 && tries < 100);
		
		export_probes[i] = probe_copy;
	}
	
	return kIOReturnSuccess;
}
#else
IOReturn dj_profile_iouc_export(const volatile dj_profile_probe_t probes[], unsigned num_probes, IOExternalMethodArguments* arguments)
{
	return kIOReturnUnsupported;
}
#endif
