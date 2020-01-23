#pragma once

#include <stdbool.h>
#include <mach/mach_types.h>

struct cs_info
{
	bool signer_type_set;
	bool flags_set;
	bool teamid_set;
	unsigned int signer_type;
	unsigned int flags;
	const char* teamid;
};

#ifdef __cplusplus
extern "C"
#endif
bool djt_task_check_security(task_t task, struct cs_info* out_codesign_info);
