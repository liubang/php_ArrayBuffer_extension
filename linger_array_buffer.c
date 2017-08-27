/*
  +----------------------------------------------------------------------+
  | linger_array_buffer                                                  |
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
  | Author: liubang <it.liubang@gmail.com>                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
# include "win32/php_stdint.h"
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_linger_array_buffer.h"

extern *zend_ce_arrayaccess;

#define linger_efree(p)		if(p) efree(p)

static int le_linger_array_buffer;

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
		int8_t    *as_int8;
		uint8_t   *as_uint8;
		int16_t   *as_int16;
		uint16_t  *as_uint16;
		int32_t   *as_int32;
		uint32_t  *as_uint32;
		float     *as_float;
		double    *as_double;
	} buf;
	size_t offset;
	size_t length;
	buffer_view_type type;
} buffer_view_object;

/* ArrayBuffer class entry and handlers */
zend_class_entry *linger_array_buffer_ce;
zend_object_handlers linger_array_buffer_handlers;

/* ArrayBufferView class entry and handlers */
zend_class_entry *int8_array_ce;
zend_class_entry *uint8_array_ce;
zend_class_entry *int16_array_ce;
zend_class_entry *uint16_array_ce;
zend_class_entry *int32_array_ce;
zend_class_entry *uint32_array_ce;
zend_class_entry *float_array_ce;
zend_class_entry *double_array_ce;
zend_object_handlers linger_array_buffer_view_handlers;

static void linger_array_buffer_free_object_storage(buffer_object *intern TSRMLS_DC)
{
	zend_object_std_dtor(&intern->std TSRMLS_CC);
	linger_efree(intern->buffer);
}

zend_object_value linger_array_buffer_create_object(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	buffer_object *intern = emalloc(sizeof(buffer_object));
	memset(intern, 0, sizeof(buffer_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	retval.handle = zend_objects_store_put(
			intern,
			(zend_objects_store_dtor_t) zend_objects_destroy_object,
			(zend_objects_free_object_storage_t) linger_array_buffer_free_object_storage,
			NULL
			TSRMLS_CC
			);

	retval.handlers = &linger_array_buffer_handlers;
	return retval;
}

static zend_object_value linger_array_buffer_clone(zval *object TSRMLS_DC)
{
	buffer_object *old_object = zend_object_store_get_object(object TSRMLS_CC);
	zend_object_value new_object_val = linger_array_buffer_create_object(Z_OBJCE_P(object) TSRMLS_CC);
	buffer_object *new_object = zend_object_store_get_object_by_handle(new_object_val.handle TSRMLS_CC);

	zend_objects_clone_members(&new_object->std, new_object_val, &old_object->std, Z_OBJ_HANDLE_P(object) TSRMLS_CC);

	new_object->buffer = old_object->buffer;
	new_object->length = old_object->length;

	if (old_object->buffer) {
		new_object->buffer = emalloc(old_object->length);
		memcpy(new_object->buffer, old_object->buffer, old_object->length);
	}
	memcpy(new_object->buffer, old_object->buffer, old_object->length);
	return new_object_val;
}

static void linger_array_buffer_view_free_object_storage(buffer_view_object *intern TSRMLS_DC)
{
	zend_object_std_dtor(&intern->std TSRMLS_CC);
	if (intern->buffer_zval) {
		zval_ptr_dtor(&intern->buffer_zval);
	}
	linger_efree(intern);
}

zend_object_value linger_array_buffer_view_create_object(zend_class_entry *class_type TSRMLS_CC)
{
	zend_object_value retval;
	buffer_view_object *intern = emalloc(sizeof(buffer_view_object));
	memset(intern, 0, sizeof(buffer_view_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	{
		zend_class_entry *base_class_type = class_type;
		while (base_class_type->parent) {
			base_class_type = base_class_type->parent;
		}

		if (base_class_type == int8_array_ce) {
			intern->type = buffer_view_int8;
		} else if (base_class_type == uint8_array_ce) {
			intern->type = buffer_view_uint8;
		} else if (base_class_type == int16_array_ce) {
			intern->type = buffer_view_int16;
		} else if (base_class_type == uint16_array_ce) {
			intern->type = buffer_view_uint16;
		} else if (base_class_type == int32_array_ce) {
			intern->type = buffer_view_int32;
		} else if (base_class_type == uint32_array_ce) {
			intern->type = buffer_view_uint32;
		} else if (base_class_type == float_array_ce) {
			intern->type = buffer_view_float;
		} else if (base_class_type == double_array_ce) {
			intern->type = buffer_view_double;
		} else {
			zend_error(E_ERROR, "Buffer view does not have a valid base class");
		}
	}

	retval.handle = zend_objects_store_put(
			intern,
			(zend_objects_store_dtor_t) zend_objects_destroy_object,
			(zend_objects_free_object_storage_t) linger_array_buffer_view_free_object_storage,
			NULL
			TSRMLS_CC
			);

	retval.handlers = &linger_array_buffer_view_handlers;
	return retval;
}

static zend_object_value linger_array_buffer_view_clone(zval *object TSRMLS_DC)
{
	buffer_view_object *old_object = zend_object_store_get_object(object TSRMLS_CC);
	zend_object_value new_object_val = linger_array_buffer_view_create_object(Z_OBJCE_P(object) TSRMLS_CC);
	buffer_view_object *new_object = zend_object_store_get_object_by_handle(new_object_val.handle TSRMLS_CC);

	zend_objects_clone_members(&new_object->std, new_object_val, &old_object->std, Z_OBJ_HANDLE_P(object) TSRMLS_CC);

	new_object->buffer_zval = old_object->buffer_zval;
	if (new_object->buffer_zval) {
		Z_ADDREF_P(new_object->buffer_zval);
	}

	new_object->buf.as_int8 = old_object->buf.as_int8;
	new_object->offset = old_object->offset;
	new_object->length = old_object->length;
	new_object->type = old_object->type;

	return new_object_val;
}


zval *linger_buffer_view_offset_get(buffer_view_object *intern, size_t offset)
{
	zval *retval;
	MAKE_STD_ZVAL(retval);

	switch (intern->type) {
		case buffer_view_int8:
			ZVAL_LONG(retval, intern->buf.as_int8[offset]);
			break;
		case buffer_view_uint8:
			ZVAL_LONG(retval, intern->buf.as_uint8[offset]);
			break;
		case buffer_view_int16:
			ZVAL_LONG(retval, intern->buf.as_int16[offset]);
			break;
		case buffer_view_uint16:
			ZVAL_LONG(retval, intern->buf.as_uint16[offset]);
			break;
		case buffer_view_int32:
			ZVAL_LONG(retval, intern->buf.as_int32[offset]);
			break;
		case buffer_view_uint32: {
			uint32_t value = intern->buf.as_uint32[offset];
			if (value <= LONG_MAX) {
				ZVAL_LONG(retval, value);
			} else {
				ZVAL_DOUBLE(retval, value);
			}
			break;
		}
		case buffer_view_float:
			ZVAL_DOUBLE(retval, intern->buf.as_float[offset]);
			break;
		case buffer_view_double:
			ZVAL_DOUBLE(retval, intern->buf.as_double[offset]);
			break;
		default:
			zend_error_noreturn(E_ERROR, "Invalid buffer view type");
	}
	return retval;
}

void linger_buffer_view_offset_set(buffer_view_object *intern, long offset, zval *value)
{
	if (intern->type == buffer_view_float || intern->type == buffer_view_double) {
		Z_ADDREF_P(value);
		convert_to_double_ex(&value);
		if (intern->type == buffer_view_float) {
			intern->buf.as_float[offset] = Z_DAVL_P(value);
		} else {
			intern->buf.as_double[offset] = Z_DAVL_P(value);
		}
		zval_ptr_dtor(&value);
	} else {
		Z_ADDREF_P(value);
		convert_to_long_ex(&value);
		switch (intern->type) {
			case buffer_view_int8:
				intern->buf.as_int8[offset] = Z_LVAL_P(value);
				break;
			case buffer_view_uint8:
				intern->buf.as_uint8[offset] = Z_LVAL_P(value);
				break;
			case buffer_view_int16:
				intern->buf.as_int16[offset] = Z_LVAL_P(value);
				break;
			case buffer_view_uint16:
				intern->buf.as_uint16[offset] = Z_LVAL_P(value);
				break;
			case buffer_view_int32:
				intern->buf.as_int32[offset] = Z_LVAL_P(value);
				break;
			case buffer_view_uint32:
				intern->buf.as_uint32[offset] = Z_LVAL_P(value);
				break;
			default:
				zend_error(E_ERROR, "Invalid buffer view type");
		}	
		zval_ptr_dtor(&value);
	}
}


/* ArrayBuffer arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

/* ArrayBufferView arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_view_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_view_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_view_offset_set, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_METHOD(linger_ArrayBuffer, __construct)
{
	buffer_object *intern;
	long length;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &length) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	zend_restore_error_handling(&error_handling TSRMLS_CC);
	if (length < 0) {
		zend_throw_exception(NULL, "Buffer length must be positive", 0 TSRMLS_CC);
		return;
	}
	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	intern->buffer = emalloc(length);
	intern->length = length;

	memset(intern->buffer, 0, length);
}

PHP_METHOD(linger_ArrayBuffer, length)
{
	buffer_object *intern;	
	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG(intern->length);
}

const zend_function_entry linger_array_buffer_methods[] = {
	PHP_ME(linger_ArrayBuffer, __construct, arginfo_buffer_ctor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(linger_ArrayBuffer, length, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END	
};

size_t linger_buffer_view_get_bytes_per_element(buffer_view_object *intern)
{
	switch (intern->type) {
		case buffer_view_int8:
		case buffer_view_uint8:
			return 1;
		case buffer_view_int16:
		case buffer_view_uint16:
			return 2;
		case buffer_view_int32:
		case buffer_view_uint32:
		case buffer_view_float:
			return 4;
		case buffer_view_double:
			return 8;
		default:
			zend_error_noreturn(E_ERROR, "Invalid buffer view type");
			
	}	
}

static long get_long_from_zval(zval *offset)
{
	if (Z_TYPE_P(offset) == IS_LONG) {
		return Z_LVAL_P(offset);
	} else {
		zval tmp = *offset;
		zval_copy_ctor(&tmp);
		convert_to_long(&tmp);
		return Z_LVAL(tmp);
	}
}

static zval *linger_array_buffer_view_read_dimension(zval *object, zval *zv_offset, int type TSRMLS_DC)
{
	buffer_view_object *intern = zend_object_store_get_object(object TSRMLS_CC);
	zval *retval;
	long offset;

	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->read_dimension(object, zv_offset, type TSRMLS_CC);
	}
	
	if (!zv_offset) {
		zend_throw_exception(NULL, "Cannot append to a typed array", 0 TSRMLS_CC);
		return NULL;
	}

	offset = get_long_from_zval(zv_offset);
	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0 TSRMLS_CC);
		return NULL;
	}

	retval = linger_buffer_view_offset_get(intern, offset);
	Z_DELREF_P(retval);
	return retval;
}

static void linger_array_buffer_view_write_dimension(zval *object, zval *zv_offset, zval *value TSRMLS_DC)
{
	buffer_view_object *intern;
	long offset;
	intern = zend_object_store_get_object(object TSRMLS_CC);
	
	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->write_dimension(object, zv_offset, value TSRMLS_CC);
	}

	if (!zv_offset) {
		zend_throw_exception(NULL, "Cannot append to a typed array", 0 TSRMLS_CC);
		return;
	}

	offset = get_long_from_zval(zv_offset);
	if (offset < 0 || offset > intern->length) {
		zend_throw_excpetion(NULL, "Offset is outside the buffer range", 0 TSRMLS_CC);
		return;
	}

	linger_buffer_view_offset_set(intern, offset, value);
}

static int linger_array_buffer_view_has_dimension(zval *object, zval *zv_offset,  int check_empty TSRMLS_DC)
{
	buffer_view_object *intern;
	long offset;
	intern = zend_object_store_get_object(object TSRMLS_CC);
	
	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->has_dimension(object, zv_offset, check_empty TSRMLS_CC);
	}

	offset = get_long_from_zval(zv_offset);
	if (offset < 0 || offset > intern->length) {
		return 0;
	}

	if (check_empty) {
		int retval;
		zval *value = linger_buffer_view_offset_get(intern, offset);
		retval = zend_is_true(value);
		zval_ptr_dtor(&value);
		return retval;
	}

	return 1;
}

static void linger_array_buffer_view_unset_dimension(zval *object, zval *zv_offset TSRMLS_DC)
{
	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->write_dimension(object, zv_offset, value TSRMLS_CC);
	}
	zend_throw_exception(NULL, "Cannot unset offsets in a typed array", 0 TSRMLS_CC);
}

static int linger_array_buffer_view_compare_objects(zval *obj1, zval *obj2 TSRMLS_DC)
{
	buffer_view_object *intern1 = zend_object_store_get_object(obj1 TSRMLS_CC);
	buffer_view_object *intern2 = zend_object_store_get_object(obj2 TSRMLS_CC);

	if (memcmp(intern1, intern2, sizeof(buffer_view_object)) == 0) {
		return 0;
	} else {
		return 1;
	}
}

static HashTable *linger_array_buffer_view_get_debug_info(zval *obj, int *is_temp TSRMLS_DC) 
{
	buffer_view_object *intern = zend_object_store_get_object(obj TSRMLS_CC);
	HashTable *properties = Z_OBJPROP_P(obj);
	HashTable *ht;
	int i;

	ALLOC_HASHTABLE(ht);
	ZEND_INIT_SYMTABLE_EX(ht, intern->length + zend_hash_num_elements(properties), 0);
	zend_hash_copy(ht, properties, (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));

	*is_temp = 1;
	for (i = 0; i < intern->length; i++) {
		zval *value = linger_buffer_view_offset_get(intern, i);
		zend_hash_index_update(ht, i, (void *) &value, sizeof(zval *), NULL);
	}

	return ht;
}

PHP_FUNCTION(linger_array_buffer_view_ctor)
{
	zval *buffer_zval;
	long offset = 0, length = 0;
	buffer_object *buffer_intern;
	buffer_view_object *view_intern;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|ll", &buffer_zval, linger_array_buffer_ce, &offset, &length) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	zend_restore_error_handling(&error_handling TSRMLS_CC);

	view_intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	buffer_intern = zend_object_store_get_object(buffer_zval TSRMLS_CC);

	if (offset < 0) {
		zend_throw_exception(NULL, "Offset must be non-negative", 0 TSRMLS_CC);
		return;
	}
	if (offset >= buffer_intern->length) {
		zend_throw_exception(NULL, "Offset has to be smaller than the buffer length", 0 TSRMLS_CC);
		return;
	}
	if (length < 0) {
		zend_throw_exception(NULL, "Length must be positive or zero", 0 TSRMLS_CC);
		return;
	}
	view_intern->offset = offset;
	view_intern->buffer_zval = buffer_zval;
	Z_ADDREF_P(buffer_zval);

	{
		size_t bytes_per_element = linger_buffer_view_get_bytes_per_element(view_intern);
		size_t max_length = (buffer_intern->length - offset) / bytes_per_element;
		if (length == 0) {
			view_intern->length = max_length;
		} else if (length > max_length) {
			zend_throw_exception(NULL, "Length is large than the buffer", 0 TSRMLS_CC);
			return;
		} else {
			view_intern->length = length;
		}
	}
	view_intern->buf.as_int8 = buffer_intern->buffer;
	view_intern->buf.as_int8 += offset;
}

const zend_function_entry linger_array_buffer_view_methods[] = {
	PHP_ME_MAPPING(__construct, linger_array_buffer_view_ctor, arginfo_buffer_view_ctor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_FE_END
};


PHP_MINIT_FUNCTION(linger_array_buffer)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Linger\\ArrayBuffer", linger_array_buffer_methods);
	linger_array_buffer_ce = zend_register_internal_class(&ce TSRMLS_CC);
	linger_array_buffer_ce->create_object = linger_array_buffer_create_object;
	memcpy(&linger_array_buffer_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	linger_array_buffer_handlers.clone_obj = linger_array_buffer_clone;

#define REGISTER_ARRAY_BUFFER_VIEW_CLASS(class_name, type)		                 \
	do {														                 \
		INIT_CLASS_ENTRY(ce, #class_name, linger_array_buffer_view_methods);	 \
		type##_array_ce = zend_register_internal_class(&ce TSRMLS_CC);           \
		type##_array_ce->create_object = linger_array_buffer_view_create_object; \
	} while (0)

	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\Int8Array, int8);
	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\UInt8Array, uint8);
	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\Int16Array, int16);
	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\UInt16Array, uint16);
	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\Int32Array, int32);
	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\UInt32Array, uint32);
	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\FloatArray, float);
	REGISTER_ARRAY_BUFFER_VIEW_CLASS(Linger\\ArrayBufferView\\DoubleArray, double);

	memcpy(&linger_array_buffer_view_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	linger_array_buffer_view_handlers.clone_obj = linger_array_buffer_view_clone;
	linger_array_buffer_view_handlers.read_dimension = linger_array_buffer_view_read_dimension;
	linger_array_buffer_view_handlers.write_dimension = linger_array_buffer_view_write_dimension;
	linger_array_buffer_view_handlers.has_dimension = linger_array_buffer_view_has_dimension;
	linger_array_buffer_view_handlers.unset_dimension = linger_array_buffer_view_unset_dimension;
	linger_array_buffer_view_handlers.compare_objects = linger_array_buffer_view_compare_objects;
	linger_array_buffer_view_handlers.get_debug_info = linger_array_buffer_view_get_debug_info;
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(linger_array_buffer)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(linger_array_buffer)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(linger_array_buffer)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(linger_array_buffer)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "linger_array_buffer support", "enabled");
	php_info_print_table_row(2, "version", PHP_LINGER_ARRAY_BUFFER_VERSION);
	php_info_print_table_row(2, "author", "liubang <it.liubang@gmail.com>");
	php_info_print_table_end();

}

zend_module_entry linger_array_buffer_module_entry = {
	STANDARD_MODULE_HEADER,
	"linger_array_buffer",
	NULL,
	PHP_MINIT(linger_array_buffer),
	PHP_MSHUTDOWN(linger_array_buffer),
	PHP_RINIT(linger_array_buffer),		
	PHP_RSHUTDOWN(linger_array_buffer),	
	PHP_MINFO(linger_array_buffer),
	PHP_LINGER_ARRAY_BUFFER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LINGER_ARRAY_BUFFER
ZEND_GET_MODULE(linger_array_buffer)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
