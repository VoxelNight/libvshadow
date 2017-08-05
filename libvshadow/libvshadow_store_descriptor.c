/*
 * Store descriptor functions
 *
 * Copyright (C) 2011-2016, Joachim Metz <joachim.metz@gmail.com>
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
#include <memory.h>
#include <types.h>

#include "libvshadow_block_descriptor.h"
#include "libvshadow_block_range_descriptor.h"
#include "libvshadow_block_tree.h"
#include "libvshadow_debug.h"
#include "libvshadow_definitions.h"
#include "libvshadow_io_handle.h"
#include "libvshadow_libbfio.h"
#include "libvshadow_libcdata.h"
#include "libvshadow_libcerror.h"
#include "libvshadow_libcnotify.h"
#include "libvshadow_libcthreads.h"
#include "libvshadow_libfdatetime.h"
#include "libvshadow_libfguid.h"
#include "libvshadow_libuna.h"
#include "libvshadow_store_block.h"
#include "libvshadow_store_descriptor.h"

#include "vshadow_store.h"

/* Creates a store descriptor
 * ake sure the value store_descriptor is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_initialize(
     libvshadow_store_descriptor_t **store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_initialize";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( *store_descriptor != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid store descriptor value already set.",
		 function );

		return( -1 );
	}
	*store_descriptor = memory_allocate_structure(
	                     libvshadow_store_descriptor_t );

	if( *store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create store descriptor.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *store_descriptor,
	     0,
	     sizeof( libvshadow_store_descriptor_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear store descriptor.",
		 function );

		memory_free(
		 *store_descriptor );

		*store_descriptor = NULL;

		return( -1 );
	}
	if( libcdata_list_initialize(
	     &( ( *store_descriptor )->block_list ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create block list.",
		 function );

		goto on_error;
	}
	if( libcdata_list_initialize(
	     &( ( *store_descriptor )->block_descriptors_list ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create block descriptors list.",
		 function );

		goto on_error;
	}
	if( libcdata_btree_initialize(
	     &( ( *store_descriptor )->forward_block_descriptors_tree ),
	     LIBVSHADOW_BLOCK_DESCRIPTORS_TREE_MAXIMUM_NUMBER_OF_SUB_NODES,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create forward block descriptors tree.",
		 function );

		goto on_error;
	}
	if( libcdata_btree_initialize(
	     &( ( *store_descriptor )->reverse_block_descriptors_tree ),
	     LIBVSHADOW_BLOCK_DESCRIPTORS_TREE_MAXIMUM_NUMBER_OF_SUB_NODES,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create reverse block descriptors tree.",
		 function );

		goto on_error;
	}
	if( libcdata_range_list_initialize(
	     &( ( *store_descriptor )->block_offset_list ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create block offsets list.",
		 function );

		goto on_error;
	}
	if( libcdata_range_list_initialize(
	     &( ( *store_descriptor )->previous_block_offset_list ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create previous block offsets list.",
		 function );

		goto on_error;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_initialize(
	     &( ( *store_descriptor )->read_write_lock ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to intialize read/write lock.",
		 function );

		goto on_error;
	}
#endif
	return( 1 );

on_error:
	if( *store_descriptor != NULL )
	{
		if( ( *store_descriptor )->block_offset_list != NULL )
		{
			libcdata_range_list_free(
			 &( ( *store_descriptor )->block_offset_list ),
			 NULL,
			 NULL );
		}
		if( ( *store_descriptor )->reverse_block_descriptors_tree != NULL )
		{
			libcdata_btree_free(
			 &( ( *store_descriptor )->reverse_block_descriptors_tree ),
			 NULL,
			 NULL );
		}
		if( ( *store_descriptor )->forward_block_descriptors_tree != NULL )
		{
			libcdata_btree_free(
			 &( ( *store_descriptor )->forward_block_descriptors_tree ),
			 NULL,
			 NULL );
		}
		if( ( *store_descriptor )->block_descriptors_list != NULL )
		{
			libcdata_list_free(
			 &( ( *store_descriptor )->block_descriptors_list ),
			 NULL,
			 NULL );
		}
		memory_free(
		 *store_descriptor );

		*store_descriptor = NULL;
	}
	return( -1 );
}

/* Frees a store descriptor
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_free(
     libvshadow_store_descriptor_t **store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_free";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( *store_descriptor != NULL )
	{
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
		if( libcthreads_read_write_lock_free(
		     &( ( *store_descriptor )->read_write_lock ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free read/write lock.",
			 function );

			result = -1;
		}
#endif
		if( ( *store_descriptor )->operating_machine_string != NULL )
		{
			memory_free(
			 ( *store_descriptor )->operating_machine_string );
		}
		if( ( *store_descriptor )->service_machine_string != NULL )
		{
			memory_free(
			 ( *store_descriptor )->service_machine_string );
		}
		if( libcdata_btree_free(
		     &( ( *store_descriptor )->reverse_block_descriptors_tree ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libvshadow_block_descriptor_free_reverse,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free reverse block descriptors tree.",
			 function );

			result = -1;
		}
		if( libcdata_btree_free(
		     &( ( *store_descriptor )->forward_block_descriptors_tree ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libvshadow_block_descriptor_free,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free forward block descriptors tree.",
			 function );

			result = -1;
		}
		if( libcdata_list_free(
		     &( ( *store_descriptor )->block_list ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libvshadow_store_block_free,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free forward block descriptors tree.",
			 function );

			result = -1;
		}
		if( libcdata_list_free(
		     &( ( *store_descriptor )->block_descriptors_list ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libvshadow_block_descriptor_free,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free forward block descriptors tree.",
			 function );

			result = -1;
		}
		if( libcdata_range_list_free(
		     &( ( *store_descriptor )->block_offset_list ),
		     NULL,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free block offsets list.",
			 function );

			result = -1;
		}
		if( libcdata_range_list_free(
		     &( ( *store_descriptor )->previous_block_offset_list ),
		     NULL,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free previous block offsets list.",
			 function );

			result = -1;
		}
		memory_free(
		 *store_descriptor );

		*store_descriptor = NULL;
	}
	return( result );
}

/* Determines if the store has in-volume data
 * Returns 1 if the store has in-volume data, 0 if not or -1 on error
 */
int libvshadow_store_descriptor_has_in_volume_data(
     libvshadow_store_descriptor_t *store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_has_in_volume_data";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	return( (int) store_descriptor->has_in_volume_store_data );
}

/* Compares 2 store descriptors by their creation time
 * Returns LIBCDATA_COMPARE_LESS, LIBCDATA_COMPARE_EQUAL, LIBCDATA_COMPARE_GREATER if successful or -1 on error
 */
int libvshadow_store_descriptor_compare_by_creation_time(
     libvshadow_store_descriptor_t *first_store_descriptor,
     libvshadow_store_descriptor_t *second_store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_compare_by_creation_time";

	if( first_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid first store descriptor.",
		 function );

		return( -1 );
	}
	if( second_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid second store descriptor.",
		 function );

		return( -1 );
	}
	if( first_store_descriptor->creation_time < second_store_descriptor->creation_time )
	{
		return( LIBCDATA_COMPARE_LESS );
	}
	else if( first_store_descriptor->creation_time > second_store_descriptor->creation_time )
	{
		return( LIBCDATA_COMPARE_GREATER );
	}
	return( LIBCDATA_COMPARE_EQUAL );
}

/* Compares 2 store descriptors by their identifier
 * Returns LIBCDATA_COMPARE_LESS, LIBCDATA_COMPARE_EQUAL, LIBCDATA_COMPARE_GREATER if successful or -1 on error
 */
int libvshadow_store_descriptor_compare_by_identifier(
     libvshadow_store_descriptor_t *first_store_descriptor,
     libvshadow_store_descriptor_t *second_store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_compare_by_identifier";
	int result            = 0;

	if( first_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid first store descriptor.",
		 function );

		return( -1 );
	}
	if( second_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid second store descriptor.",
		 function );

		return( -1 );
	}
	result = memory_compare(
	          first_store_descriptor->identifier,
	          second_store_descriptor->identifier,
	          16 );

	if( result < 0 )
	{
		return( LIBCDATA_COMPARE_LESS );
	}
	else if( result > 0 )
	{
		return( LIBCDATA_COMPARE_GREATER );
	}
	return( LIBCDATA_COMPARE_EQUAL );
}

/* Reads the catalog entry
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_catalog_entry(
     libvshadow_store_descriptor_t *store_descriptor,
     const uint8_t *catalog_block_data,
     size_t catalog_block_data_size,
     uint64_t *entry_type,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_read_catalog_entry";

#if defined( HAVE_DEBUG_OUTPUT )
	uint64_t value_64bit  = 0;
#endif

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( catalog_block_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid catalog block data.",
		 function );

		return( -1 );
	}
	if( catalog_block_data_size < 128 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid catalog block data size value too small.",
		 function );

		return( -1 );
	}
	if( catalog_block_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid catalog block data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( entry_type == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid entry type.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: catalog block entry data:\n",
		 function );
		libcnotify_print_data(
		 catalog_block_data,
		 128,
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif
	byte_stream_copy_to_uint64_little_endian(
	 catalog_block_data,
	 *entry_type );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: entry type\t\t\t: %" PRIu64 "\n",
		 function,
		 *entry_type );
	}
#endif
	if( ( *entry_type != 0 )
	 && ( *entry_type != 1 )
	 && ( *entry_type != 2 )
	 && ( *entry_type != 3 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported catalog entry type: %" PRIu64 ".",
		 function,
		 *entry_type );

		goto on_error;
	}
	if( ( *entry_type == 0 )
	 || ( *entry_type == 1 ) )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "\n" );
		}
#endif
/* TODO empty data check ? */
	}
	else if( *entry_type == 2 )
	{
		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 8 ] ),
		 store_descriptor->volume_size );

		if( memory_copy(
		     store_descriptor->identifier,
		     &( catalog_block_data[ 16 ] ),
		     16 ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy identifier.",
			 function );

			goto on_error;
		}
		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 48 ] ),
		 store_descriptor->creation_time );

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: volume size\t\t\t: %" PRIu64 "\n",
			 function,
			 store_descriptor->volume_size );

			if( libvshadow_debug_print_guid_value(
			     function,
			     "store identifier\t\t",
			     &( catalog_block_data[ 16 ] ),
			     16,
			     LIBFGUID_ENDIAN_LITTLE,
			     LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print GUID value.",
				 function );

				goto on_error;
			}
			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 32 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: unknown2\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 40 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: unknown3\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			if( libvshadow_debug_print_filetime_value(
			     function,
			     "creation time\t\t\t",
			     &( catalog_block_data[ 48 ] ),
			     8,
			     LIBFDATETIME_ENDIAN_LITTLE,
			     LIBFDATETIME_STRING_FORMAT_TYPE_CTIME | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print filetime value.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
			 "%s: unknown4:\n",
			 function );
			libcnotify_print_data(
			 &( catalog_block_data[ 56 ] ),
			 72,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
		}
#endif
	}
	else if( *entry_type == 3 )
	{
		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 8 ] ),
		 store_descriptor->store_block_list_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 32 ] ),
		 store_descriptor->store_header_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 40 ] ),
		 store_descriptor->store_block_range_list_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 48 ] ),
		 store_descriptor->store_bitmap_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 56 ] ),
		 store_descriptor->store_inode );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 72 ] ),
		 store_descriptor->store_previous_bitmap_offset );

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store block list offset\t\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_block_list_offset );

			if( libvshadow_debug_print_guid_value(
			     function,
			     "store identifier\t\t",
			     &( catalog_block_data[ 16 ] ),
			     16,
			     LIBFGUID_ENDIAN_LITTLE,
			     LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print GUID value.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
			 "%s: store header offset\t\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_header_offset );

			libcnotify_printf(
			 "%s: store block range list offset\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_block_range_list_offset );

			libcnotify_printf(
			 "%s: store bitmap offset\t\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_bitmap_offset );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 56 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: store file reference\t\t: MFT entry: %" PRIu64 ", sequence: %" PRIu64 "\n",
			 function,
			 value_64bit & 0xffffffffffffUL,
			 value_64bit >> 48 );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 64 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: allocated size\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			libcnotify_printf(
			 "%s: store previous bitmap offset\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_previous_bitmap_offset );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 80 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: unknown2\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			libcnotify_printf(
			 "%s: unknown3:\n",
			 function );
			libcnotify_print_data(
			 &( catalog_block_data[ 88 ] ),
			 40,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
		}
#endif
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );

on_error:
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_write(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Reads the store header
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_header(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block = NULL;
	uint8_t *store_header_data            = NULL;
	static char *function                 = "libvshadow_store_descriptor_read_store_header";
	size_t store_header_data_offset       = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint32_t value_32bit                  = 0;
#endif

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     store_descriptor->store_header_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 store_descriptor->store_header_offset );

		goto on_error;
	}
	
	/* Add the block to the block list
	 */
    if (libcdata_list_append_value(
                         store_descriptor->block_list,
                         (intptr_t *) store_block,
                         error ) == -1)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to add store block at offset: 0x%08" PRIx64 " to store block list.",
		 function,
		 store_descriptor->store_header_offset );

		goto on_error;
	}
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf("%s: adding block with offset = 0x08%" PRIx64 " to list\n", function, store_block->offset);
	}
#endif
	
	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_HEADER )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		return( -1 );
	}
	store_header_data = &( store_block->data[ sizeof( vshadow_store_header_t ) ] );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: store header information:\n",
		 function );
		libcnotify_print_data(
		 store_header_data,
		 sizeof( vshadow_store_information_t ),
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif
	if( memory_copy(
	     store_descriptor->copy_identifier,
	     ( (vshadow_store_information_t *) store_header_data )->copy_identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy shadow copy identifier.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     store_descriptor->copy_set_identifier,
	     ( (vshadow_store_information_t *) store_header_data )->copy_set_identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy shadow copy set identifier.",
		 function );

		goto on_error;
	}
	byte_stream_copy_to_uint32_little_endian(
	 ( (vshadow_store_information_t *) store_header_data )->attribute_flags,
	 store_descriptor->attribute_flags );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( libvshadow_debug_print_guid_value(
		     function,
		     "unknown5\t\t\t",
		     ( (vshadow_store_information_t *) store_header_data )->unknown5,
		     16,
		     LIBFGUID_ENDIAN_LITTLE,
		     LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print GUID value.",
			 function );

			goto on_error;
		}
		if( libvshadow_debug_print_guid_value(
		     function,
		     "copy identifier\t\t",
		     ( (vshadow_store_information_t *) store_header_data )->copy_identifier,
		     16,
		     LIBFGUID_ENDIAN_LITTLE,
		     LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print GUID value.",
			 function );

			goto on_error;
		}
		if( libvshadow_debug_print_guid_value(
		     function,
		     "copy set identifier\t",
		     ( (vshadow_store_information_t *) store_header_data )->copy_set_identifier,
		     16,
		     LIBFGUID_ENDIAN_LITTLE,
		     LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print GUID value.",
			 function );

			goto on_error;
		}
		byte_stream_copy_to_uint32_little_endian(
		 ( (vshadow_store_information_t *) store_header_data )->type,
		 value_32bit );
		libcnotify_printf(
		 "%s: type\t\t\t: 0x%08" PRIx32 "\n",
		 function,
		 value_32bit );

		byte_stream_copy_to_uint32_little_endian(
		 ( (vshadow_store_information_t *) store_header_data )->provider,
		 value_32bit );
		libcnotify_printf(
		 "%s: provider\t\t\t: %" PRIu32 "\n",
		 function,
		 value_32bit );

		libcnotify_printf(
		 "%s: attribute flags\t\t: 0x%08" PRIx32 "\n",
		 function,
		 store_descriptor->attribute_flags );
		libvshadow_debug_print_attribute_flags(
		 store_descriptor->attribute_flags );
		libcnotify_printf(
		 "\n" );

		byte_stream_copy_to_uint32_little_endian(
		 ( (vshadow_store_information_t *) store_header_data )->unknown10,
		 value_32bit );
		libcnotify_printf(
		 "%s: unknown10\t\t: 0x%08" PRIx32 "\n",
		 function,
		 value_32bit );
	}
#endif
	store_header_data_offset = sizeof( vshadow_store_information_t );

	byte_stream_copy_to_uint16_little_endian(
	 &( store_header_data[ store_header_data_offset ] ),
	 store_descriptor->operating_machine_string_size );

	store_header_data_offset += 2;

	if( ( store_header_data_offset + store_descriptor->operating_machine_string_size ) > store_block->data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: operating machine string size value out of bounds.",
		 function );

		goto on_error;
	}
	store_descriptor->operating_machine_string = (uint8_t *) memory_allocate(
	                                                          sizeof( uint8_t ) * store_descriptor->operating_machine_string_size );

	if( store_descriptor->operating_machine_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create operating machine string.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     store_descriptor->operating_machine_string,
	     &( store_header_data[ store_header_data_offset ] ),
	     (size_t) store_descriptor->operating_machine_string_size ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy operating machine string.",
		 function );

		goto on_error;
	}
	store_header_data_offset += store_descriptor->operating_machine_string_size;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( libvshadow_debug_print_utf16_string_value(
		     function,
		     "operating machine string\t",
		     store_descriptor->operating_machine_string,
		     (size_t) store_descriptor->operating_machine_string_size,
		     LIBUNA_ENDIAN_LITTLE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print UTF-16 string value.",
			 function );

			goto on_error;
		}
	}
#endif
	byte_stream_copy_to_uint16_little_endian(
	 &( store_header_data[ store_header_data_offset ] ),
	 store_descriptor->service_machine_string_size );

	store_header_data_offset += 2;

	if( ( store_header_data_offset + store_descriptor->service_machine_string_size ) > store_block->data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: operating machine string size value out of bounds.",
		 function );

		goto on_error;
	}
	store_descriptor->service_machine_string = (uint8_t *) memory_allocate(
	                                                        sizeof( uint8_t ) * store_descriptor->service_machine_string_size );

	if( store_descriptor->service_machine_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create service machine string.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     store_descriptor->service_machine_string,
	     &( store_header_data[ store_header_data_offset ] ),
	     (size_t) store_descriptor->service_machine_string_size ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy service machine string.",
		 function );

		goto on_error;
	}
	store_header_data_offset += store_descriptor->service_machine_string_size;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( libvshadow_debug_print_utf16_string_value(
		     function,
		     "service machine string\t",
		     store_descriptor->service_machine_string,
		     (size_t) store_descriptor->service_machine_string_size,
		     LIBUNA_ENDIAN_LITTLE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print UTF-16 string value.",
			 function );

			goto on_error;
		}
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( store_header_data_offset < store_block->data_size )
		{
			libcnotify_printf(
			 "%s: trailing data:\n",
			 function );
			libcnotify_print_data(
			 &( store_header_data[ store_header_data_offset ] ),
			 store_block->data_size - store_header_data_offset,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
		libcnotify_printf(
		 "\n" );
		}
	}
#endif
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );

on_error:
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_write(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Reads the store bitmap
 * This function is not multi-thread safe acquire write lock before call
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_bitmap(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     off64_t file_offset,
     libcdata_range_list_t *offset_list,
     off64_t *bitmap_offset,
     off64_t *next_offset,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block = NULL;
	uint8_t *block_data                   = NULL;
	static char *function                 = "libvshadow_store_descriptor_read_store_bitmap";
	off64_t start_offset                  = 0;
	uint32_t value_32bit                  = 0;
	uint16_t block_size                   = 0;
	uint8_t bit_index                     = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( bitmap_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid bitmap offset.",
		 function );

		return( -1 );
	}
	if( next_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid next offset.",
		 function );

		return( -1 );
	}
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     file_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 file_offset );

		goto on_error;
	}
	
	/* Add the block to the block list
	 */
    if (libcdata_list_append_value(
                         store_descriptor->block_list,
                         (intptr_t *) store_block,
                         error ) == -1)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to add store block at offset: 0x%08" PRIx64 " to store block list.",
		 function,
		 store_descriptor->store_header_offset );

		goto on_error;
	}
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf("%s: adding block with offset = 0x08%" PRIx64 " to list\n", function, store_block->offset);
	}
#endif

	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_BITMAP )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		goto on_error;
	}
	*next_offset = store_block->next_offset;

	block_data = &( store_block->data[ sizeof( vshadow_store_block_header_t ) ] );
	block_size = (uint16_t) ( store_block->data_size - sizeof( vshadow_store_block_header_t ) );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: store: %02d block bitmap:\n",
		 function,
		 store_descriptor->index );
		libcnotify_print_data(
		 block_data,
		 block_size,
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif
	start_offset = -1;

	while( block_size > 0 )
	{
		byte_stream_copy_to_uint32_little_endian(
		 block_data,
		 value_32bit );

		for( bit_index = 0;
		     bit_index < 32;
		     bit_index++ )
		{
			if( ( value_32bit & 0x00000001UL ) == 0 )
			{
				if( start_offset >= 0 )
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: store: %02d offset range\t: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
						 function,
						 store_descriptor->index,
						 start_offset,
						 *bitmap_offset,
						 *bitmap_offset - start_offset );
					}
#endif
					if( libcdata_range_list_insert_range(
					     offset_list,
					     (uint64_t) start_offset,
					     (uint64_t) ( *bitmap_offset - start_offset ),
					     NULL,
					     NULL,
					     NULL,
					     error ) != 1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
						 "%s: unable to insert offset range to offset list.",
						 function );

						goto on_error;
					}
					start_offset = -1;
				}
			}
			else
			{
				if( start_offset < 0 )
				{
					start_offset = *bitmap_offset;
				}
			}
			*bitmap_offset += 0x4000;

			value_32bit >>= 1;
		}
		block_data += 4;
		block_size -= 4;
	}
	if( start_offset >= 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store: %02d offset range\t: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
			 function,
			 store_descriptor->index,
			 start_offset,
			 *bitmap_offset,
			 *bitmap_offset - start_offset );
		}
#endif
		if( libcdata_range_list_insert_range(
		     offset_list,
		     (uint64_t) start_offset,
		     (uint64_t) ( *bitmap_offset - start_offset ),
		     NULL,
		     NULL,
		     NULL,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
			 "%s: unable to append offset range to offset list.",
			 function );

			goto on_error;
		}
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
#endif
	return( 1 );

on_error:
	return( -1 );
}

/* Reads the store block list
 * This function is not multi-thread safe acquire write lock before call
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_block_list(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     off64_t file_offset,
     off64_t *next_offset,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block           = NULL;
	libvshadow_block_descriptor_t *block_descriptor = NULL;
	uint8_t *block_data                             = NULL;
	static char *function                           = "libvshadow_store_descriptor_read_store_block_list";
	uint16_t block_size                             = 0;
	int list_entry_number                           = 0;
	int result                                      = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( next_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid next offset.",
		 function );

		return( -1 );
	}
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     file_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 file_offset );

		goto on_error;
	}
	
	/* Add the block to the block list
	 */
    if (libcdata_list_append_value(
                         store_descriptor->block_list,
                         (intptr_t *) store_block,
                         error ) == -1)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to add store block at offset: 0x%08" PRIx64 " to store block list.",
		 function,
		 store_descriptor->store_header_offset );

		goto on_error;
	}
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf("%s: adding block with offset = 0x08%" PRIx64 " to list\n", function, store_block->offset);
	}
#endif
	
	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_INDEX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		goto on_error;
	}
	*next_offset = store_block->next_offset;

	block_data = &( store_block->data[ sizeof( vshadow_store_block_header_t ) ] );
	block_size = (uint16_t) ( store_block->data_size - sizeof( vshadow_store_block_header_t ) );

	while( block_size >= sizeof( vshadow_store_block_list_entry_t ) )
	{
		if( libvshadow_block_descriptor_initialize(
		     &block_descriptor,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create block descriptor.",
			 function );

			goto on_error;
		}
		result = libvshadow_block_descriptor_read_data(
			  block_descriptor,
			  block_data,
			  block_size,
			  store_descriptor->index,
			  error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read block descriptor.",
			 function );

			goto on_error;
		}
		else if( result != 0 )
		{
 			/* add disk location */
 			block_descriptor->descriptor_location = file_offset + 128 + (list_entry_number * sizeof( vshadow_store_block_list_entry_t ));
			block_descriptor->list_entry_number = list_entry_number;
			
			if( libvshadow_block_tree_insert(
			     store_descriptor->forward_block_descriptors_tree,
			     store_descriptor->reverse_block_descriptors_tree,
			     block_descriptor,
			     store_descriptor->index,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to insert block descriptor in tree.",
				 function );

				goto on_error;
			}
			if( libcdata_list_append_value(
			     store_descriptor->block_descriptors_list,
			     (intptr_t *) block_descriptor,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to append block descriptor to list.",
				 function );

				goto on_error;
			}
			
			/* Add the offsets to the file runs if we need to/can
			 * We just add the offset of the data itself, not the block descriptor
			 * The block descriptor location will be covered when the block list is scanned later
			 */
			if (store_descriptor->store_runs != NULL)
			{
				libvshadow_store_run_mark_as_used(&(store_descriptor->store_runs), block_descriptor->offset, error);
			}
			
			block_descriptor = NULL;
		}
		
		if( block_descriptor != NULL )
		{
			if( libvshadow_block_descriptor_free(
			     &block_descriptor,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free block descriptor.",
				 function );

				block_descriptor = NULL;

				goto on_error;
			}
			block_descriptor = NULL;
		}
		
		block_data += sizeof( vshadow_store_block_list_entry_t );
		block_size -= sizeof( vshadow_store_block_list_entry_t );
		list_entry_number++;
	}
	return( 1 );

on_error:
	if( block_descriptor != NULL )
	{
		libvshadow_block_descriptor_free(
		 &block_descriptor,
		 NULL );
	}
	return( -1 );
}

/* Reads the store block range list
 * This function is not multi-thread safe acquire write lock before call
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_block_range_list(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     off64_t file_offset,
     off64_t *next_offset,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block                       = NULL;
	libvshadow_block_range_descriptor_t *block_range_descriptor = NULL;
	uint8_t *block_data                                         = NULL;
	static char *function                                       = "libvshadow_store_descriptor_read_store_block_range_list";
	uint16_t block_size                                         = 0;
	int result                                                  = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( next_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid next offset.",
		 function );

		return( -1 );
	}
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     file_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 file_offset );

		goto on_error;
	}
	
	/* Add the block to the block list
	 */
    if (libcdata_list_append_value(
                         store_descriptor->block_list,
                         (intptr_t *) store_block,
                         error ) == -1)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to add store block at offset: 0x%08" PRIx64 " to store block list.",
		 function,
		 store_descriptor->store_header_offset );

		goto on_error;
	}
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf("%s: adding block with offset = 0x08%" PRIx64 " to list\n", function, store_block->offset);
	}
#endif
	
	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_BLOCK_RANGE )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		goto on_error;
	}
	*next_offset = store_block->next_offset;

	block_data = &( store_block->data[ sizeof( vshadow_store_block_header_t ) ] );
	block_size = (uint16_t) ( store_block->data_size - sizeof( vshadow_store_block_header_t ) );

	while( block_size >= sizeof( vshadow_store_block_list_entry_t ) )
	{
		if( libvshadow_block_range_descriptor_initialize(
		     &block_range_descriptor,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create block range descriptor.",
			 function );

			goto on_error;
		}
		result = libvshadow_block_range_descriptor_read_data(
			  block_range_descriptor,
			  block_data,
			  block_size,
			  store_descriptor->index,
			  error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read block range descriptor.",
			 function );

			goto on_error;
		}
		else if( result != 0 )
		{
/* TODO do something useful with the range descriptors */
/* I've got you man. Did something useful with them. (Sort of).*/
		}
		if( block_range_descriptor != NULL )
		{
			if( libvshadow_block_range_descriptor_free(
			     &block_range_descriptor,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free block range descriptor.",
				 function );

				block_range_descriptor = NULL;

				goto on_error;
			}
			block_range_descriptor = NULL;
		}
		block_data += sizeof( vshadow_store_block_range_list_entry_t );
		block_size -= sizeof( vshadow_store_block_range_list_entry_t );
	}
	return( 1 );

on_error:
	if( block_range_descriptor != NULL )
	{
		libvshadow_block_range_descriptor_free(
		 &block_range_descriptor,
		 NULL );
	}
	return( -1 );
}

/* Reads the block descriptors
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_block_descriptors(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     libcerror_error_t **error )
{
	static char *function      = "libvshadow_store_descriptor_read_block_descriptors";
	off64_t bitmap_offset      = 0;
	off64_t store_block_offset = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( store_descriptor->block_descriptors_read == 0 )
	{
		bitmap_offset      = 0;
		store_block_offset = store_descriptor->store_bitmap_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_bitmap(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     store_descriptor->block_offset_list,
			     &bitmap_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store bitmap.",
				 function );

				goto on_error;
			}
		}
		bitmap_offset      = 0;
		store_block_offset = store_descriptor->store_previous_bitmap_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_bitmap(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     store_descriptor->previous_block_offset_list,
			     &bitmap_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store previous bitmap.",
				 function );

				goto on_error;
			}
		}
		store_block_offset = store_descriptor->store_block_list_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_block_list(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store block list.",
				 function );

				goto on_error;
			}
		}
		store_block_offset = store_descriptor->store_block_range_list_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_block_range_list(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store block range list.",
				 function );

				goto on_error;
			}
		}
		store_descriptor->block_descriptors_read = 1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );

on_error:
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_write(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Reads data at the specified offset into a buffer
 * Returns the number of bytes read or -1 on error
 */
ssize_t libvshadow_store_descriptor_read_buffer(
         libvshadow_store_descriptor_t *store_descriptor,
         libbfio_handle_t *file_io_handle,
         uint8_t *buffer,
         size_t buffer_size,
         off64_t offset,
         libvshadow_store_descriptor_t *active_store_descriptor,
         libcerror_error_t **error )
{
	libcdata_tree_node_t *tree_node                         = NULL;
	libvshadow_block_descriptor_t *block_descriptor         = NULL;
	libvshadow_block_descriptor_t *overlay_block_descriptor = NULL;
	libvshadow_block_descriptor_t *reverse_block_descriptor = NULL;
	static char *function                                   = "libvshadow_store_descriptor_read_buffer";
	intptr_t *value                                         = NULL;
	off64_t block_descriptor_offset                         = 0;
	off64_t block_offset                                    = 0;
	off64_t block_range_offset                              = 0;
	off64_t overlay_block_offset                            = 0;
	off64_t previous_block_range_offset                     = 0;
	size64_t block_range_size                               = 0;
	size64_t previous_block_range_size                      = 0;
	size_t buffer_offset                                    = 0;
	size_t block_size                                       = 0;
	size_t read_size                                        = 0;
	ssize_t read_count                                      = 0;
	uint32_t overlay_bitmap                                 = 0;
	uint32_t relative_block_offset                          = 0;
	uint8_t bit_count                                       = 0;
	int in_block_descriptor_list                            = 0;
	int in_current_bitmap                                   = 0;
	int in_previous_bitmap                                  = 0;
	int in_reverse_block_descriptor_list                    = 0;
	int result                                              = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( store_descriptor->has_in_volume_store_data == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor - missing in-volume store data.",
		 function );

		return( -1 );
	}
	if( active_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid active store descriptor.",
		 function );

		return( -1 );
	}
	/* This function will acquire the write lock
	 */
	if( libvshadow_store_descriptor_read_block_descriptors(
	     store_descriptor,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read block descriptors.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: store: %02d requested offset: 0x%08" PRIx64 "\n",
		 function,
		 store_descriptor->index,
		 offset );
	}
#endif
	while( buffer_size > 0 )
	{
		block_offset          = offset;
		relative_block_offset = (uint32_t) (offset % 0x4000);
		block_size            = 0x4000 - relative_block_offset;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store: %02d block offset: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->index,
			 block_offset );
		}
#endif
		in_block_descriptor_list         = 0;
		in_reverse_block_descriptor_list = 0;
		in_current_bitmap                = 0;
		in_previous_bitmap               = 0;

		result = libcdata_btree_get_value_by_value(
		          store_descriptor->forward_block_descriptors_tree,
		          (intptr_t *) &block_offset,
		          (int (*)(intptr_t *, intptr_t *, libcerror_error_t **)) &libvshadow_block_descriptor_compare_range_by_original_offset_value,
		          &tree_node,
		          (intptr_t **) &block_descriptor,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve block range for offset: 0x%08" PRIx64 ".",
			 function,
			 block_offset );

			goto on_error;
		}
		else if( result != 0 )
		{
			if( block_descriptor == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				 "%s: missing block descriptor.",
				 function );

				goto on_error;
			}
			in_block_descriptor_list = 1;

			if( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_FORWARDER ) != 0 )
			{
				block_descriptor_offset = block_descriptor->relative_offset;
			}
			else
			{
				block_descriptor_offset = block_descriptor->offset;
			}
			if( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_OVERLAY ) != 0 )
			{
				overlay_block_descriptor = block_descriptor;
			}
			else
			{
				overlay_block_descriptor = block_descriptor->overlay;
			}
			if( overlay_block_descriptor != NULL )
			{
				if( store_descriptor->index != active_store_descriptor->index )
				{
					if( block_descriptor == overlay_block_descriptor )
					{
						block_descriptor         = NULL;
						in_block_descriptor_list = 0;
					}
				}
				else
				{
					overlay_block_offset = overlay_block_descriptor->original_offset;
					overlay_bitmap       = overlay_block_descriptor->bitmap;

					bit_count = 32;

					while( overlay_block_offset < block_offset )
					{
						overlay_bitmap >>= 1;

						overlay_block_offset += 512;

						bit_count--;

						if( bit_count == 0 )
						{
							break;
						}
					}
					if( ( overlay_bitmap & 0x00000001UL ) != 0 )
					{
						block_descriptor_offset = overlay_block_descriptor->offset;
						block_descriptor        = overlay_block_descriptor;

						block_size = 0;

						while( ( overlay_bitmap & 0x00000001UL ) != 0 )
						{
							overlay_bitmap >>= 1;

							block_size += 512;

							bit_count--;

							if( bit_count == 0 )
							{
								break;
							}
						}
					}
					else
					{
						if( block_descriptor == overlay_block_descriptor )
						{
							block_descriptor         = NULL;
							in_block_descriptor_list = 0;
						}
						block_size = 0;

						while( ( overlay_bitmap & 0x00000001UL ) == 0 )
						{
							overlay_bitmap >>= 1;

							block_size += 512;

							bit_count--;

							if( bit_count == 0 )
							{
								break;
							}
						}
					}
				}
			}
		}
		if( in_block_descriptor_list == 0 )
		{
			/* Only the most recent store seems to bother checking the current bitmap
			 */
			if( ( store_descriptor->next_store_descriptor == NULL )
			 && ( store_descriptor->index == active_store_descriptor->index ) )
			{
				if( store_descriptor->reverse_block_descriptors_tree == NULL )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
					 "%s: invalid store descriptor - missing reverse block descriptors tree.",
					 function );

					goto on_error;
				}
				result = libcdata_btree_get_value_by_value(
					  store_descriptor->reverse_block_descriptors_tree,
					  (intptr_t *) &block_offset,
					  (int (*)(intptr_t *, intptr_t *, libcerror_error_t **)) &libvshadow_block_descriptor_compare_range_by_relative_offset_value,
					  &tree_node,
					  (intptr_t **) &reverse_block_descriptor,
					  error );

				if( result == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve reverse block descriptor from root node.",
					 function );

					goto on_error;
				}
				in_reverse_block_descriptor_list = result;

				result = libcdata_range_list_get_range_at_offset(
					  store_descriptor->block_offset_list,
					  (uint64_t) block_offset,
					  (uint64_t *) &block_range_offset,
					  (uint64_t *) &block_range_size,
					  &value,
					  error );

				if( result == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve offset: 0x%08" PRIx64 " from block offset list.",
					 function,
					 offset );

					goto on_error;
				}
#if defined( HAVE_DEBUG_OUTPUT )
				else if( result != 0 )
				{
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: store: %02d block offset list: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
						 function,
						 store_descriptor->index,
						 block_range_offset,
						 block_range_offset + block_range_size,
						 block_range_size );
					}
				}
#endif
				in_current_bitmap = result;

				if( store_descriptor->store_previous_bitmap_offset != 0 )
				{
					result = libcdata_range_list_get_range_at_offset(
						  store_descriptor->previous_block_offset_list,
						  (uint64_t) block_offset,
						  (uint64_t *) &previous_block_range_offset,
						  (uint64_t *) &previous_block_range_size,
						  &value,
						  error );

					if( result == -1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						 "%s: unable to retrieve offset: 0x%08" PRIx64 " from previous block offset list.",
						 function,
						 offset );

						goto on_error;
					}
#if defined( HAVE_DEBUG_OUTPUT )
					else if( result != 0 )
					{
						if( libcnotify_verbose != 0 )
						{
							libcnotify_printf(
							 "%s: store: %02d previous block offset list: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
							 function,
							 store_descriptor->index,
							 previous_block_range_offset,
							 previous_block_range_offset + previous_block_range_size,
							 previous_block_range_size );
						}
					}
#endif
					in_previous_bitmap = result;
				}
				else
				{
					in_previous_bitmap = 1;
				}
			}
		}
		block_descriptor_offset += relative_block_offset;

		if( buffer_size > block_size )
		{
			read_size = block_size;
		}
		else
		{
			read_size = buffer_size;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store: %02d range: 0x%08" PRIx64 " - 0x%08" PRIx64 " size: %" PRIzd "",
			 function,
			 store_descriptor->index,
			 block_offset,
			 block_offset + block_size,
			 block_size );

			if( block_descriptor != NULL )
			{
				libcnotify_printf(
				 ", flags: 0x%08" PRIx32 "",
				 block_descriptor->flags );
			}
			libcnotify_printf(
			 "\n" );

			if( in_block_descriptor_list != 0 )
			{
				libcnotify_printf(
				 "\tIn block list\n" );
			}
			if( block_descriptor != NULL )
			{
				if( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_OVERLAY ) != 0 )
				{
					libcnotify_printf(
					 "\tIs overlay\n" );
				}
			}
			if( in_reverse_block_descriptor_list != 0 )
			{
				libcnotify_printf(
				 "\tIn reverse block list\n" );
			}
			if( in_current_bitmap != 0 )
			{
				libcnotify_printf(
				 "\tIn current bitmap\n" );
			}
			if( ( store_descriptor->store_previous_bitmap_offset != 0 )
			 && ( in_previous_bitmap != 0 ) )
			{
				libcnotify_printf(
				 "\tIn previous bitmap\n" );
			}
			libcnotify_printf(
			 "\n" );
		}
#endif
		if( in_block_descriptor_list != 0 )
		{
			if( ( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_FORWARDER ) != 0 )
			 && ( store_descriptor->next_store_descriptor != NULL ) )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from next store at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_descriptor_offset );
				}
#endif
				read_count = libvshadow_store_descriptor_read_buffer(
					      store_descriptor->next_store_descriptor,
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      block_descriptor_offset,
					      active_store_descriptor,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from next store descriptor.",
					 function );

					goto on_error;
				}
			}
			else
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from current volume at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_descriptor_offset );
				}
#endif
				if( libbfio_handle_seek_offset(
				     file_io_handle,
				     block_descriptor_offset,
				     SEEK_SET,
				     error ) == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_SEEK_FAILED,
					 "%s: unable to seek block offset: %" PRIi64 ".",
					 function,
					 block_descriptor_offset );

					goto on_error;
				}
				read_count = libbfio_handle_read_buffer(
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from file IO handle.",
					 function );

					goto on_error;
				}
			}
		}
		else
		{
			/* Check if the next store defines the block
			 */
			if( store_descriptor->next_store_descriptor != NULL )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from next store at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_offset );
				}
#endif
				read_count = libvshadow_store_descriptor_read_buffer(
					      store_descriptor->next_store_descriptor,
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      block_offset,
					      active_store_descriptor,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from next store descriptor.",
					 function );

					goto on_error;
				}
			}
			else if( ( in_reverse_block_descriptor_list == 0 )
			      && ( in_current_bitmap != 0 )
			      && ( in_previous_bitmap != 0 ) )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d filling block with zero bytes\n",
					 function,
					 store_descriptor->index,
					 block_offset,
					 read_size );
				}
#endif
				if( memory_set(
				     &( buffer[ buffer_offset ] ),
				     0,
				     read_size ) == NULL )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_MEMORY,
					 LIBCERROR_MEMORY_ERROR_SET_FAILED,
					 "%s: unable to clear buffer.",
					 function );

					goto on_error;
				}
				read_count = (ssize_t) read_size;
			}
			else
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from current volume at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_offset );
				}
#endif
				if( libbfio_handle_seek_offset(
				     file_io_handle,
				     block_offset,
				     SEEK_SET,
				     error ) == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_SEEK_FAILED,
					 "%s: unable to seek block offset: %" PRIi64 ".",
					 function,
					 block_offset );

					goto on_error;
				}
				read_count = libbfio_handle_read_buffer(
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from file IO handle.",
					 function );

					goto on_error;
				}
			}
		}
		offset        += read_count;
		buffer_offset += read_count;
		buffer_size   -= read_count;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "\n" );
		}
#endif
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( (ssize_t) buffer_offset );

on_error:
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_read(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Write version of the above function */

/* Reads data at the specified offset into a buffer
 * Returns the number of bytes read or -1 on error
 */
ssize_t libvshadow_store_descriptor_write_buffer(
         libvshadow_store_descriptor_t *store_descriptor,
         libbfio_handle_t *file_io_handle,
         uint8_t *buffer,
         size_t buffer_size,
         off64_t offset,
         libvshadow_store_descriptor_t *active_store_descriptor,
         libcerror_error_t **error )
{
    static char *function                                   = "libvshadow_store_descriptor_write_buffer";
	libcdata_tree_node_t *block_tree_node = NULL;
	libcdata_list_element_t *last_descriptor_element = NULL;
	libvshadow_block_descriptor_t *temp_block_descriptor = NULL;
	libvshadow_block_descriptor_t *new_block_descriptor = NULL;
	libvshadow_store_block_t *temp_block = NULL;
	libvshadow_store_block_t *new_block = NULL;
	libcdata_list_element_t *temp_block_element = NULL;
	libcdata_list_element_t *temp_block_element2 = NULL;
	off64_t chunk_offset;
	off64_t temp_offset1;
	off64_t next_descriptor_address = 0;
	size64_t write_size;
	uint8_t descriptor_buffer[32];
	uint8_t data_buffer[16384];
	uint32_t overlay_bitmap;
	int i;
	int result;


#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
		libcnotify_printf("%s: Entered function, specified write offset = 0x%08" PRIx64 " \n", function, offset);
#endif
    
	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( store_descriptor->has_in_volume_store_data == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor - missing in-volume store data.",
		 function );

		return( -1 );
	}
	if( active_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid active store descriptor.",
		 function );

		return( -1 );
	}
	/* This function will acquire the write lock
	 */
	if( libvshadow_store_descriptor_read_block_descriptors(
	     store_descriptor,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read block descriptors.",
		 function );

		return( -1 );
	}

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: store: %02d requested offset: 0x%08" PRIx64 "\n",
		 function,
		 store_descriptor->index,
		 offset );
	}
#endif

	/* What 16K chunk are we writing to?
	 */
	chunk_offset = offset - (offset % 0x4000);
	
	/* Figure out how much data we will be writing this function call
	 * For simplicity we only write up to 16K at a time
	 */
    write_size = ((offset / 0x4000) * 0x4000 + 0x4000) - offset;
    if (buffer_size < write_size)
        write_size = buffer_size;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
			"%s: chunk offset = 0x%" PRIx64 " offset = %" PRIx64 " write size = %" PRIx64 "\n",
			function, chunk_offset, offset, write_size);
	}
#endif

	/* First let's see if the requested write offset already has a block descriptor
	 */
	result = libcdata_btree_get_value_by_value(
			  store_descriptor->forward_block_descriptors_tree,
			  (intptr_t *) &chunk_offset,
			  (int (*)(intptr_t *, intptr_t *, libcerror_error_t **)) &libvshadow_block_descriptor_compare_range_by_original_offset_value,
			  &block_tree_node,
			  (intptr_t **) &temp_block_descriptor,
			  error );

	/* Is there already a block for this offset?
	 */
	if (result == 1)
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
				"%s: found an existing descriptor at offset = %" PRIx64 " type = %d original offset = %" PRIx64 "\n",
				function, temp_block_descriptor->descriptor_location, temp_block_descriptor->flags, temp_block_descriptor->original_offset);
		}
#endif
		
		/* If we're writing an entire block
		 * We just replace the current block and clear any overlays for it
		 */
		if (write_size == 16384)
		{
			/* If offset = 0, this is probably a forwarder block.
			 * To convert it to a regular block, we need to find an address for it's data, and set the offsets in the block descriptor.
			 */
			if (temp_block_descriptor->offset == 0)
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: existing descriptor has offset = 0\n",
						function);
				}
#endif
				temp_block_descriptor->offset = libvshadow_store_run_get_next_free(&(store_descriptor->store_runs), error);
				if (temp_block_descriptor->offset == 0)
				{
					libcnotify_printf("%s: store: %02d Couldn't get free address for converting a forwader block at  0x%08" PRIx64 "to a normal block\n",
						function, store_descriptor->index, temp_block_descriptor->descriptor_location);
					goto on_error;
				}
				temp_block_descriptor->relative_offset = temp_block_descriptor->offset - store_descriptor->store_header_offset;
				/* Write the new addresses to disk
				 */
				memory_set(descriptor_buffer, 0, 32);
				memory_copy(descriptor_buffer + 8, &temp_block_descriptor->relative_offset, 8);
				memory_copy(descriptor_buffer + 16, &temp_block_descriptor->offset, 8);
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->descriptor_location + 8, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer + 8, 16, error) != 16)
				{
					libcnotify_printf("%s: store: %02d error writing new offsets to probable forwarder block at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->descriptor_location);
					goto on_error;
				}

#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: wrote new offset = %" PRIx64 " relative offset = %" PRIx64 "\n",
						function, temp_block_descriptor->offset, temp_block_descriptor->relative_offset);
				}
#endif
			}
			
			/* Seek to the correct location to write the new data
			 */
			libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->offset, SEEK_SET, error);
		    if (libbfio_handle_write_buffer(file_io_handle, buffer, write_size, error) != 16384)
			{
				libcnotify_printf("%s: store: %02d error writing new data to offset: 0x%08" PRIx64 "\n",
					function, store_descriptor->index, offset);
				goto on_error;
			} 
			
			/* If the existing block was not a regular block, change it to a regular block
			 */
			if (temp_block_descriptor->flags != 0)
			{
				memory_set(descriptor_buffer, 0, 32);
				/* We're writing over the Flags (to make it a normal descriptor) and the Allocation Bitmap (which should be 0 for regular blocks)
				 */
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->descriptor_location+24, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer, 8, error) != 8)
				{
					libcnotify_printf("%s: store: %02d error changing block descriptor flag to '0' on descriptor at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->descriptor_location);
					goto on_error;
				}
				temp_block_descriptor->flags = 0;
				
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: just updated flags to 0 for descriptor at location = %" PRIx64 "\n",
						function, temp_block_descriptor->descriptor_location);
				}
#endif
			}
			
			/* If the existing block had an overlay associated with it, clear it (in memory and on disk)
			 */
			if (temp_block_descriptor->overlay != NULL)
			{
				/* Write over the overlay's data
				 */
				memory_set(data_buffer, 0, 16384);
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->overlay->offset, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, data_buffer, 16384, error) != 16384)
				{
					libcnotify_printf("%s: store: %02d error writing over overlay block descriptor's data at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->overlay->offset);
					goto on_error;
				}
				/* Change the overlay descriptor's flag to 4 ('Unused') and clear it's offsets
				 * I'm not sure if setting the relative offset to 0 is considered acceptable or not (for an unused descriptor)
				 */
				memory_set(descriptor_buffer, 0, 32);
				descriptor_buffer[24] = 0x04;
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->overlay->descriptor_location, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer, 32, error) != 32)
				{
					libcnotify_printf("%s: store: %02d error clearing overlay descriptor at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->overlay->descriptor_location);
					goto on_error;
				}
				/* Remove the overlay pointer from the regular descriptors stuct
				 */
				temp_block_descriptor->overlay = NULL;
				
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: just cleared overlay for existing block with the overlay's offset = %" PRIx64 " overlay's descriptor location = %" PRIx64 "\n",
						function, temp_block_descriptor->overlay->offset, temp_block_descriptor->overlay->descriptor_location);
				}
#endif
			}
		}
		
		/* Else, we are writing less than 16K
		 */
		else
		{
#if defined( HAVE_DEBUG_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
					"%s: writing less than 16K with existing block\n",
					function);
			}
#endif
			
			/* If the current block is a regular block and doesn't have an overlay, just write the new data (partially) over the old data
			 */
			if (temp_block_descriptor->flags == 0 && temp_block_descriptor->overlay == NULL)
			{
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->offset + (offset % 16384), SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, buffer, write_size, error) != write_size)
				{
					libcnotify_printf("%s: store: %02d error writing partial 16K chunk over existing descriptor data at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->offset + (offset % 16384));
					goto on_error;
				}
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: existing block was regular with no overlay. Writing data to offset = %" PRIx64 "\n",
						function, temp_block_descriptor->offset + (offset % 16384));
				}
#endif
			}
			/* Else, if the current block is a regular block that also already has an overlay,
			 * then merge the current overlay on to the current regular block and write the new data over the (now merged) regular block data.
			 * This is done so we don't get bits of inter-snapshot data left behind in the regular block.
			 */
			else if (temp_block_descriptor->flags == 0 && temp_block_descriptor->overlay != NULL)
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: existing block was regular with an overlay. Going to read data from offset, then write back.\n",
						function);
				}
#endif
				/* So the easiest way to get the merged data is just to read that 16K from the snapshot like normal
				 */
				if (libvshadow_store_descriptor_read_buffer(
					store_descriptor, file_io_handle,
					data_buffer, 0x4000, 
					temp_block_descriptor->original_offset,
					active_store_descriptor, error) != 0x4000)
				{
					libcnotify_printf("%s: store: %02d error reading 16K chunk from (original offset): 0x%08" PRIx64 " so we can merge overlays during write\n",
						function, store_descriptor->index, temp_block_descriptor->original_offset);
					goto on_error;
				}
				
				/* Write the updated data to the regular blocks offset
				 */
				memory_copy(data_buffer + (offset % 16384), buffer, write_size);
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->offset, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, data_buffer, 16384, error) != 16384)
				{
					libcnotify_printf("%s: store: %02d error writing merged 16K chunk over existing descriptor data at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->offset);
					goto on_error;
				}
				/* Now clear the overlay block that we don't need anymore
				 */
				memory_set(descriptor_buffer, 0, 32);
				descriptor_buffer[24] = 0x04;
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->overlay->descriptor_location, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer, 32, error) != 32)
				{
					libcnotify_printf("%s: store: %02d error clearing overlay descriptor at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->overlay->descriptor_location);
					goto on_error;
				}
				/* Remove the overlay pointer from the regular descriptors stuct
				 */
				temp_block_descriptor->overlay = NULL;
			}
			/* Else, the current block is either an overlay or a forwarder (or unkown, which we'll write over with an overlay)
			 */
			else
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: existing block was either overlay or forwarded (or unknown)\n",
						function);
				}
#endif
				
				/* First, grab the current data
				 */
				if (libvshadow_store_descriptor_read_buffer(
					store_descriptor, file_io_handle,
					data_buffer, 0x4000, 
					temp_block_descriptor->original_offset,
					active_store_descriptor, error) != 0x4000)
				{
					libcnotify_printf("%s: store: %02d error reading 16K chunk from (original offset): 0x%08" PRIx64 " so we can merge new data into current overlay or replace forwarder\n",
						function, store_descriptor->index, temp_block_descriptor->original_offset);
					goto on_error;
				}
				/* Copy the new data over it
				 */
				memory_copy(data_buffer + (offset % 16384), buffer, write_size);
				/* Create a bitmap for the new data
				 */
				overlay_bitmap = 0;
				for (i = (offset % 0x4000) / 512; i <= ((offset % 0x4000) + write_size - 1) / 512; i++)
				{
					overlay_bitmap += ((uint32_t)1) << ((7 - (i % 8)) + (i / 8) * 8);
				}
				/* If this is a forwarder block, allocate some disk space for it
				 */
				if (temp_block_descriptor->offset == 0)
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
							"%s: Found block descriptor had offset = 0. Finding space for data, setting offset and changing descriptor type to overlay.\n",
							function);
					}
#endif
					temp_block_descriptor->offset = libvshadow_store_run_get_next_free(&(store_descriptor->store_runs), error);
					if (temp_block_descriptor->offset == 0)
					{
						libcnotify_printf("%s: store: %02d Couldn't get free address for converting a forwader block at  0x%08" PRIx64 "to a normal block\n",
							function, store_descriptor->index, temp_block_descriptor->descriptor_location);
						goto on_error;
					}
					temp_block_descriptor->relative_offset = temp_block_descriptor->offset - store_descriptor->store_header_offset;
					/* Write the new addresses to disk
					 */
					memory_set(descriptor_buffer, 0, 32);
					descriptor_buffer[20] = 0x02;
					memory_copy(descriptor_buffer + 8, &temp_block_descriptor->relative_offset, 8);
					memory_copy(descriptor_buffer + 16, &temp_block_descriptor->offset, 8);
					libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->descriptor_location + 8, SEEK_SET, error);
					if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer + 8, 20, error) != 20)
					{
						libcnotify_printf("%s: store: %02d error writing new offsets to probable forwarder block at: 0x%08" PRIx64 "\n",
							function, store_descriptor->index, temp_block_descriptor->descriptor_location);
						goto on_error;
					}
				}
				
				/* Force the block to be an overlay
				 */
				temp_block_descriptor->flags = 2;
				
				/* Merge the new and existing bitmaps and write the merged bitmap to disk
				 */
				if (temp_block_descriptor->overlay != NULL)
				{
					/* If this is an unknown (or something) and has an overlay itself, copy the overlay's bitmap for later user
					 */
					overlay_bitmap |= temp_block_descriptor->overlay->bitmap;
					libcnotify_printf(
						"%s: current descriptor has an overlay, merging that overlay's bitmap into new bitmap. overlay's bitmap: %x\n",
						function, temp_block_descriptor->overlay->bitmap);
				}
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: old bitmap: %x new data bitmap: %x combined bitmap: %x\n",
						function, temp_block_descriptor->bitmap, overlay_bitmap , temp_block_descriptor->bitmap | overlay_bitmap);
				}
#endif
				temp_block_descriptor->bitmap |= overlay_bitmap;
				memory_copy(descriptor_buffer + 28, &temp_block_descriptor->bitmap, 4);
				memory_copy(descriptor_buffer + 24, &temp_block_descriptor->flags, 4); /* force flag to be overlay */
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->descriptor_location + 24, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer + 24, 8, error) != 8)
				{
					libcnotify_printf("%s: store: %02d error writing new offsets to probable forwarder block at: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->descriptor_location);
					goto on_error;
				}
				/* Zero out the parts of the 16K overlay data that isn't being updated/overlayed
				 */
				for (i = 0; i < 32; i++)
				{
					if ( (temp_block_descriptor->bitmap & ( ((uint32_t)1) << i )) == 0 )
					{
						memory_set(data_buffer + (i * 512), 0, 512);
					}
				}
				
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: writing merged data to disk at offset 0x%08" PRIx64 "\n",
						function, temp_block_descriptor->offset);
				}
#endif
				/* Write out the 16K of data
				 */
				libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->offset, SEEK_SET, error);
				if (libbfio_handle_write_buffer(file_io_handle, data_buffer, 16384, error) != 16384)
				{
					libcnotify_printf("%s: store: %02d error writing new merged overlay data to offset: 0x%08" PRIx64 "\n",
						function, store_descriptor->index, temp_block_descriptor->offset);
					goto on_error;
				}
				/* If the existing block itself has an overlay, clear that out
				 */
				if (temp_block_descriptor->overlay != NULL)
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
							"%s: Clearing out overlay for existing block descriptor\n",
							function);
					}
#endif
					/* Write over the overlay's data
					 */
					memory_set(data_buffer, 0, 16384);
					libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->overlay->offset, SEEK_SET, error);
					if (libbfio_handle_write_buffer(file_io_handle, data_buffer, 16384, error) != 16384)
					{
						libcnotify_printf("%s: store: %02d error writing over overlay block descriptor's data at: 0x%08" PRIx64 "\n",
							function, store_descriptor->index, temp_block_descriptor->overlay->offset);
						goto on_error;
					}
					/* Change the overlay descriptor's flag to 4 ('Unused') and clear it's offsets
					 * I'm not sure if setting the relative offset to 0 is considered acceptable or not (for an unused descriptor)
					 */
					memory_set(descriptor_buffer, 0, 32);
					descriptor_buffer[24] = 0x04;
					libbfio_handle_seek_offset(file_io_handle, temp_block_descriptor->overlay->descriptor_location, SEEK_SET, error);
					if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer, 32, error) != 32)
					{
						libcnotify_printf("%s: store: %02d error clearing overlay descriptor at: 0x%08" PRIx64 "\n",
							function, store_descriptor->index, temp_block_descriptor->overlay->descriptor_location);
						goto on_error;
					}
					/* Remove the overlay pointer from the regular descriptors stuct
					 */
					temp_block_descriptor->overlay = NULL;
				}
				
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: finished writing partial data over existing overlay or forwarder (or unknown, hopefully it will be fine)\n",
						function);
				}
#endif
			} /* end writing over overlay or forwarder */
		} /* end writing less than 16K */
	} /* end offset has existing block */
	
	/* Else, offset doesn't have a block descriptor yet, create a new one
	 */
	else
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
				"%s: No existing block was found, creating a new one\n",
				function);
		}
#endif
		
		/* First we check if there is space for a new descriptor in the current last block
		 * We grab the last block descriptor and check which block descriptor number it is
		 */
		libcdata_list_get_last_element(store_descriptor->block_descriptors_list, &last_descriptor_element, error);
		libcdata_list_element_get_value(last_descriptor_element, (intptr_t **) &temp_block_descriptor, error);
		if (temp_block_descriptor->list_entry_number >= 507)
		{
#if defined( HAVE_DEBUG_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
					"%s: last block descriptor in list had entry number 507, creating a new block\n",
					function);
			}
#endif
			
			/* Each block list block can store 508 entries.
			 * If it's full, we need to create a new store block.
			 * list_entry_number starts at 0, fyi
			 */
			
			/* Find the last block list block, so we can copy the header info to the new block we're creating
			 * and so we can update the address of the next block to the new one we're creating
			 */
			libcdata_list_get_last_element(store_descriptor->block_list, &temp_block_element2, error);
			while (temp_block_element2)
			{
				temp_block_element = temp_block_element2;
				libcdata_list_element_get_value(temp_block_element, (intptr_t **) &temp_block, error);
				if (temp_block->record_type == 3)
                {
					break;
                }
				libcdata_list_element_get_previous_element(temp_block_element, &temp_block_element2, error);
			}
			
			/* We're not going to be storing descriptors in this block in memory, so data section just needs to hold the header info (128 bytes)
			 */
			libvshadow_store_block_initialize(&new_block, 128, error);
			memory_copy(new_block->data, temp_block->data, 128);
			
			/* Get a free address for the new block on disk
			 */
			temp_offset1 = libvshadow_store_run_get_next_free(&(store_descriptor->store_runs), error);
			memory_copy(new_block->data + 32, &temp_offset1, 8);
			new_block->offset = temp_offset1;
			if (new_block->offset == 0)
			{
				libcnotify_printf("%s: store: %02d Couldn't get free address for a new block\n",
					function, store_descriptor->index);
				goto on_error;
			}
			
			/* Update the next block address in the last block
			 */
            temp_block->next_offset = temp_block->offset;
			libbfio_handle_seek_offset(file_io_handle, temp_block->offset + 40, SEEK_SET, error);
			if (libbfio_handle_write_buffer(file_io_handle, (uint8_t*)&temp_offset1, 8, error) != 8)
			{
				libcnotify_printf("%s: store: %02d Error updating block next block at: 0x%08" PRIx64 "\n",
					function, store_descriptor->index, temp_block->offset + 40);
				goto on_error;
			}
			
			/* Figure out what the relative offset is and update the new block with it
			 */
			new_block->relative_offset = new_block->offset - (temp_block->offset - temp_block->relative_offset);
			memory_copy(new_block->data + 24, &(new_block->relative_offset), 8);
			
            /* Fill in the record type on the new block
             */
            new_block->data[20] = 3;
            new_block->record_type = 3;
            
			/* Write the new block to disk
			 */
			memory_set(data_buffer, 0, 16384);
			memory_copy(data_buffer, new_block->data, 128);
			libbfio_handle_seek_offset(file_io_handle, new_block->offset, SEEK_SET, error);
			if (libbfio_handle_write_buffer(file_io_handle, data_buffer, 16384, error) != 16384)
			{
				libcnotify_printf("%s: store: %02d Error writing new block at: 0x%08" PRIx64 "\n",
					function, store_descriptor->index, new_block->offset);
				goto on_error;
			}
			
			/* Add the new block to the block list
			 */
			if( libcdata_list_append_value(
			     store_descriptor->block_list,
			     (intptr_t *) new_block,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to append new block to list.",
				 function );

				goto on_error;
			}
			
#if defined( HAVE_DEBUG_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
					"%s: new block written to offset = %" PRIx64 "\n",
					function, new_block->offset);
			}
#endif
			
			next_descriptor_address = new_block->offset + 128;
		}
		else
			next_descriptor_address = temp_block_descriptor->descriptor_location + 32;
		
		/* Now we get to actually create the new block descriptor
		 */
		libvshadow_block_descriptor_initialize(&new_block_descriptor, error);
		new_block_descriptor->descriptor_location = next_descriptor_address;
		new_block_descriptor->original_offset = offset - (offset % 16384);
		/*
		new_block_descriptor->index = temp_block_descriptor->index + 1;
		new_block_descriptor->reverse_index = temp_block_descriptor->reverse_index + 1;
		 */
		new_block_descriptor->offset = libvshadow_store_run_get_next_free(&(store_descriptor->store_runs), error);
		if (new_block_descriptor->offset == 0)
		{
				libcnotify_printf("%s: store: %02d Couldn't get free address for new block descriptor data at original offset 0x%08" PRIx64 "\n",
					function, store_descriptor->index, new_block_descriptor->original_offset);
				libvshadow_block_descriptor_free(&new_block_descriptor, error);
				goto on_error;
		}
		new_block_descriptor->relative_offset = new_block_descriptor->offset - (temp_block_descriptor->offset - temp_block_descriptor->relative_offset);
		if (temp_block_descriptor->list_entry_number == 507)
			new_block_descriptor->list_entry_number = 0;
		else
			new_block_descriptor->list_entry_number = temp_block_descriptor->list_entry_number + 1;
		if (write_size == 16384)
		{
			new_block_descriptor->flags = 0;
			memory_copy(data_buffer, buffer, 16384);
		}
		else
		{
			new_block_descriptor->flags = 2;
			/* First, grab the current data
			 */
			if (libvshadow_store_descriptor_read_buffer(
				store_descriptor, file_io_handle,
				data_buffer, 0x4000, 
				new_block_descriptor->original_offset,
				active_store_descriptor, error) != 0x4000)
			{
				libcnotify_printf("%s: store: %02d error reading 16K chunk from (original offset): 0x%08" PRIx64 " so we can create a new overlay for it\n",
					function, store_descriptor->index, new_block_descriptor->original_offset);
				goto on_error;
			}
			
			/* Copy the new data over it
			 */
			memory_copy(data_buffer + (offset % 16384), buffer, write_size);

			/* Create a bitmap for the new data
			 */
			new_block_descriptor->bitmap = 0;
			for (i = (offset % 0x4000) / 512; i <= ((offset % 0x4000) + write_size - 1) / 512; i++)
			{
				new_block_descriptor->bitmap += ((uint32_t)1) << ((7 - (i % 8)) + (i / 8) * 8);
			}
			/* Zero out the parts of the 16K overlay data that isn't being overlayed
			 */
			for (i = 0; i < 32; i++)
			{
				if ( (new_block_descriptor->bitmap & ( ((uint32_t)1) << i )) == 0 )
				{
					memory_set(data_buffer + (i * 512), 0, 512);
				}
			}
		}
		/* Now we write the data buffer to disk
		 */
		libbfio_handle_seek_offset(file_io_handle, new_block_descriptor->offset, SEEK_SET, error);
		if (libbfio_handle_write_buffer(file_io_handle, data_buffer, 16384, error) != 16384)
		{
			libcnotify_printf("%s: store: %02d error writing new data to 0x%08" PRIx64 " with original offset of 0x%08" PRIx64 "\n",
				function, store_descriptor->index, new_block_descriptor->offset, new_block_descriptor->original_offset);
			goto on_error;
		}
		
		/* And write the new block descriptor to disk
		 */
		memcpy(descriptor_buffer + 0, (void*)&(new_block_descriptor->original_offset), 8);
		memcpy(descriptor_buffer + 8, (void*)&(new_block_descriptor->relative_offset), 8);
		memcpy(descriptor_buffer + 16, (void*)&(new_block_descriptor->offset), 8);
		memcpy(descriptor_buffer + 24, (void*)&(new_block_descriptor->flags), 4);
		memcpy(descriptor_buffer + 28, (void*)&(new_block_descriptor->bitmap), 4);
		libbfio_handle_seek_offset(file_io_handle, new_block_descriptor->descriptor_location, SEEK_SET, error);
		if (libbfio_handle_write_buffer(file_io_handle, descriptor_buffer, 32, error) != 32)
		{
			libcnotify_printf("%s: store: %02d error writing new block descriptor to offset: 0x%08" PRIx64 "\n",
				function, store_descriptor->index, new_block_descriptor->descriptor_location);
			goto on_error;
		} 
		
		/* Add the new block descriptor to the trees and list
		 */
		libvshadow_block_tree_insert(
							 store_descriptor->forward_block_descriptors_tree,
							 store_descriptor->reverse_block_descriptors_tree,
							 new_block_descriptor,
							 store_descriptor->index,
							 error );
		libcdata_list_append_value(
							 store_descriptor->block_descriptors_list,
							 (intptr_t *) new_block_descriptor,
							 error );
							 
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
				"%s: new descriptor written to 0x%08" PRIx64 " new data written to %" PRIx64 "\n",
				function, new_block_descriptor->descriptor_location, new_block_descriptor->offset);
		}
#endif
	}

    /* done, return the size written to disk */
    return write_size;
    
    on_error:
    /*
    #if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
        libcthreads_read_write_lock_release_for_read(store_descriptor->read_write_lock, NULL);
    #endif
    */
    #if defined( HAVE_DEBUG_OUTPUT )
        if( libcnotify_verbose != 0 )
            libcnotify_printf(
             "%s: got a \"goto on_error\", about to exit with -1\n",
             function);
    #endif
    
    return( -1 );
}

/* Retrieves the volume size
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_volume_size(
     libvshadow_store_descriptor_t *store_descriptor,
     size64_t *volume_size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_volume_size";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( volume_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume size.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	*volume_size = store_descriptor->volume_size;

#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );
}

/* Retrieves the identifier
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_identifier(
     libvshadow_store_descriptor_t *store_descriptor,
     uint8_t *guid,
     size_t size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_identifier";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( guid == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.",
		 function );

		return( -1 );
	}
	if( size < 16 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid GUID value too small.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( memory_copy(
	     guid,
	     store_descriptor->identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy identifier.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the creation date and time
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_creation_time(
     libvshadow_store_descriptor_t *store_descriptor,
     uint64_t *filetime,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_creation_time";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( filetime == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filetime.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	*filetime = store_descriptor->creation_time;

#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );
}

/* Retrieves the copy identifier
 * Returns 1 if successful, 0 if not available or -1 on error
 */
int libvshadow_store_descriptor_get_copy_identifier(
     libvshadow_store_descriptor_t *store_descriptor,
     uint8_t *guid,
     size_t size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_copy_identifier";
	int result            = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( guid == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.",
		 function );

		return( -1 );
	}
	if( size < 16 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid GUID value too small.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( store_descriptor->has_in_volume_store_data != 0 )
	{
		result = 1;

		if( memory_copy(
		     guid,
		     store_descriptor->copy_identifier,
		     16 ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy shadow copy identifier.",
			 function );

			result = -1;
		}
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the copy set identifier
 * Returns 1 if successful, 0 if not available or -1 on error
 */
int libvshadow_store_descriptor_get_copy_set_identifier(
     libvshadow_store_descriptor_t *store_descriptor,
     uint8_t *guid,
     size_t size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_copy_set_identifier";
	int result            = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( guid == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.",
		 function );

		return( -1 );
	}
	if( size < 16 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid GUID value too small.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( store_descriptor->has_in_volume_store_data != 0 )
	{
		result = 1;

		if( memory_copy(
		     guid,
		     store_descriptor->copy_set_identifier,
		     16 ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy shadow copy set identifier.",
			 function );

			result = -1;
		}
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the attribute flags
 * Returns 1 if successful, 0 if not available or -1 on error
 */
int libvshadow_store_descriptor_get_attribute_flags(
     libvshadow_store_descriptor_t *store_descriptor,
     uint32_t *attribute_flags,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_attribute_flags";
	int result            = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( attribute_flags == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid attribute flags.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( store_descriptor->has_in_volume_store_data != 0 )
	{
		result = 1;

		*attribute_flags = store_descriptor->attribute_flags;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the number of blocks
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_number_of_blocks(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     int *number_of_blocks,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_number_of_blocks";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( store_descriptor->has_in_volume_store_data == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor - missing in-volume store data.",
		 function );

		return( -1 );
	}
	/* This function will acquire the write lock
	 */
	if( libvshadow_store_descriptor_read_block_descriptors(
	     store_descriptor,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read block descriptors.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libcdata_list_get_number_of_elements(
	     store_descriptor->block_descriptors_list,
	     number_of_blocks,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of block descriptors.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves a specific block descriptor
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_block_descriptor_by_index(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     int block_index,
     libvshadow_block_descriptor_t **block_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_block_descriptor_by_index";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( store_descriptor->has_in_volume_store_data == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor - missing in-volume store data.",
		 function );

		return( -1 );
	}
	/* This function will acquire the write lock
	 */
	if( libvshadow_store_descriptor_read_block_descriptors(
	     store_descriptor,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read block descriptors.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libcdata_list_get_value_by_index(
	     store_descriptor->block_descriptors_list,
	     block_index,
	     (intptr_t **) block_descriptor,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve block descriptor: %d.",
		 function,
		 block_index );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

