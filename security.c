#include "security.h"
#include <string.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <libkern/version.h>
#include <libkern/OSKextLib.h>

extern void* get_bsdtask_info(task_t) __attribute__((weak_import));
// ubc.h
extern struct cs_blob* ubc_cs_blob_get(vnode_t, cpu_type_t, off_t)  __attribute__((weak_import));

static vnode_t proc_get_textvp(proc_t p);
static struct cs_info csblob_get_info(const struct cs_blob* cs);

bool djt_task_check_security(task_t task, struct cs_info* out_codesign_info)
{
	*out_codesign_info = (struct cs_info){};
	if (!OSKextSymbolIsResolved(get_bsdtask_info) || !OSKextSymbolIsResolved(ubc_cs_blob_get))
		return false;
	
	proc_t p = get_bsdtask_info(task);
	if (p == NULL)
		return false;
	
	vnode_t executable_vnode = proc_get_textvp(p);
	if (executable_vnode == NULLVP)
		return false;
	
	struct cs_blob* cs = ubc_cs_blob_get(executable_vnode, CPU_TYPE_X86_64, 0 /* offset */);
	*out_codesign_info = csblob_get_info(cs);
	
	vnode_put(executable_vnode);
	return true;
}

/* Offsets are obtained by running this command in (offline) lldb session on kernel image from KDK:
 * p &((struct proc*)0)->p_textvp
 */

// KDK_10.12_16A239m.kdk through KDK_10.12.6_16G29.kdk
// KDK_10.13_17A360a.kdk through KDK_10.13.6_17G6009.kdk
static const unsigned proc_darwin_16_17_textvp_offset = 0x00000000000002c0;

// KDK_10.14_18A377a.kdk through KDK_10.14.6_18G84.kdk
static const unsigned proc_darwin_18_textvp_offset = 0x00000000000002a8;

// KDK KDK_10.15_19A536g.kdk through KDK_10.15.2_19C39d.kdk
static const unsigned proc_darwin_19_textvp_offset = 0x00000000000002c8;

static vnode_t proc_get_textvp(proc_t p)
{
	// struct proc is opaque, and changes from version to version.
	
	vnode_t result = NULLVP;
	switch (version_major)
	{
	case 16: // Sierra
	case 17: // High Sierra
		memcpy(&result, ((char*)p) + proc_darwin_16_17_textvp_offset, sizeof(result));
		break;

	case 18: // Mojave
		memcpy(&result, ((char*)p) + proc_darwin_18_textvp_offset, sizeof(result));
		break;

	case 19: // Catalina
		if (version_minor <= 2) // Who knows what the future holds?
			memcpy(&result, ((char*)p) + proc_darwin_19_textvp_offset, sizeof(result));
		break;
	};
	
	if (0 == vnode_get(result))
		return result;
	else
		return NULLVP;
}

// KDK_10.13_17A360a.kdk through KDK_10.13.6_17G6009.kdk
// KDK_10.14_18A377a.kdk through KDK_10.14.6_18G84.kdk
// KDK KDK_10.15_19A536g.kdk through KDK_10.15.2_19C39d.kdk
static const unsigned csblob_darwin_17_19_csb_signer_type_offset = 0x00000000000000a0;
// KDK_10.12_16A239m.kdk through KDK_10.12.6_16G29.kdk
static const unsigned csblob_darwin_16_19_csb_flags_offset = 0x000000000000000c;
static const unsigned csblob_darwin_16_19_csb_teamid_offset = 0x0000000000000088;

static struct cs_info csblob_get_info(const struct cs_blob* cs)
{
	struct cs_info info = {};
	// struct cs_blob is opaque, and changes from version to version.
	switch (version_major)
	{
	case 19: // Catalina
		if (version_minor > 2) // Who knows what the future holds?
			break;
	case 18: // Mojave
	case 17: // High Sierra
		info.signer_type_set = true;
		memcpy(&info.signer_type, ((const char*)cs) + csblob_darwin_17_19_csb_signer_type_offset, sizeof(info.signer_type));
	
	case 16: // Sierra
		info.flags_set = true;
		memcpy(&info.flags, ((const char*)cs) + csblob_darwin_16_19_csb_flags_offset, sizeof(info.flags));

		info.teamid_set = true;
		memcpy(&info.teamid, ((const char*)cs) + csblob_darwin_16_19_csb_teamid_offset, sizeof(info.teamid));
		
		break;
	}
	
	return info;
}

