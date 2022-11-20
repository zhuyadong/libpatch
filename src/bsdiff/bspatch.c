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

#include <limits.h>
#include "bspatch.h"

static int64_t offtin(uint8_t *buf)
{
	int64_t y;

	y = buf[7] & 0x7F;
	y = y * 256;
	y += buf[6];
	y = y * 256;
	y += buf[5];
	y = y * 256;
	y += buf[4];
	y = y * 256;
	y += buf[3];
	y = y * 256;
	y += buf[2];
	y = y * 256;
	y += buf[1];
	y = y * 256;
	y += buf[0];

	if (buf[7] & 0x80)
		y = -y;

	return y;
}

#if !defined(BSPATCH_EXECUTABLE)
DLLAPI int bspatch(const uint8_t *old, int64_t oldsize, uint8_t *new, int64_t newsize, struct bspatch_stream *stream)
{
	uint8_t buf[8];
	int64_t oldpos, newpos;
	int64_t ctrl[3];
	int64_t i;

	oldpos = 0;
	newpos = 0;
	while (newpos < newsize)
	{
		/* Read control data */
		for (i = 0; i <= 2; i++)
		{
			if (stream->read(stream, buf, 8))
				return -5;
			ctrl[i] = offtin(buf);
		};

		/* Sanity-check */
		if (ctrl[0] < 0 || ctrl[0] > INT_MAX ||
			ctrl[1] < 0 || ctrl[1] > INT_MAX ||
			newpos + ctrl[0] > newsize)
			return -1;

		/* Read diff string */
		if (stream->read(stream, new + newpos, ctrl[0]))
			return -2;

		/* Add old data to diff string */
		for (i = 0; i < ctrl[0]; i++)
			if ((oldpos + i >= 0) && (oldpos + i < oldsize))
				new[newpos + i] += old[oldpos + i];

		/* Adjust pointers */
		newpos += ctrl[0];
		oldpos += ctrl[0];

		/* Sanity-check */
		if (newpos + ctrl[1] > newsize)
			return -3;

		/* Read extra string */
		if (stream->read(stream, new + newpos, ctrl[1]))
			return -4;

		/* Adjust pointers */
		newpos += ctrl[1];
		oldpos += ctrl[2];
	};

	return 0;
}
#endif

#include <bzlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int bz2_read(const struct bspatch_stream *stream, void *buffer, int length)
{
	int n;
	int bz2err;
	BZFILE *bz2;

	bz2 = (BZFILE *)stream->opaque;
	n = BZ2_bzRead(&bz2err, bz2, buffer, length);
	if (n != length)
		return -1;

	return 0;
}

#if !defined(BSPATCH_EXECUTABLE)
DLLAPI int mbspatch(const uint8_t *olddata, int64_t oldsize, const char *patchpath, const char *newpath)
{
	FILE *f;
	int fd, ret;
	int bz2err;
	uint8_t header[24];
	uint8_t *new = NULL;
	int64_t newsize, rdsize;
	BZFILE *bz2;
	struct bspatch_stream stream;
	struct stat sb;

	/* Open patch file */
	if ((f = fopen(patchpath, "rb")) == NULL)
		return 1;

	/* Read header */
	if (fread(header, 1, 24, f) != 24)
	{
		fclose(f);
		return 2;
	}

	/* Check for appropriate magic */
	if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0)
	{
		fclose(f);
		return 3;
	}

	/* Read lengths from header */
	newsize = offtin(header + 16);
	if (newsize < 0)
	{
		fclose(f);
		return 4;
	}

	if ((new = malloc(newsize + 1)) == NULL)
	{
		fclose(f);
		return 5;
	}

	if (NULL == (bz2 = BZ2_bzReadOpen(&bz2err, f, 0, 0, NULL, 0)))
	{
		fclose(f);
		free(new);
		return 6;
	}

	stream.read = bz2_read;
	stream.opaque = bz2;
	if ((ret = bspatch(olddata, oldsize, new, newsize, &stream)))
	{
		BZ2_bzReadClose(&bz2err, bz2);
		fclose(f);
		free(new);
		return 7;
	}

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose(&bz2err, bz2);
	fclose(f);

	/* Write the new file */
	if (((fd = open(newpath, O_CREAT | O_TRUNC | X_WRONLY, sb.st_mode)) < 0) ||
		(write(fd, new, newsize) != newsize) || ((fd = close(fd)) == -1))
	{
		if (fd > 0)
			close(fd);
		free(new);
		return 8;
	}

	free(new);
	return 0;
}

DLLAPI int fbspatch(const char *oldpath, const char *patchpath, const char *newpath)
{
	FILE *f;
	int fd, ret;
	int bz2err;
	uint8_t header[24];
	uint8_t *old = NULL, *new = NULL;
	int64_t oldsize, newsize, rdsize;
	BZFILE *bz2;
	struct bspatch_stream stream;
	struct stat sb;

	/* Open patch file */
	if ((f = fopen(patchpath, "rb")) == NULL)
		return 1;

	/* Read header */
	if (fread(header, 1, 24, f) != 24)
	{
		fclose(f);
		return 2;
	}

	/* Check for appropriate magic */
	if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0)
	{
		fclose(f);
		return 3;
	}

	/* Read lengths from header */
	newsize = offtin(header + 16);
	if (newsize < 0)
	{
		fclose(f);
		return 4;
	}

	/* Close patch file and re-open it via libbzip2 at the right places */
	if (((fd = open(oldpath, X_RDONLY, 0)) < 0) ||
		((oldsize = lseek(fd, 0, SEEK_END)) == -1) ||
		((old = malloc(oldsize + 1)) == NULL) ||
		(lseek(fd, 0, SEEK_SET) != 0) ||
		((rdsize = read(fd, old, oldsize)) != oldsize) ||
		(fstat(fd, &sb)) ||
		((fd = close(fd)) == -1))
	{
		if (old)
			free(old);
		if (fd > 0)
			fclose(fd);
		fclose(f);
		return 5;
	}
	if ((new = malloc(newsize + 1)) == NULL)
	{
		free(old);
		fclose(f);
		return 6;
	}

	if (NULL == (bz2 = BZ2_bzReadOpen(&bz2err, f, 0, 0, NULL, 0)))
	{
		fclose(f);
		free(old);
		free(new);
		return 7;
	}

	stream.read = bz2_read;
	stream.opaque = bz2;
	if ((ret = bspatch(old, oldsize, new, newsize, &stream)))
	{
		BZ2_bzReadClose(&bz2err, bz2);
		fclose(f);
		free(old);
		free(new);
		return 8;
	}

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose(&bz2err, bz2);
	fclose(f);

	/* Write the new file */
	if (((fd = open(newpath, O_CREAT | O_TRUNC | X_WRONLY, sb.st_mode)) < 0) ||
		(write(fd, new, newsize) != newsize) || ((fd = close(fd)) == -1))
	{
		if (fd > 0)
			close(fd);
		free(old);
		free(new);
		return 9;
	}

	free(new);
	free(old);

	return 0;
}
#endif

#if defined(BSPATCH_EXECUTABLE)
int main(int argc, char *argv[])
{
	int ret = 0;
	if (argc != 4)
		errx(1, "usage: %s oldfile newfile patchfile\n", argv[0]);

	ret = fbspatch(argv[1], argv[2], argv[3]);
	switch (ret)
	{
	case 1:
		errx(1, "fopen(%s)", argv[3]);
		break;
	case 2:
		errx(1, "fread(%s)", argv[3]);
		break;
	case 3:
		errx(1, "Corrupt patch\n");
		break;
	case 4:
		errx(1, "Corrupt patch\n");
		break;
	case 5:
		errx(1, "%s", argv[1]);
		break;
	case 6:
		errx(1, "new malloc");
		break;
	case 7:
		errx(1, "BZ2_bzReadOpen");
		break;
	case 8:
		errx(1, "bspatch");
		break;
	case 9:
		errx(1, "%s", argv[2]);
		break;
	default:
		break;
	}
	return 0;
}
#endif
