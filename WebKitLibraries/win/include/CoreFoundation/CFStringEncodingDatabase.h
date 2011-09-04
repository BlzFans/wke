/*
 * Copyright (c) 2009 Brent Fulgham <bfulgham@gmail.org>.  All rights reserved.
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
/*
 *  CFStringEncodingDatabase.h
 *  CoreFoundation
 *
 *  Created by Aki Inoue on 07/12/05.
 *  Copyright (c) 2007-2009, Apple Inc. All rights reserved.
 *
 */

#pragma once

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CoreFoundation_Prefix.h>

__private_extern__ uint16_t __CFStringEncodingGetWindowsCodePage(CFStringEncoding encoding);
__private_extern__ CFStringEncoding __CFStringEncodingGetFromWindowsCodePage(uint16_t codepage);

__private_extern__ bool __CFStringEncodingGetCanonicalName(CFStringEncoding encoding, char *buffer, CFIndex bufferSize);
__private_extern__ CFStringEncoding __CFStringEncodingGetFromCanonicalName(const char *canonicalName);

__private_extern__ CFStringEncoding __CFStringEncodingGetMostCompatibleMacScript(CFStringEncoding encoding);

__private_extern__ const char *__CFStringEncodingGetName(CFStringEncoding encoding); // Returns simple non-localizd name
