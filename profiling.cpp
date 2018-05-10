/* Kext profiling helpers.

Dual-licensed under the MIT and zLib licenses.


Copyright 2012-2016 Phillip Dennis-Jordan

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



Copyright (c) 2012-2016 Phillip Dennis-Jordan

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
