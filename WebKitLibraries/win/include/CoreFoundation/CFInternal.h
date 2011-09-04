/*
 * Copyright (c) 2008-2009 Brent Fulgham <bfulgham@gmail.org>.  All rights reserved.
 * Copyright (c) 2009 Grant Erickson <gerickson@nuovations.com>. All rights reserved.
 *
 * This source code is a modified version of the CoreFoundation sources released by Apple Inc. under
 * the terms of the APSL version 2.0 (see below).
 *
 * For information about changes from the original Apple source release can be found by reviewing the
 * source control system for the project at https://sourceforge.net/svn/?group_id=246198.
 *
 * The original license information is as follows:
 * 
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
/*	CFInternal.h
	Copyright (c) 1998-2007, Apple Inc. All rights reserved.
*/

/*
        NOT TO BE USED OUTSIDE CF!
*/

#if !CF_BUILDING_CF
    #error The header file CFInternal.h is for the exclusive use of CoreFoundation. No other project should include it.
#endif

#if !defined(__COREFOUNDATION_CFINTERNAL__)
#define __COREFOUNDATION_CFINTERNAL__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFStorage.h>
#include "CFLogUtilities.h"
#include "CFRuntime.h"
#if defined(__MACH__)
#include <xlocale.h>
#include <mach/mach_time.h>
#elif DEPLOYMENT_TARGET_WINDOWS
#include <windows.h>
#include <malloc.h>
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
#include <sys/time.h>
#include <pthread.h>
#endif
#include <limits.h>
#include "auto_stubs.h"
#ifdef DEPLOYMENT_TARGET_MACOSX
#include <libkern/OSAtomic.h>
#endif //DEPLOYMENT_TARGET_MACOSX

#if !(DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_FREEBSD)
extern int flsl(long mask);
#endif

#if DEPLOYMENT_TARGET_WINDOWS
#define __PRETTY_FUNCTION__ __FUNCTION__
#define __builtin_expect(expr, val) (expr)
#endif

#if defined(__BIG_ENDIAN__)
#define __CF_BIG_ENDIAN__ 1
#define __CF_LITTLE_ENDIAN__ 0
#endif

#if defined(__LITTLE_ENDIAN__)
#define __CF_LITTLE_ENDIAN__ 1
#define __CF_BIG_ENDIAN__ 0
#endif


#include "ForFoundationOnly.h"

CF_EXPORT const char *_CFProcessName(void);
CF_EXPORT CFStringRef _CFProcessNameString(void);

CF_EXPORT Boolean _CFIsCFM(void);

CF_EXPORT Boolean _CFGetCurrentDirectory(char *path, int maxlen);

CF_EXPORT CFStringRef _CFGetUserName(void);

CF_EXPORT CFArrayRef _CFGetWindowsBinaryDirectories(void);

CF_EXPORT CFStringRef _CFStringCreateHostName(void);

#if DEPLOYMENT_TARGET_MACOSX
CF_EXPORT void _CFMachPortInstallNotifyPort(CFRunLoopRef rl, CFStringRef mode);
#endif

#if defined(__ppc__) || defined(__ppc64__) || defined(__powerpc__)
    #define HALT asm __volatile__("trap")
#elif defined(__i386__) || defined(__x86_64__)
    #if defined(__GNUC__)
        #define HALT __asm__ __volatile__("int3")
    #elif defined(_MSC_VER)
        #define HALT __asm int 3;
    #else
        #error Compiler not supported
    #endif
#endif

#if defined(DEBUG)
    #define __CFAssert(cond, prio, desc, a1, a2, a3, a4, a5)	\
	do {			\
	    if (!(cond)) {	\
		CFLog(prio, CFSTR(desc), a1, a2, a3, a4, a5); \
		/* HALT; */		\
	    }			\
	} while (0)
#else
    #define __CFAssert(cond, prio, desc, a1, a2, a3, a4, a5)	\
	do {} while (0)
#endif

#define CFAssert(condition, priority, description)			\
    __CFAssert((condition), (priority), description, 0, 0, 0, 0, 0)
#define CFAssert1(condition, priority, description, a1)			\
    __CFAssert((condition), (priority), description, (a1), 0, 0, 0, 0)
#define CFAssert2(condition, priority, description, a1, a2)		\
    __CFAssert((condition), (priority), description, (a1), (a2), 0, 0, 0)
#define CFAssert3(condition, priority, description, a1, a2, a3)		\
    __CFAssert((condition), (priority), description, (a1), (a2), (a3), 0, 0)
#define CFAssert4(condition, priority, description, a1, a2, a3, a4)	\
    __CFAssert((condition), (priority), description, (a1), (a2), (a3), (a4), 0)

#define __kCFLogAssertion	3

#if defined(DEBUG)
extern void __CFGenericValidateType_(CFTypeRef cf, CFTypeID type, const char *func);
#define __CFGenericValidateType(cf, type) __CFGenericValidateType_(cf, type, __PRETTY_FUNCTION__)
#else
#define __CFGenericValidateType(cf, type) ((void)0)
#endif

#define CF_INFO_BITS (!!(__CF_BIG_ENDIAN__) * 3)
#define CF_RC_BITS (!!(__CF_LITTLE_ENDIAN__) * 3)

/* Bit manipulation macros */
/* Bits are numbered from 31 on left to 0 on right */
/* May or may not work if you use them on bitfields in types other than UInt32, bitfields the full width of a UInt32, or anything else for which they were not designed. */
/* In the following, N1 and N2 specify an inclusive range N2..N1 with N1 >= N2 */
#define __CFBitfieldMask(N1, N2)	((((UInt32)~0UL) << (31UL - (N1) + (N2))) >> (31UL - N1))
#define __CFBitfieldGetValue(V, N1, N2)	(((V) & __CFBitfieldMask(N1, N2)) >> (N2))
#define __CFBitfieldSetValue(V, N1, N2, X)	((V) = ((V) & ~__CFBitfieldMask(N1, N2)) | (((X) << (N2)) & __CFBitfieldMask(N1, N2)))
#define __CFBitfieldMaxValue(N1, N2)	__CFBitfieldGetValue(0xFFFFFFFFUL, (N1), (N2))

#define __CFBitIsSet(V, N)  (((V) & (1UL << (N))) != 0)
#define __CFBitSet(V, N)  ((V) |= (1UL << (N)))
#define __CFBitClear(V, N)  ((V) &= ~(1UL << (N)))

#if DEPLOYMENT_TARGET_WINDOWS
CF_INLINE bool _CFAtomicCompareAndSwap32Barrier(int32_t oldValue, int32_t newValue, volatile int32_t *theValue) {
    int32_t actualOldValue = InterlockedCompareExchange((volatile LONG *)theValue, newValue, oldValue);
    return actualOldValue == oldValue ? true : false;
}
CF_INLINE bool _CFAtomicCompareAndSwapPtrBarrier(void* oldValue, void* newValue, void* volatile *theValue) {
	void *actualOldValue = InterlockedCompareExchangePointer((volatile PVOID*)theValue, newValue, (PVOID)oldValue);
	return actualOldValue == oldValue ? true : false;
}
CF_INLINE int32_t _CFAtomicIncrement32(volatile int32_t *theValue) {
	return (unsigned int)InterlockedIncrement((volatile LONG*)theValue);
}
CF_INLINE void _CFMemoryBarrier(void) {
	MemoryBarrier();
}
#elif DEPLOYMENT_TARGET_MACOSX
CF_INLINE bool _CFAtomicCompareAndSwap32Barrier(int32_t __oldValue, int32_t __newValue, volatile int32_t *__theValue) {
	return OSAtomicCompareAndSwap32Barrier(__oldValue, __newValue, __theValue);
}
CF_INLINE bool _CFAtomicCompareAndSwapPtrBarrier(void* __oldValue, void* __newValue, void* volatile *__theValue) {
	return OSAtomicCompareAndSwapPtrBarrier(__oldValue, __newValue, __theValue);
}
CF_INLINE int32_t _CFAtomicIncrement32(volatile int32_t *theValue) {
	return OSAtomicIncrement32(theValue);
}
CF_INLINE void _CFMemoryBarrier(void) {
	OSMemoryBarrier();
}
#elif DEPLOYMENT_TARGET_LINUX
// Simply leverage GCC's atomic built-ins (see http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html)
CF_INLINE bool _CFAtomicCompareAndSwap32Barrier(int32_t __oldValue, int32_t __newValue, volatile int32_t *__theValue) {
	return __sync_bool_compare_and_swap(__theValue, __oldValue, __newValue);
}
CF_INLINE bool _CFAtomicCompareAndSwapPtrBarrier(void* __oldValue, void* __newValue, void* volatile *__theValue) {
	return __sync_bool_compare_and_swap(__theValue, __oldValue, __newValue);
}
CF_INLINE int32_t _CFAtomicIncrement32(volatile int32_t *theValue) {
	return __sync_fetch_and_add(theValue, 1);
}
CF_INLINE void _CFMemoryBarrier(void) {
	__sync_synchronize();
}
#else
#error "Don't know how to perform atomic operations."
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_FREEBSD
#define	_CFIsSetUgid()	issetugid()
#elif DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_SOLARIS
#include <unistd.h>
CF_INLINE bool _CFIsSetUgid(void) {
	return (getuid() != geteuid() || getgid() != getegid());
}
#elif DEPLOYMENT_TARGET_WINDOWS
#define	_CFIsSetUgid()	0
#else
#error "Don't know how to determine a setuid or setgid executable."
#endif /* DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_FREEBSD */


typedef struct ___CFThreadSpecificData {
    void *_unused1;
    void *_allocator;
#if DEPLOYMENT_TARGET_WINDOWS
    HHOOK _messageHook;
#endif
// If you add things to this struct, add cleanup to __CFFinalizeThreadData()
} __CFThreadSpecificData;

extern __CFThreadSpecificData *__CFGetThreadSpecificData(void);
__private_extern__ void __CFFinalizeThreadData(void *arg);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
extern pthread_key_t __CFTSDKey;
#elif DEPLOYMENT_TARGET_WINDOWS
extern DWORD __CFTSDKey;
#endif

//extern void *pthread_getspecific(pthread_key_t key);

CF_INLINE __CFThreadSpecificData *__CFGetThreadSpecificData_inline(void) {
#if DEPLOYMENT_TARGET_MACOSX|| DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
    __CFThreadSpecificData *data = (__CFThreadSpecificData *)pthread_getspecific(__CFTSDKey);
    return data ? data : __CFGetThreadSpecificData();
#elif DEPLOYMENT_TARGET_WINDOWS
    __CFThreadSpecificData *data = (__CFThreadSpecificData *)TlsGetValue(__CFTSDKey);
    return data ? data : __CFGetThreadSpecificData();
#endif
}

#define __kCFAllocatorTypeID_CONST	2

CF_INLINE CFAllocatorRef __CFGetDefaultAllocator(void) {
    CFAllocatorRef allocator = (CFAllocatorRef)__CFGetThreadSpecificData_inline()->_allocator;
    if (NULL == allocator) {
	allocator = kCFAllocatorSystemDefault;
    }
    return allocator;
}

extern CFTypeID __CFGenericTypeID(const void *cf);

// This should only be used in CF types, not toll-free bridged objects!
// It should not be used with CFAllocator arguments!
// Use CFGetAllocator() in the general case, and this inline function in a few limited (but often called) situations.
CF_INLINE CFAllocatorRef __CFGetAllocator(CFTypeRef cf) {	// !!! Use with CF types only, and NOT WITH CFAllocator!
    CFAssert1(__kCFAllocatorTypeID_CONST != __CFGenericTypeID(cf), __kCFLogAssertion, "__CFGetAllocator(): CFAllocator argument", cf);
    if (__builtin_expect(__CFBitfieldGetValue(((const CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 7, 7), 1)) {
	return kCFAllocatorSystemDefault;
    }
    return *(CFAllocatorRef *)((char *)cf - sizeof(CFAllocatorRef));
}

// Don't define a __CFGetCurrentRunLoop(), because even internal clients should go through the real one


#if !defined(LLONG_MAX)
    #if defined(_I64_MAX)
	#define LLONG_MAX	_I64_MAX
    #else
	#warning Arbitrarily defining LLONG_MAX
       #define LLONG_MAX	(int64_t)9223372036854775807
    #endif
#endif /* !defined(LLONG_MAX) */

#if !defined(LLONG_MIN)
    #if defined(_I64_MIN)
	#define LLONG_MIN	_I64_MIN
    #else
	#warning Arbitrarily defining LLONG_MIN
	#define LLONG_MIN	(-LLONG_MAX - (int64_t)1)
    #endif
#endif /* !defined(LLONG_MIN) */

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
    #define __CFMin(A,B) ({__typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
    #define __CFMax(A,B) ({__typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __b : __a; })
#else /* __GNUC__ */
    #define __CFMin(A,B) ((A) < (B) ? (A) : (B))
    #define __CFMax(A,B) ((A) > (B) ? (A) : (B))
#endif /* __GNUC__ */

/* Secret CFAllocator hint bits */
#define __kCFAllocatorTempMemory	0x2
#define __kCFAllocatorNoPointers	0x10
#define __kCFAllocatorDoNotRecordEvent	0x100
#define __kCFAllocatorGCScannedMemory 0x200     /* GC:  memory should be scanned. */
#define __kCFAllocatorGCObjectMemory 0x400      /* GC:  memory needs to be finalized. */

CF_INLINE auto_memory_type_t CF_GET_GC_MEMORY_TYPE(CFOptionFlags flags) {
	auto_memory_type_t type = (flags & __kCFAllocatorGCScannedMemory ? 0 : AUTO_UNSCANNED) | (flags & __kCFAllocatorGCObjectMemory ? AUTO_OBJECT : 0);
    return type;
}

CF_EXPORT CFAllocatorRef _CFTemporaryMemoryAllocator(void);

extern SInt64 __CFTimeIntervalToTSR(CFTimeInterval ti);
extern CFTimeInterval __CFTSRToTimeInterval(SInt64 tsr);

extern CFStringRef __CFCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions);

/* result is long long or int, depending on doLonglong
*/
extern Boolean __CFStringScanInteger(CFStringInlineBuffer *buf, CFTypeRef locale, SInt32 *indexPtr, Boolean doLonglong, void *result);
extern Boolean __CFStringScanDouble(CFStringInlineBuffer *buf, CFTypeRef locale, SInt32 *indexPtr, double *resultPtr); 
extern Boolean __CFStringScanHex(CFStringInlineBuffer *buf, SInt32 *indexPtr, unsigned *result);


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_LINUX || (DEPLOYMENT_TARGET_WINDOWS && __GNUC__)

#define STACK_BUFFER_DECL(T, N, C) T N[C];

#elif DEPLOYMENT_TARGET_WINDOWS && _MSC_VER

#define STACK_BUFFER_DECL(T, N, C) T* N = (T*)_alloca((C + 1) * sizeof(T));

#endif


#ifdef __CONSTANT_CFSTRINGS__
#define CONST_STRING_DECL(S, V) const CFStringRef S = (const CFStringRef)__builtin___CFStringMakeConstantString("" V "");
#else

struct CF_CONST_STRING {
    CFRuntimeBase _base;
    uint8_t *_ptr;
    uint32_t _length;
};

extern int __CFConstantStringClassReference[];

/* CFNetwork also has a copy of the CONST_STRING_DECL macro (for use on platforms without constant string support in cc); please warn cfnetwork-core@group.apple.com of any necessary changes to this macro. -- REW, 1/28/2002 */
#if DEPLOYMENT_TARGET_WINDOWS
#define ___WindowsConstantStringClassReference (uintptr_t)&__CFConstantStringClassReference
#else
#define ___WindowsConstantStringClassReference NULL
#endif

#if __CF_BIG_ENDIAN__
#define CONST_STRING_DECL(S, V)			\
	static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, {0x00, 0x00, 0x07, 0xc8}}, (uint8_t *)V, sizeof(V) - 1}; \
const CFStringRef S = (CFStringRef) & __ ## S ## __;
#elif !DEPLOYMENT_TARGET_WINDOWS || (DEPLOYMENT_TARGET_WINDOWS && defined(__GNUC__))
#define CONST_STRING_DECL(S, V)			\
	static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, {0xc8, 0x07, 0x00, 0x00}}, (uint8_t *)V, sizeof(V) - 1}; \
const CFStringRef S = (CFStringRef) & __ ## S ## __;
#elif DEPLOYMENT_TARGET_WINDOWS
#define CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, {0xc8, 0x07, 0x00, 0x00}},(uint8_t *) V, sizeof(V) - 1}; \
CF_EXPORT const CFStringRef S = (CFStringRef) & __ ## S ## __;

#define CONST_STRING_DECL_EXPORT(S, V)			\
struct CF_CONST_STRING __ ## S ## __ = {{___WindowsConstantStringClassReference, {0xc8, 0x07, 0x00, 0x00}}, (uint8_t *)V, sizeof(V) - 1}; \
CF_EXPORT const CFStringRef S = (CFStringRef) & __ ## S ## __;

#else
#define CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)NULL, {0xc8, 0x07, 0x00, 0x00}},(uint8_t *) V, sizeof(V) - 1}; \
const CFStringRef S = (CFStringRef) & __ ## S ## __;

#define CONST_STRING_DECL_EXPORT(S, V)			\
struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)NULL, {0xc8, 0x07, 0x00, 0x00}}, (uint8_t *)V, sizeof(V) - 1}; \
CF_EXPORT const CFStringRef S = (CFStringRef) & __ ## S ## __;

#endif // __WIN32__
#endif // __BIG_ENDIAN__

#undef ___WindowsConstantStringClassReference

/* Buffer size for file pathname */
#if DEPLOYMENT_TARGET_WINDOWS
    #define CFMaxPathSize ((CFIndex)262)
    #define CFMaxPathLength ((CFIndex)260)
#else
    #define CFMaxPathSize ((CFIndex)1026)
    #define CFMaxPathLength ((CFIndex)1024)
#endif

#define __CFOASafe 0
#define __CFSetLastAllocationEventName(a, b) ((void) 0)

CF_EXPORT CFStringRef _CFCreateLimitedUniqueString(void);

/* Comparators are passed the address of the values; this is somewhat different than CFComparatorFunction is used in public API usually. */
CF_EXPORT CFIndex	CFBSearch(const void *element, CFIndex elementSize, const void *list, CFIndex count, CFComparatorFunction comparator, void *context);

CF_EXPORT CFHashCode	CFHashBytes(UInt8 *bytes, CFIndex length);

CF_EXPORT CFStringEncoding CFStringFileSystemEncoding(void);

__private_extern__ CFStringRef __CFStringCreateImmutableFunnel3(CFAllocatorRef alloc, const void *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean possiblyExternalFormat, Boolean tryToReduceUnicode, Boolean hasLengthByte, Boolean hasNullByte, Boolean noCopy, CFAllocatorRef contentsDeallocator, UInt32 converterFlags);

extern const void *__CFStringCollectionCopy(CFAllocatorRef allocator, const void *ptr);
extern const void *__CFTypeCollectionRetain(CFAllocatorRef allocator, const void *ptr);
extern void __CFTypeCollectionRelease(CFAllocatorRef allocator, const void *ptr);


#if DEPLOYMENT_TARGET_MACOSX

typedef OSSpinLock CFSpinLock_t;

#define CFSpinLockInit OS_SPINLOCK_INIT
#define CF_SPINLOCK_INIT_FOR_STRUCTS(X) (X = CFSpinLockInit)

CF_INLINE void __CFSpinLock(CFSpinLock_t *lockp) {
    OSSpinLockLock(lockp);
}

CF_INLINE void __CFSpinUnlock(CFSpinLock_t *lockp) {
    OSSpinLockUnlock(lockp);
}

#elif DEPLOYMENT_TARGET_WINDOWS

typedef CRITICAL_SECTION CFSpinLock_t;

#define CFSpinLockInit {0}

// For some reason, the {0} initializer does not work when the spinlock is a member of a structure; hence this macro
#define CF_SPINLOCK_INIT_FOR_STRUCTS(X) InitializeCriticalSection(&X)
extern CFSpinLock_t *theLock;
CF_INLINE void __CFSpinLock(CFSpinLock_t *slock) {
    if (NULL == slock->DebugInfo) {
        InitializeCriticalSection(slock);
    }
    EnterCriticalSection(slock);
}

CF_INLINE void __CFSpinUnlock(CFSpinLock_t *lock) {
    LeaveCriticalSection(lock);
}

#elif DEPLOYMENT_TARGET_LINUX

typedef struct __CFSpinLock {
	int init;
	pthread_spinlock_t lock;
} CFSpinLock_t;

#define CFSpinLockInit {0}
#define CF_SPINLOCK_INIT_FOR_STRUCTS(X) do { pthread_spin_init(&X.lock, PTHREAD_PROCESS_PRIVATE); X.init = 1; } while (0)

CF_INLINE void __CFSpinLock(CFSpinLock_t *lockp) {
	if (lockp->init == 0) {
		!pthread_spin_init(&lockp->lock, PTHREAD_PROCESS_PRIVATE) &&
		(lockp->init = 1);
	}
	pthread_spin_lock(&lockp->lock);
}

CF_INLINE void __CFSpinUnlock(CFSpinLock_t *lockp) {
	pthread_spin_unlock(&lockp->lock);
}

#else

#warning CF spin locks not defined for this platform -- CF is not thread-safe
#define __CFSpinLock(A)		do {} while (0)
#define __CFSpinUnlock(A)	do {} while (0)

#endif

#if !defined(CHECK_FOR_FORK)
#define CHECK_FOR_FORK() do { } while (0)
#endif

#if !defined(HAS_FORKED)
#define HAS_FORKED() 0
#endif

#if defined(__svr4__) || defined(__hpux__) || DEPLOYMENT_TARGET_WINDOWS
#include <errno.h>
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
#include <sys/errno.h>
#endif

#define thread_errno() errno
#define thread_set_errno(V) do {errno = (V);} while (0)

extern void *__CFStartSimpleThread(void *func, void *arg);

/* ==================== Simple file access ==================== */
/* For dealing with abstract types.  MF:!!! These ought to be somewhere else and public. */
    
CF_EXPORT CFStringRef _CFCopyExtensionForAbstractType(CFStringRef abstractType);

/* ==================== Simple file access ==================== */
/* These functions all act on a c-strings which must be in the file system encoding. */
    
CF_EXPORT Boolean _CFCreateDirectory(const char *path);
CF_EXPORT Boolean _CFRemoveDirectory(const char *path);
CF_EXPORT Boolean _CFDeleteFile(const char *path);

CF_EXPORT Boolean _CFReadBytesFromFile(CFAllocatorRef alloc, CFURLRef url, void **bytes, CFIndex *length, CFIndex maxLength);
    /* resulting bytes are allocated from alloc which MUST be non-NULL. */
    /* maxLength of zero means the whole file.  Otherwise it sets a limit on the number of bytes read. */

CF_EXPORT Boolean _CFWriteBytesToFile(CFURLRef url, const void *bytes, CFIndex length);
#if DEPLOYMENT_TARGET_MACOSX
CF_EXPORT Boolean _CFWriteBytesToFileWithAtomicity(CFURLRef url, const void *bytes, unsigned int length, SInt32 mode, Boolean atomic);
#endif

CF_EXPORT CFMutableArrayRef _CFContentsOfDirectory(CFAllocatorRef alloc, char *dirPath, void *dirSpec, CFURLRef dirURL, CFStringRef matchingAbstractType);
    /* On Mac OS 8/9, one of dirSpec, dirPath and dirURL must be non-NULL */
    /* On all other platforms, one of path and dirURL must be non-NULL */
    /* If both are present, they are assumed to be in-synch; that is, they both refer to the same directory.  */
    /* alloc may be NULL */
    /* return value is CFArray of CFURLs */

CF_EXPORT SInt32 _CFGetFileProperties(CFAllocatorRef alloc, CFURLRef pathURL, Boolean *exists, SInt32 *posixMode, SInt64 *size, CFDateRef *modTime, SInt32 *ownerID, CFArrayRef *dirContents);
    /* alloc may be NULL */
    /* any of exists, posixMode, size, modTime, and dirContents can be NULL.  Usually it is not a good idea to pass NULL for exists, since interpretting the other values sometimes requires that you know whether the file existed or not.  Except for dirContents, it is pretty cheap to compute any of these things as loing as one of them must be computed. */


/* ==================== Simple path manipulation ==================== */
/* These functions all act on a UniChar buffers. */

CF_EXPORT Boolean _CFIsAbsolutePath(UniChar *unichars, CFIndex length);
CF_EXPORT Boolean _CFStripTrailingPathSlashes(UniChar *unichars, CFIndex *length);
CF_EXPORT Boolean _CFAppendPathComponent(UniChar *unichars, CFIndex *length, CFIndex maxLength, UniChar *component, CFIndex componentLength);
CF_EXPORT Boolean _CFAppendPathExtension(UniChar *unichars, CFIndex *length, CFIndex maxLength, UniChar *extension, CFIndex extensionLength);
CF_EXPORT Boolean _CFTransmutePathSlashes(UniChar *unichars, CFIndex *length, UniChar replSlash);
CF_EXPORT CFIndex _CFStartOfLastPathComponent(UniChar *unichars, CFIndex length);
CF_EXPORT CFIndex _CFLengthAfterDeletingLastPathComponent(UniChar *unichars, CFIndex length);
CF_EXPORT CFIndex _CFStartOfPathExtension(UniChar *unichars, CFIndex length);
CF_EXPORT CFIndex _CFLengthAfterDeletingPathExtension(UniChar *unichars, CFIndex length);

#ifdef COCOTRON 

extern SEL (*__CFGetObjCSelector)(const char *);
extern void* (*__CFSendObjCMsg)(const void*, SEL, ...);
extern void* __CFISAForTypeID(CFTypeID typeid);

#define CF_IS_OBJC(typeID, obj)	(((CFRuntimeBase *)(obj))->_cfisa != NULL && ((CFRuntimeBase *)(obj))->_cfisa != __CFISAForTypeID(typeID))

#define CF_OBJC_VOIDCALL0(obj, sel) \
        {void (*func)(const void *, SEL) = (void *)__CFSendObjCMsg; \
        static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
        func((const void *)obj, s);}
#define CF_OBJC_VOIDCALL1(obj, sel, a1) \
        {void (*func)(const void *, SEL, ...) = (void *)__CFSendObjCMsg; \
        static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
        func((const void *)obj, s, (a1));}
#define CF_OBJC_VOIDCALL2(obj, sel, a1, a2) \
        {void (*func)(const void *, SEL, ...) = (void *)__CFSendObjCMsg; \
        static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
        func((const void *)obj, s, (a1), (a2));}

// Invoke an ObjC method, leaving the result in "retvar".
// Assumes CF_IS_OBJC has already been checked.
#define CF_OBJC_CALL0(rettype, retvar, obj, sel) \
        {rettype (*func)(const void *, SEL) = (void *)__CFSendObjCMsg; \
        static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
        retvar = func((const void *)obj, s);}
#define CF_OBJC_CALL1(rettype, retvar, obj, sel, a1) \
        {rettype (*func)(const void *, SEL, ...) = (void *)__CFSendObjCMsg; \
        static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
        retvar = func((const void *)obj, s, (a1));}
#define CF_OBJC_CALL2(rettype, retvar, obj, sel, a1, a2) \
        {rettype (*func)(const void *, SEL, ...) = (void *)__CFSendObjCMsg; \
        static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
        retvar = func((const void *)obj, s, (a1), (a2));}

#define CF_OBJC_FUNCDISPATCH0(typeID, rettype, obj, sel) \
	if (CF_IS_OBJC(typeID, obj)) \
	{rettype (*func)(id, SEL) = (void *)__CFSendObjCMsg; \
	static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
	return func((id)obj, s);}
#define CF_OBJC_FUNCDISPATCH1(typeID, rettype, obj, sel, a1) \
	if (CF_IS_OBJC(typeID, obj)) \
	{rettype (*func)(id, SEL, ...) = (void *)__CFSendObjCMsg; \
	static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
	return func((id)obj, s, (a1));}
#define CF_OBJC_FUNCDISPATCH2(typeID, rettype, obj, sel, a1, a2) \
	if (CF_IS_OBJC(typeID, obj)) \
	{rettype (*func)(id, SEL, ...) = (void *)__CFSendObjCMsg; \
	static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
	return func((id)obj, s, (a1), (a2));}
#define CF_OBJC_FUNCDISPATCH3(typeID, rettype, obj, sel, a1, a2, a3) \
	if (CF_IS_OBJC(typeID, obj)) \
	{rettype (*func)(id, SEL, ...) = (void *)__CFSendObjCMsg; \
	static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
	return func((id)obj, s, (a1), (a2), (a3));}
#define CF_OBJC_FUNCDISPATCH4(typeID, rettype, obj, sel, a1, a2, a3, a4) \
	if (CF_IS_OBJC(typeID, obj)) \
	{rettype (*func)(id, SEL, ...) = (void *)__CFSendObjCMsg; \
	static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
	return func((id)obj, s, (a1), (a2), (a3), (a4));}
#define CF_OBJC_FUNCDISPATCH5(typeID, rettype, obj, sel, a1, a2, a3, a4, a5) \
	if (CF_IS_OBJC(typeID, obj)) \
	{rettype (*func)(id, SEL, ...) = (void *)__CFSendObjCMsg; \
	static SEL s = NULL; if (!s) s = __CFGetObjCSelector(sel); \
	return func((id)obj, s, (a1), (a2), (a3), (a4), (a5));}

#else

#define CF_IS_OBJC(typeID, obj)	(false)

#define CF_OBJC_VOIDCALL0(obj, sel)
#define CF_OBJC_VOIDCALL1(obj, sel, a1)
#define CF_OBJC_VOIDCALL2(obj, sel, a1, a2)

#define CF_OBJC_CALL0(rettype, retvar, obj, sel)
#define CF_OBJC_CALL1(rettype, retvar, obj, sel, a1)
#define CF_OBJC_CALL2(rettype, retvar, obj, sel, a1, a2)

#if defined DEPLOYMENT_TARGET_WINDOWS
#define CF_OBJC_FUNCDISPATCH0(typeID, rettype, obj, sel) ((void)0)
#define CF_OBJC_FUNCDISPATCH1(typeID, rettype, obj, sel, a1) ((void)0)
#define CF_OBJC_FUNCDISPATCH2(typeID, rettype, obj, sel, a1, a2) ((void)0)
#define CF_OBJC_FUNCDISPATCH3(typeID, rettype, obj, sel, a1, a2, a3) ((void)0)
#define CF_OBJC_FUNCDISPATCH4(typeID, rettype, obj, sel, a1, a2, a3, a4) ((void)0)
#define CF_OBJC_FUNCDISPATCH5(typeID, rettype, obj, sel, a1, a2, a3, a4, a5) ((void)0)
#else
#define CF_OBJC_FUNCDISPATCH0(typeID, rettype, obj, sel)
#define CF_OBJC_FUNCDISPATCH1(typeID, rettype, obj, sel, a1)
#define CF_OBJC_FUNCDISPATCH2(typeID, rettype, obj, sel, a1, a2)
#define CF_OBJC_FUNCDISPATCH3(typeID, rettype, obj, sel, a1, a2, a3)
#define CF_OBJC_FUNCDISPATCH4(typeID, rettype, obj, sel, a1, a2, a3, a4)
#define CF_OBJC_FUNCDISPATCH5(typeID, rettype, obj, sel, a1, a2, a3, a4, a5)
#endif

#define __CFISAForTypeID(x) (0)

#endif

#define __CFMaxRuntimeTypes	65535

/* See comments in CFBase.c
*/
#if DEPLOYMENT_TARGET_MACOSX && defined(__ppc__)
extern void __CF_FAULT_CALLBACK(void **ptr);
extern void *__CF_INVOKE_CALLBACK(void *, ...);
#define FAULT_CALLBACK(V) __CF_FAULT_CALLBACK(V)
#define INVOKE_CALLBACK1(P, A) (__CF_INVOKE_CALLBACK(P, A))
#define INVOKE_CALLBACK2(P, A, B) (__CF_INVOKE_CALLBACK(P, A, B))
#define INVOKE_CALLBACK3(P, A, B, C) (__CF_INVOKE_CALLBACK(P, A, B, C))
#define INVOKE_CALLBACK4(P, A, B, C, D) (__CF_INVOKE_CALLBACK(P, A, B, C, D))
#define INVOKE_CALLBACK5(P, A, B, C, D, E) (__CF_INVOKE_CALLBACK(P, A, B, C, D, E))
#define UNFAULT_CALLBACK(V) do { V = (void *)((uintptr_t)V & ~0x3); } while (0)
#else
#define FAULT_CALLBACK(V)
#define INVOKE_CALLBACK1(P, A) (P)(A)
#define INVOKE_CALLBACK2(P, A, B) (P)(A, B)
#define INVOKE_CALLBACK3(P, A, B, C) (P)(A, B, C)
#define INVOKE_CALLBACK4(P, A, B, C, D) (P)(A, B, C, D)
#define INVOKE_CALLBACK5(P, A, B, C, D, E) (P)(A, B, C, D, E)
#define UNFAULT_CALLBACK(V) do { } while (0)
#endif

/* For the support of functionality which needs CarbonCore or other frameworks */
// These macros define an upcall or weak "symbol-lookup" wrapper function.
// The parameters are:
//   R : the return type of the function
//   N : the name of the function (in the other library)
//   P : the parenthesized parameter list of the function
//   A : the parenthesized actual argument list to be passed
//  opt: a fifth optional argument can be passed in which is the
//       return value of the wrapper when the function cannot be
//       found; should be of type R, & can be a function call
// The name of the resulting wrapper function is:
//    __CFCarbonCore_N (where N is the second parameter)
//    __CFNetwork_N (where N is the second parameter)
//
// Example:
//   DEFINE_WEAK_CARBONCORE_FUNC(void, DisposeHandle, (Handle h), (h))
//

#if DEPLOYMENT_TARGET_MACOSX

extern void *__CFLookupCFNetworkFunction(const char *name);

#define DEFINE_WEAK_CFNETWORK_FUNC(R, N, P, A, ...)	\
 static R __CFNetwork_ ## N P {				\
 typedef R (*CALL_FUN_TYPE ## N) P;    \
 static CALL_FUN_TYPE ## N dyfunc = (CALL_FUN_TYPE ## N)(~(uintptr_t)0);	\
 if (((CALL_FUN_TYPE ## N)(~(uintptr_t)0)) == dyfunc) {		\
 dyfunc = (CALL_FUN_TYPE ## N)__CFLookupCFNetworkFunction(#N); }	\
 if (dyfunc) { return dyfunc A ; }			\
 return __VA_ARGS__ ;				\
 }
 
#else

#define DEFINE_WEAK_CFNETWORK_FUNC(R, N, P, A, ...)

#endif


#if !defined(DEFINE_WEAK_CARBONCORE_FUNC)
#define DEFINE_WEAK_CARBONCORE_FUNC(R, N, P, A, ...)
#endif


__private_extern__ CFArrayRef _CFBundleCopyUserLanguages(Boolean useBackstops);

/* GC related internal SPIs. */
extern malloc_zone_t *__CFCollectableZone;

/* !!! Avoid #importing objc.h; e.g. converting this to a .m file */
struct __objcFastEnumerationStateEquivalent {
    unsigned long state;
    unsigned long *itemsPtr;
    unsigned long *mutationsPtr;
    unsigned long extra[5];
};

unsigned long _CFStorageFastEnumeration(CFStorageRef storage, struct __objcFastEnumerationStateEquivalent *state, void *stackbuffer, unsigned long count);

#if DEPLOYMENT_TARGET_MACOSX
// Allocate an id[count], new slots are nil
extern void *__CFAllocateObjectArray(unsigned long count);
extern void *__CFReallocateObjectArray(id *array, unsigned long count);
extern void __CFFreeObjectArray(id *array);

// check against LONG_MAX to catch negative numbers
#define new_id_array(N, C) \
        size_t N ## _count__ = (C); \
        if (N ## _count__ > LONG_MAX) { \
	    id rr = [objc_lookUpClass("NSString") stringWithFormat:@"*** attempt to create a temporary id buffer which is too large or with a negative count (%lu) -- possibly data is corrupt", N ## _count__]; \
	    @throw [NSException exceptionWithName:NSGenericException reason:rr userInfo:nil]; \
	} \
        NSInteger N ## _is_stack__ = (N ## _count__ <= 256); \
        id N ## _buffer__[N ## _is_stack__ ? N ## _count__ : 0]; \
        if (N ## _is_stack__) memset(N ## _buffer__, 0, sizeof(N ## _buffer__)); \
        id * N = N ## _is_stack__ ? N ## _buffer__ : __CFAllocateObjectArray(N ## _count__); \
        if (! N) { \
	    id rr = [objc_lookUpClass("NSString") stringWithFormat:@"*** attempt to create a temporary id buffer of length (%lu) failed", N ## _count__]; \
	    @throw [NSException exceptionWithName:NSMallocException reason:rr userInfo:nil]; \
	} \
        do {} while (0)

#define free_id_array(N) \
        if (! N ## _is_stack__) __CFFreeObjectArray(N)

extern void *__CFFullMethodName(Class cls, id obj, SEL sel);
extern void *__CFExceptionProem(id obj, SEL sel);
extern void __CFRequireConcreteImplementation(Class absClass, id obj, SEL sel);
#endif

#endif /* ! __COREFOUNDATION_CFINTERNAL__ */

