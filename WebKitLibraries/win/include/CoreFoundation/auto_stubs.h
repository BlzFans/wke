/*
 * Copyright (c) 2008-2009 Brent Fulgham <bfulgham@gmail.org>.  All rights reserved.
 *
 * This source code is a modified version of the CoreFoundation sources released by Apple Inc. under
 * the terms of the APSL version 2.0 (see below).
 *
 * For information about changes from the original Apple source release can be found by reviewing the
 * source control system for the project at https://sourceforge.net/svn/?group_id=246198.
 *
 * The original license information is as follows:
 * 
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*	auto_stubs.h
	Copyright 2005-2007, Apple Inc. All rights reserved.
*/

#if !defined(AUTO_STUBS_H)
#define AUTO_STUBS_H 1

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if !defined(__OBJC__)
typedef void* id;
typedef void* Class;
#define OBJC_INLINE
#define YES TRUE
#define NO FALSE
#endif
static __inline int flsl( long mask ) {
    int idx = 0;
	while (mask != 0) mask = (unsigned long)mask >> 1, idx++;
	return idx;
}
#if DEPLOYMENT_TARGET_MACOSX
#include <malloc/malloc.h>
#endif

#if defined(__GCC__)
#include <objc/objc.h>
#endif

/* Stubs for functions in libauto. */

#if !DEPLOYMENT_TARGET_MACOSX
typedef unsigned long malloc_zone_t;
typedef unsigned char boolean_t;
#endif

typedef malloc_zone_t auto_zone_t;

enum { AUTO_TYPE_UNKNOWN = -1, AUTO_UNSCANNED = 1, AUTO_OBJECT = 2, AUTO_MEMORY_SCANNED = 0, AUTO_MEMORY_UNSCANNED = AUTO_UNSCANNED, AUTO_OBJECT_SCANNED = AUTO_OBJECT, AUTO_OBJECT_UNSCANNED = AUTO_OBJECT | AUTO_UNSCANNED };
typedef unsigned long auto_memory_type_t;

CF_INLINE void *auto_zone(void) { return 0; }
CF_INLINE void *auto_zone_allocate_object(void *zone, size_t size, auto_memory_type_t type, boolean_t rc, boolean_t clear) { return 0; }
CF_INLINE const void *auto_zone_base_pointer(void *zone, const void *ptr) { return 0; }
CF_INLINE void auto_zone_retain(void *zone, void *ptr) {}
CF_INLINE unsigned int auto_zone_release(void *zone, void *ptr) { return 0; }
CF_INLINE unsigned int auto_zone_retain_count(void *zone, const void *ptr) { return 0; }
CF_INLINE void auto_zone_set_layout_type(void *zone, void *ptr, auto_memory_type_t type) {}
CF_INLINE void auto_zone_write_barrier_range(void *zone, void *address, size_t size) {}
CF_INLINE boolean_t auto_zone_is_finalized(void *zone, const void *ptr) { return 0; }
CF_INLINE size_t auto_zone_size(void *zone, const void *ptr) { return 0; }
CF_INLINE void auto_register_weak_reference(void *zone, const void *referent, void **referrer, uintptr_t *counter, void **listHead, void **listElement) {}
CF_INLINE void auto_unregister_weak_reference(void *zone, const void *referent, void **referrer) {}
CF_INLINE void auto_zone_register_thread(void *zone) {}
CF_INLINE void auto_zone_unregister_thread(void *zone) {}
CF_INLINE boolean_t auto_zone_is_valid_pointer(void *zone, const void *ptr) { return 0; }
CF_INLINE auto_memory_type_t auto_zone_get_layout_type(auto_zone_t *zone, void *ptr) { return AUTO_UNSCANNED; }

#if defined(__OBJC__)
CF_INLINE void objc_collect_if_needed(unsigned long options) {}
CF_INLINE Boolean objc_collecting_enabled(void) { return 0; }
CF_INLINE id objc_allocate_object(Class cls, int extra) { return 0; }
CF_INLINE id objc_assign_strongCast(id val, id *dest) { return (*dest = val); }
CF_INLINE id objc_assign_global(id val, id *dest) { return (*dest = val); }
CF_INLINE id objc_assign_ivar(id val, id dest, unsigned int offset) { id *d = (id *)((char *)dest + offset); return (*d = val); }
CF_INLINE void *objc_memmove_collectable(void *dst, const void *src, size_t size) { return memmove(dst, src, size); }
CF_INLINE Boolean objc_is_finalized(void *ptr) { return 0; }
#endif

#endif /* ! AUTO_STUBS_H */

