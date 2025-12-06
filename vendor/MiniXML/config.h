/*
 * Configuration header for Mini-XML on Windows
 * Generated for Windows build
 */

#ifndef MXML_CONFIG_H
#define MXML_CONFIG_H

/* Include standard headers */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for internal functions */
extern char *_mxml_strdupf(const char *format, ...);
extern char *_mxml_vstrdupf(const char *format, va_list ap);

/* Define to 1 if you have the <inttypes.h> header file. */
#ifdef _WIN32
#  define HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `pthread' library (-lpthread). */
/* #undef HAVE_LIBPTHREAD */

/* Define to 1 if you have the <pthread.h> header file. */
/* #undef HAVE_PTHREAD_H */

/* Define to 1 if you have the `snprintf' function. */
#ifdef _WIN32
#  define HAVE_SNPRINTF 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifdef _WIN32
#  define HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `vsnprintf' function. */
#ifdef _WIN32
#  define HAVE_VSNPRINTF 1
#endif

/* Version number */
#define MXML_VERSION "Mini-XML v2.7"

/* Define to 1 if you need to support multi-threading. */
/* #undef HAVE_PTHREAD */

#endif /* !MXML_CONFIG_H */
