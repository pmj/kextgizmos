//
//  osobject_retaincount.h
//  Kextgizmos
//
//  Created by Phil Dennis-Jordan on 21/07/2016.
//  Copyright (c) 2016 Phil & Laura Dennis Jordan. All rights reserved.
//

#pragma once

#include <libkern/c++/OSObject.h>
#include <libkern/OSDebug.h>

#define DJ_OSOBJECT_RETAINCOUNT(CLASS, SUPERCLASS) \
	virtual void taggedRelease(const void * tag, const int freeWhen) const override \
	{ \
		int count = this->getRetainCount(); \
		void* bt[10] = { 0 }; \
		OSBacktrace(bt, 10); \
		kprintf(#CLASS " %p release: %2d -> %2d (freeWhen = %d, tag = %p) trace: %p  %p  %p  %p  %p  %p  %p  %p  %p  %p\n", \
			this, count, count - 1, freeWhen, tag, \
			bt[0], bt[1], bt[2], bt[3], bt[4], bt[5], \
			bt[6], bt[7], bt[8], bt[9]); \
		OSReportWithBacktrace(#CLASS " %p release: %2d -> %2d (freeWhen = %d, tag = %p)\n", this, count, count - 1, freeWhen, tag); \
		this->SUPERCLASS::taggedRelease(tag, freeWhen); \
	} \
	virtual void taggedRetain(const void * tag = 0) const override \
	{ \
		int count = this->getRetainCount(); \
		void* bt[10] = { 0 }; \
		OSBacktrace(bt, 10); \
		kprintf(#CLASS" %p retain:  %2d -> %2d (tag = %p) trace: %p  %p  %p  %p  %p  %p  %p  %p  %p  %p\n", \
			this, count, count + 1, tag, \
			bt[0], bt[1], bt[2], bt[3], bt[4], bt[5], \
			bt[6], bt[7], bt[8], bt[9]); \
		OSReportWithBacktrace(#CLASS " %p retain:  %2d -> %2d (tag = %p)\n", this, count, count + 1, tag); \
		this->SUPERCLASS::taggedRetain(tag); \
	}
