/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_LINGER_ARRAY_BUFFER_H
#define PHP_LINGER_ARRAY_BUFFER_H

extern zend_module_entry linger_array_buffer_module_entry;
#define phpext_linger_array_buffer_ptr &linger_array_buffer_module_entry

#define PHP_LINGER_ARRAY_BUFFER_VERSION \
    "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#define PHP_LINGER_ARRAY_BUFFER_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_LINGER_ARRAY_BUFFER_API __attribute__((visibility("default")))
#else
#define PHP_LINGER_ARRAY_BUFFER_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define LINGER_ARRAY_BUFFER_G(v) \
    TSRMG(linger_array_buffer_globals_id, zend_linger_array_buffer_globals *, v)
#else
#define LINGER_ARRAY_BUFFER_G(v) (linger_array_buffer_globals.v)
#endif

typedef struct _buffer_object {
    zend_object std;
    void *buffer;
    size_t length;
} buffer_object;

typedef enum _buffer_view_type {
    buffer_view_int8,
    buffer_view_uint8,
    buffer_view_int16,
    buffer_view_uint16,
    buffer_view_int32,
    buffer_view_uint32,
    buffer_view_float,
    buffer_view_double,
} buffer_view_type;

typedef struct _buffer_view_object {
    zend_object std;
    zval *buffer_zval;
    union {
        int8_t *as_int8;
        uint8_t *as_uint8;
        int16_t *as_int16;
        uint16_t *as_uint16;
        int32_t *as_int32;
        uint32_t *as_uint32;
        float *as_float;
        double *as_double;
    } buf;
    size_t offset;
    size_t length;
    buffer_view_type type;
} buffer_view_object;

typedef struct _buffer_view_iterator {
    zend_object_iterator intern;
    buffer_view_object *view;
    size_t offset;
    zval *current;
} buffer_view_iterator;

#define linger_efree(p) \
    if (p) efree(p)
#if PHP_MAJOR_VERSION >= 5 && PHP_MAJOR_VERSION < 7
#define GET_BUFFER_OBJECT(zv) zend_object_store_get_object(zv TSRMLS_CC)
#else if PHP_MAJOR_VERSION >= 7
#define GET_BUFFER_OBJECT(zv) \
    (buffer_object *)((char *)Z_OBJ_P(zv) - XtOffsetOf(buffer_object, std))
#define ZOBJ_GET_BUFFER_OBJECT(zobj) \
    (buffer_object *)((char *)(zobj)-XtOffsetOf(buffer_object, std))
#endif

#endif /* PHP_LINGER_ARRAY_BUFFER_H */

