/* CoreGraphics - xlocale.h
 * Copyright (c) 2006-2007 Apple Inc.
 * All rights reserved. */

/* Provide functions used by CoreGraphics from `<xlocale.h>'. */

#ifndef CG_XLOCALE_H_
#define CG_XLOCALE_H_

#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <locale.h>

typedef _locale_t locale_t;

#define isalpha_l _isalpha_l
#define isdigit_l _isdigit_l
#define islower_l _islower_l
#define isprint_l _isprint_l
#define isspace_l _isspace_l
#define isupper_l _isupper_l
#define isxdigit_l _isxdigit_l
#define tolower_l _tolower_l
#define toupper_l _toupper_l

#define strtod_l _strtod_l
#define strtof_l(ptr, end, locale) ((float)_strtod_l((ptr), (end), (locale)))
#define strtol_l _strtol_l
#define strtoimax_l _strtoi64_l

#define vfprintf_l(stream, locale, format, args)			\
    _vfprintf_l((stream), (format), (locale), (args))

#define vsnprintf_l(string, size, locale, format, args)			\
    _vsnprintf_l((string), (size), (format), (locale), (args))

extern int fprintf_l(FILE *stream, locale_t locale, const char *format, ...);

extern int sprintf_l(char *string, locale_t locale, const char *format, ...);

extern int snprintf_l(char *string, size_t size, locale_t locale,
    const char *format, ...);

extern const char *strcasestr_l(const char *s1, const char *s2,
    locale_t locale);

#endif /* CG_XLOCALE_H_ */
