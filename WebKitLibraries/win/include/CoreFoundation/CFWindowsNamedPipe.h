/*	CFWindowsNamedPipe.h
	Copyright (c) 2005-2009, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFWINDOWSNAMEDPIPE__)
#define __COREFOUNDATION_CFWINDOWSNAMEDPIPE__ 1

#if TARGET_OS_WIN32

#include <CoreFoundation/CFRunLoop.h>

CF_EXTERN_C_BEGIN

typedef struct __CFWindowsNamedPipe * CFWindowsNamedPipeRef;

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
} CFWindowsNamedPipeContext;

typedef void (*CFWindowsNamedPipeCallBack)(CFWindowsNamedPipeRef port, void *msg, CFIndex size, void *info);
typedef void (*CFWindowsNamedPipeInvalidationCallBack)(CFWindowsNamedPipeRef port, void *info);

CF_EXPORT CFTypeID	CFWindowsNamedPipeGetTypeID(void);

CF_EXPORT CFStringRef	CFWindowsNamedPipeGetName(CFWindowsNamedPipeRef pipe);

CF_EXPORT CFWindowsNamedPipeRef	CFWindowsNamedPipeCreateConnectToName(CFAllocatorRef allocator, CFStringRef name, CFWindowsNamedPipeCallBack callout, CFWindowsNamedPipeContext *context, Boolean *shouldFreeInfo);
CF_EXPORT CFWindowsNamedPipeRef	CFWindowsNamedPipeCreateWithName(CFAllocatorRef allocator, CFStringRef name, CFWindowsNamedPipeCallBack callout, CFWindowsNamedPipeContext *context, Boolean *shouldFreeInfo);

CF_EXPORT void *    CFWindowsNamedPipeGetHandle(CFWindowsNamedPipeRef pipe);
CF_EXPORT Boolean    CFWindowsNamedPipeGetOverlappedRead(void * pipeHandle, void *overlapped); // pointer to OVERLAPPED
CF_EXPORT Boolean    CFWindowsNamedPipeGetOverlappedWrite(void * pipeHandle, void *overlapped); // pointer to OVERLAPPED
        
CF_EXPORT void		CFWindowsNamedPipeGetContext(CFWindowsNamedPipeRef pipe, CFWindowsNamedPipeContext *context);
CF_EXPORT void		CFWindowsNamedPipeInvalidate(CFWindowsNamedPipeRef pipe);
CF_EXPORT Boolean	CFWindowsNamedPipeIsValid(CFWindowsNamedPipeRef pipe);
CF_EXPORT void		CFWindowsNamedPipeSetInvalidationCallBack(CFWindowsNamedPipeRef port, CFWindowsNamedPipeInvalidationCallBack callout);

CF_EXPORT CFRunLoopSourceRef	CFWindowsNamedPipeCreateRunLoopSource(CFAllocatorRef allocator, CFWindowsNamedPipeRef port, CFIndex order);

CF_EXPORT void      CFWindowsNamedPipeSetBlockOnWrite(CFWindowsNamedPipeRef pipe, Boolean block);

CF_EXTERN_C_END

#endif

#endif /* ! __COREFOUNDATION_CFWINDOWSNAMEDPIPE__ */

