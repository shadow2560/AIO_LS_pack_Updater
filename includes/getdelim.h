/*
 * Copyright 2021 arnavyc.
 * SPDX-License-Identifier: 0BSD
 *
 * Licensed under the Zero Clause BSD License. See LICENSE.md file in the
 * project root, or https://opensource.org/licenses/0BSD for full license
 * information.
 */

/*
 * To use this single-file library, create a file getdelim.c with the following
 * content: (or just copy src/ay/getdelim.c)
 *
 * #define AY_GETDELIM_IMPLEMENTATION
 * #include "<location of header>"
 */

#ifndef AY_GETDELIM_H
#define AY_GETDELIM_H

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define IS_POSIX
#endif

#ifdef IS_POSIX
#define _POSIX_C_SOURCE 200809L
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(IS_POSIX)
#include <sys/types.h>
#elif defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
typedef ptrdiff_t ssize_t;
#endif

ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delimiter,
                 FILE *restrict stream);

ssize_t getline(char **restrict lineptr, size_t *restrict n,
                FILE *restrict stream);

#ifdef AY_GETDELIM_IMPLEMENTATION
#include <errno.h>

#define INITIAL_BUFFSZ 128

ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delimiter,
                 FILE *restrict stream) {
#if defined(IS_POSIX)
  flockfile(stream);
#elif defined(_WIN32)
  _lock_file(stream);
#endif

  if (lineptr == NULL || stream == NULL || n == NULL) {
    errno = EINVAL;
    return -1;
  }

#if defined(IS_POSIX)
  int c = getc_unlocked(stream);
#elif defined(_WIN32)
  int c = _getc_nolock(stream);
#else
  int c = getc(stream);
#endif

  if (c == EOF) {
    return -1;
  }

  if (*lineptr == NULL) {
    *lineptr = malloc(INITIAL_BUFFSZ);
    if (*lineptr == NULL) {
      return -1;
    }
    *n = INITIAL_BUFFSZ;
  }

  size_t pos = 0;
  while (c != EOF) {
    if (pos + 1 >= *n) {
      size_t new_size = *n + (*n >> 2);
      if (new_size < INITIAL_BUFFSZ) {
        new_size = INITIAL_BUFFSZ;
      }
      char *new_ptr = realloc(*lineptr, new_size);
      if (new_ptr == NULL) {
        return -1;
      }
      *n = new_size;
      *lineptr = new_ptr;
    }

    ((unsigned char *)(*lineptr))[pos++] = c;
    if (c == delimiter) {
      break;
    }

#if defined(IS_POSIX)
    c = getc_unlocked(stream);
#elif defined(_WIN32)
    c = _getc_nolock(stream);
#else
    c = getc(stream);
#endif
  }

#if defined(IS_POSIX)
  funlockfile(stream);
#elif defined(_WIN32)
  _unlock_file(stream);
#endif

  (*lineptr)[pos] = '\0';
  return pos;
}

ssize_t getline(char **restrict lineptr, size_t *restrict n,
                FILE *restrict stream) {
  return getdelim(lineptr, n, '\n', stream);
}
#endif /* AY_GETDELIM_IMPLEMENTATION */

#endif /* AY_GETDELIM_H */
