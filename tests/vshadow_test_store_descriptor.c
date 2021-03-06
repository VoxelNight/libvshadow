/*
 * Library store_descriptor type test program
 *
 * Copyright (C) 2011-2017, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <file_stream.h>
#include <types.h>

#if defined( HAVE_STDLIB_H ) || defined( WINAPI )
#include <stdlib.h>
#endif

#include "vshadow_test_libcerror.h"
#include "vshadow_test_libvshadow.h"
#include "vshadow_test_macros.h"
#include "vshadow_test_memory.h"
#include "vshadow_test_unused.h"

#include "../libvshadow/libvshadow_store_descriptor.h"

#if defined( __GNUC__ )

/* Tests the libvshadow_store_descriptor_initialize function
 * Returns 1 if successful or 0 if not
 */
int vshadow_test_store_descriptor_initialize(
     void )
{
	libcerror_error_t *error                        = NULL;
	libvshadow_store_descriptor_t *store_descriptor = NULL;
	int result                                      = 0;

	/* Test store_descriptor initialization
	 */
	result = libvshadow_store_descriptor_initialize(
	          &store_descriptor,
	          &error );

	VSHADOW_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

        VSHADOW_TEST_ASSERT_IS_NOT_NULL(
         "store_descriptor",
         store_descriptor );

        VSHADOW_TEST_ASSERT_IS_NULL(
         "error",
         error );

	result = libvshadow_store_descriptor_free(
	          &store_descriptor,
	          &error );

	VSHADOW_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

        VSHADOW_TEST_ASSERT_IS_NULL(
         "store_descriptor",
         store_descriptor );

        VSHADOW_TEST_ASSERT_IS_NULL(
         "error",
         error );

	/* Test error cases
	 */
	result = libvshadow_store_descriptor_initialize(
	          NULL,
	          &error );

	VSHADOW_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

        VSHADOW_TEST_ASSERT_IS_NOT_NULL(
         "error",
         error );

	libcerror_error_free(
	 &error );

	store_descriptor = (libvshadow_store_descriptor_t *) 0x12345678UL;

	result = libvshadow_store_descriptor_initialize(
	          &store_descriptor,
	          &error );

	VSHADOW_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

        VSHADOW_TEST_ASSERT_IS_NOT_NULL(
         "error",
         error );

	libcerror_error_free(
	 &error );

	store_descriptor = NULL;

#if defined( HAVE_VSHADOW_TEST_MEMORY )

	/* Test libvshadow_store_descriptor_initialize with malloc failing
	 */
	vshadow_test_malloc_attempts_before_fail = 0;

	result = libvshadow_store_descriptor_initialize(
	          &store_descriptor,
	          &error );

	if( vshadow_test_malloc_attempts_before_fail != -1 )
	{
		vshadow_test_malloc_attempts_before_fail = -1;

		if( store_descriptor != NULL )
		{
			libvshadow_store_descriptor_free(
			 &store_descriptor,
			 NULL );
		}
	}
	else
	{
		VSHADOW_TEST_ASSERT_EQUAL_INT(
		 "result",
		 result,
		 -1 );

		VSHADOW_TEST_ASSERT_IS_NULL(
		 "store_descriptor",
		 store_descriptor );

		VSHADOW_TEST_ASSERT_IS_NOT_NULL(
		 "error",
		 error );

		libcerror_error_free(
		 &error );
	}
	/* Test libvshadow_store_descriptor_initialize with memset failing
	 */
	vshadow_test_memset_attempts_before_fail = 0;

	result = libvshadow_store_descriptor_initialize(
	          &store_descriptor,
	          &error );

	if( vshadow_test_memset_attempts_before_fail != -1 )
	{
		vshadow_test_memset_attempts_before_fail = -1;

		if( store_descriptor != NULL )
		{
			libvshadow_store_descriptor_free(
			 &store_descriptor,
			 NULL );
		}
	}
	else
	{
		VSHADOW_TEST_ASSERT_EQUAL_INT(
		 "result",
		 result,
		 -1 );

		VSHADOW_TEST_ASSERT_IS_NULL(
		 "store_descriptor",
		 store_descriptor );

		VSHADOW_TEST_ASSERT_IS_NOT_NULL(
		 "error",
		 error );

		libcerror_error_free(
		 &error );
	}
#endif /* defined( HAVE_VSHADOW_TEST_MEMORY ) */

	return( 1 );

on_error:
	if( error != NULL )
	{
		libcerror_error_free(
		 &error );
	}
	if( store_descriptor != NULL )
	{
		libvshadow_store_descriptor_free(
		 &store_descriptor,
		 NULL );
	}
	return( 0 );
}

/* Tests the libvshadow_store_descriptor_free function
 * Returns 1 if successful or 0 if not
 */
int vshadow_test_store_descriptor_free(
     void )
{
	libcerror_error_t *error = NULL;
	int result               = 0;

	/* Test error cases
	 */
	result = libvshadow_store_descriptor_free(
	          NULL,
	          &error );

	VSHADOW_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

        VSHADOW_TEST_ASSERT_IS_NOT_NULL(
         "error",
         error );

	libcerror_error_free(
	 &error );

	return( 1 );

on_error:
	if( error != NULL )
	{
		libcerror_error_free(
		 &error );
	}
	return( 0 );
}

#endif /* defined( __GNUC__ ) */

/* The main program
 */
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
int wmain(
     int argc VSHADOW_TEST_ATTRIBUTE_UNUSED,
     wchar_t * const argv[] VSHADOW_TEST_ATTRIBUTE_UNUSED )
#else
int main(
     int argc VSHADOW_TEST_ATTRIBUTE_UNUSED,
     char * const argv[] VSHADOW_TEST_ATTRIBUTE_UNUSED )
#endif
{
	VSHADOW_TEST_UNREFERENCED_PARAMETER( argc )
	VSHADOW_TEST_UNREFERENCED_PARAMETER( argv )

#if defined( __GNUC__ )

	VSHADOW_TEST_RUN(
	 "libvshadow_store_descriptor_initialize",
	 vshadow_test_store_descriptor_initialize );

	VSHADOW_TEST_RUN(
	 "libvshadow_store_descriptor_free",
	 vshadow_test_store_descriptor_free );

	/* TODO add tests for libvshadow_store_descriptor_has_in_volume_data */
	/* TODO add tests for libvshadow_store_descriptor_compare_by_creation_time */
	/* TODO add tests for libvshadow_store_descriptor_compare_by_identifier */
	/* TODO add tests for libvshadow_store_descriptor_read_catalog_entry */
	/* TODO add tests for libvshadow_store_descriptor_read_store_header */
	/* TODO add tests for libvshadow_store_descriptor_read_store_bitmap */
	/* TODO add tests for libvshadow_store_descriptor_read_store_block_list */
	/* TODO add tests for libvshadow_store_descriptor_read_store_block_range_list */
	/* TODO add tests for libvshadow_store_descriptor_read_block_descriptors */
	/* TODO add tests for libvshadow_store_descriptor_read_buffer */
	/* TODO add tests for libvshadow_store_descriptor_get_volume_size */
	/* TODO add tests for libvshadow_store_descriptor_get_identifier */
	/* TODO add tests for libvshadow_store_descriptor_get_creation_time */
	/* TODO add tests for libvshadow_store_descriptor_get_copy_identifier */
	/* TODO add tests for libvshadow_store_descriptor_get_copy_set_identifier */
	/* TODO add tests for libvshadow_store_descriptor_get_attribute_flags */
	/* TODO add tests for libvshadow_store_descriptor_get_number_of_blocks */
	/* TODO add tests for libvshadow_store_descriptor_get_block_descriptor_by_index */

#endif /* defined( __GNUC__ ) */

	return( EXIT_SUCCESS );

on_error:
	return( EXIT_FAILURE );
}

