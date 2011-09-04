/* CoreGraphics - unistd.h
   Copyright (c) 2006-2009 Apple Inc.
   All rights reserved. */

/* Provide functions used by ImageIO from `<unistd.h>'. */

#ifndef IMAGE_IO_UNISTD_H_
#define IMAGE_IO_UNISTD_H_

#pragma once

#include <sys/types.h>

/* Used in `ColorSync/Dispatcher/Sources/PrefsFile.c'. */
extern char *realpath(const char *pathname, char resolvedPath[]);

#endif /* IMAGE_IO_UNISTD_H_ */
