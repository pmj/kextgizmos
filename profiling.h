/* Kext profiling helpers.
 * Copyright 2012-2016 Phillip Dennis-Jordan. All rights reserved.*/

#pragma once

#include <stdint.h>
#include <IOKit/IOReturn.h>

struct dj_profile_probe
{
	int64_t num_samples_1;
	int64_t num_samples_2;
	uint64_t sum_ns;
	union
	{
		__uint128_t sum_sq_ns;
		struct { uint64_t sum_sq_ns_lo; uint64_t sum_sq_ns_hi; };
	};
	uint64_t min_ns;
	uint64_t max_ns;
};
typedef struct dj_profile_probe dj_profile_probe_t;

#define DJ_PROFILE_PROBE_INIT (struct dj_profile_probe){ .num_samples_1 = 0, .num_samples_2 = 0, .sum_ns = 0, .sum_sq_ns = 0, .min_ns = UINT64_MAX, .max_ns = 0 }

#ifdef KERNEL

#ifdef __cplusplus
extern "C" {
#endif

struct IOExternalMethodArguments;

/* external IOUserClient method implementation expecting 1 scalar output &
 * variable sized struct output, ideally big enough to hold an array of num_probes
 * dj_profile_probe_t structs. Makes an attempt at getting a consistent snapshot
 * of each probe, but not across probes. */
IOReturn dj_profile_iouc_export(const volatile dj_profile_probe_t probes[], unsigned num_probes, struct IOExternalMethodArguments* arguments);

#ifdef DJ_PROFILE_ENABLE

uint64_t dj_absolute_nanoseconds(void);

void dj_profile_sample(dj_profile_probe_t* probe, uint64_t start_ns, uint64_t end_ns);

#define DJ_PROFILE_TIME(name) uint64_t name = dj_absolute_nanoseconds()
#define DJ_PROFILE_VAR(name) uint64_t name = 0
#define DJ_PROFILE_TAKE_TIME(name) name = dj_absolute_nanoseconds()
#define DJ_PROFILE_RECORD_SAMPLE(start_name, end_name, probe_index, probe_array) dj_profile_sample(&(probe_array)[probe_index], start_name, end_name)

#else

#define DJ_PROFILE_TIME(name) ({})
#define DJ_PROFILE_VAR(name) ({})
#define DJ_PROFILE_TAKE_TIME(name) ({})
#define DJ_PROFILE_RECORD_SAMPLE(start_name, end_name, probe_index, probe_array) ({})

#endif

#ifdef __cplusplus
}
#endif

#else //!KERNEL

#endif
