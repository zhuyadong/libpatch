/*-
 * Copyright 2003-2005 Colin Percival
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BSDIFF_H
#define BSDIFF_H

#include <stddef.h>
#include <stdint.h>

#ifdef WIN32
#define X_RDONLY _O_RDONLY | _O_BINARY
#define X_WRONLY _O_WRONLY | _O_BINARY
#define DLLAPI _declspec(dllexport)
#else
#define X_RDONLY O_RDONLY
#define X_WRONLY O_WRONLY
#define DLLAPI
#endif

struct bsdiff_stream
{
	void *opaque;

	void *(*malloc)(size_t size);
	void (*free)(void *ptr);
	int (*write)(struct bsdiff_stream *stream, const void *buffer, int size);
};

DLLAPI int bsdiff(const uint8_t *old, int64_t oldsize, const uint8_t *new, int64_t newsize, struct bsdiff_stream *stream);
DLLAPI int fbsdiff(const char *oldpath, const char *newpath, const char *patchpath);
DLLAPI int mbsdiff(const uint8_t *olddata, uint64_t oldsize, const uint8_t *newdata, uint64_t newsize, const char *patchpath);
DLLAPI int mbscmp(const uint8_t *olddata, const uint8_t *newdata, uint64_t size);

#endif
