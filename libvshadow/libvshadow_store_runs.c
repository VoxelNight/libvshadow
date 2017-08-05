/*
 * Library to access the Windows NT Volume Shadow Snapshot (VSS) format
 * Store run related functions
 *
 * This particular file:
 * Copyright (C) 2016-2017, James Clawson <voxelnight@vexatioustendencies.com>
 * 
 * The rest of the project:
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

#include "libvshadow_debug.h"
#include "libvshadow_libcnotify.h"

#include "libvshadow_store_runs.h"


/* Adds a new run to the store's runs list. Initializes all values.
 * This function is not multi-thread safe acquire write lock before call
 * Returns 0 if no errors occurred, or -1 otherwise
 */
int libvshadow_store_run_add_run(
     libvshadow_store_run_t **store_run,
	 off64_t start_offset,
	 size64_t size,
     libcerror_error_t **error )
{
	static char *function                           = "libvshadow_store_run_add_run";
	libvshadow_store_run_t **last_next_run = store_run;
	off64_t run_start = 0;
	off64_t run_stop = 0;
	off64_t run_size = 0;
	off64_t run_bitmap_size = 0;
	
	/* Some basic error checking
	 */
	if( store_run == NULL)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store run.",
		 function );

		return( -1 );
	}
	
	/* Calculate some numbers
	 */
	run_start = start_offset - (start_offset % (128 * 1024)) + (128 * 1024);
	run_stop = (start_offset + size) - ((start_offset + size) % (128 * 1024));
	run_size = run_stop - run_start;
	run_bitmap_size = run_size / (128 * 1024);
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
			"%s: Calculated run values: run_start =  %" PRIu64 " run_stop =  %" PRIu64 " run_size = %" PRIu64 " run_bitmap_size = %" PRIu64 "\n",
			function, run_start, run_stop, run_size, run_bitmap_size);
	}
#endif
	
	/* For simplicity, runs must be at least 128KB
	 */
	if( run_bitmap_size < 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: attempted to add a run with size = 0.",
		 function );

		return( -1 );
	}
	
	/* Find the last pointer in the store_run list
	 */
	while (*last_next_run != NULL)
	{
		/* We check for duplicate run start values and handle as appropriate
		 */
		if ((*last_next_run)->run_start_offset == run_start)
		{
			/* If the size is also the same, we already have this run, just return
			 */
			if ((*last_next_run)->run_bitmap_size == run_bitmap_size)
			{
				return 0;
			}
			else if ((*last_next_run)->run_bitmap_size > run_bitmap_size)
			{
				/* We're adding a new run at the same start offset as an existing run that is smaller than the existing run
				 * This shouldn't happen.
				 */
				libcnotify_printf(
					 "%s: Attempted to re-add an existing run, but existing run was bigger\n",
					 function);
				return -1;
			}
			else
			{
				/* This could be pretty-fied
				 * We're making the run's bitmap bigger
				 */
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
						"%s: Expanding existing run's bitmap. Old size =  %" PRIu64 " New size =  %" PRIu64 "\n",
						function, (*last_next_run)->run_bitmap_size, run_bitmap_size);
				}
#endif
				(*last_next_run)->run_bitmap = realloc((*last_next_run)->run_bitmap, run_bitmap_size);
				memory_set((*last_next_run)->run_bitmap + (*last_next_run)->run_bitmap_size, 0, (run_bitmap_size - (*last_next_run)->run_bitmap_size));
				
				/* This is a sort of hack-y solution to a problem that can come up when expanding a run that didn't end on a multiple of 128K.
				 * We just mark the whole first byte of the additional bitmap space as used to be on the safe side.
				 */
				(*last_next_run)->run_bitmap[(*last_next_run)->run_bitmap_size] = 0xff;
				(*last_next_run)->run_bitmap_size = run_bitmap_size;
				(*last_next_run)->run_free_space = run_size - (*last_next_run)->run_size + (*last_next_run)->run_free_space - (128 * 1024);
				(*last_next_run)->run_size = run_size;
				(*last_next_run)->run_full = 0;
				(*last_next_run)->run_first_free_addr = run_start; /* if the run was already full, we reset it and rescan through the bitmap */
				libvshadow_store_run_update_next_free(last_next_run, error);
				
				return 0;
			}
		}
		last_next_run =  &((*last_next_run)->next_run);
	}
	
	(*last_next_run) = malloc(sizeof(libvshadow_store_run_t));
	if( *last_next_run == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: Failed to allocate memory for new run structure.",
		 function );

		return( -1 );
	}
	
	memory_set(
	     *last_next_run,
	     0,
	     sizeof( libvshadow_store_run_t ) );
	
	/* Calculate the usable space for the run
	 * It has to start and end on a multiple of 128KB
	 * So we'll (probably) lose a little bit of space
	 */
	(*last_next_run)->run_start_offset = run_start;
	(*last_next_run)->run_first_free_addr = run_start;
	
	/* Calculate the bitmap size
	 *  Each byte of the bitmap represents 128KB 
	 */
	(*last_next_run)->run_bitmap_size = run_bitmap_size;
	(*last_next_run)->run_size = run_size;
	(*last_next_run)->run_free_space = run_size;
	
	(*last_next_run)->run_bitmap = malloc(run_bitmap_size);
	if( (*last_next_run)->run_bitmap == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: Failed to allocate memory for run bitmap.",
		 function );

		free(*last_next_run);
		*last_next_run = NULL;

		return( -1 );
	}
	memory_set(
	     (*last_next_run)->run_bitmap,
	     0,
	     (*last_next_run)->run_bitmap_size );
	
	return 0;
}


/* Frees the store's runs.
 * This function is not multi-thread safe acquire write lock before call
 * Returns 0 if no errors occurred, or -1 otherwise
 */
int libvshadow_store_run_free_runs(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error )
{
	static char *function                           = "libvshadow_store_run_free_runs";
	
 	if( store_run == NULL || *store_run == NULL)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store run.",
		 function );

		return( -1 );
	}
	
	if ( (*store_run)->next_run != NULL )
		libvshadow_store_run_free_runs( &((*store_run)->next_run), error );
	
	if ( (*store_run)->run_bitmap != NULL)
		free ((*store_run)->run_bitmap);
	
	free(*store_run);
	*store_run = NULL;
	
	return 0;
}


/* Finds and returns the address of the next 16k block available for writing and marks it as used
 * This function is not multi-thread safe acquire write lock before call
 * Returns the next available address (as an offset from the start of the volume) if no errors occurred, or 0 otherwise
 */ 
off64_t libvshadow_store_run_get_next_free(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error )
{
	off64_t next_free = 0;
	static char *function                           = "libvshadow_store_run_get_next_free";
	
 	if( store_run == NULL || *store_run == NULL)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store run.",
		 function );

		return( 0 );
	}
	
	/* If this run is full, check the next one or fail
	 */
	if ( (*store_run)->run_full)
	{
		if ((*store_run)->next_run != NULL )
			return libvshadow_store_run_get_next_free( &((*store_run)->next_run), error );
		else
			return 0;
	}
	
	next_free = (*store_run)->run_first_free_addr;
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
			 "%s: Was asked for next free block. Returing:  0x%08" PRIx64 "\n", 
			 function, next_free);
	}
#endif
	
	/* Mark this part of the bitmap as used and update the next free addr for this run
	 */
	libvshadow_store_run_mark_as_used(store_run, next_free, error);
	
	return next_free;
}


/* Updates the next_free value in the given run, updates run_full if needed
 * This function is not multi-thread safe acquire write lock before call
 * Returns 0 for no error, -1 for error
 */
int libvshadow_store_run_update_next_free(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error )
{
	static char *function                           = "libvshadow_store_run_update_next_free";
	uint64_t bitmap_byte_offset;
	uint64_t i, n;
	off64_t new_next_first = 0;
	
	/* Return 0 for error, I know...
	 */
	if( store_run == NULL || *store_run == NULL)
	{
		return -1;
	}
	
	/* This will be the starting bitmap byte offset
	 * We'll just look at blocks after this
	 */
	bitmap_byte_offset = ((*store_run)->run_first_free_addr - (*store_run)->run_start_offset) / (128 * 1024);
	
	/* Check each byte, if it's not full, then check each bit in it
	 */
	for (i = bitmap_byte_offset; i < (*store_run)->run_bitmap_size; i++)
	{
		if ( (*store_run)->run_bitmap[i] != 255 )
		{
			for (n = 0; n < 8; n++)
			{
				if ( ((*store_run)->run_bitmap[i] & ((uint8_t)128 >> n)) == 0)
				{
					new_next_first = (*store_run)->run_start_offset;
					new_next_first += i * (128 * 1024);
					new_next_first += n * (16 * 1024);
					break;
				}
			}
			break;
		}
	}

	/* If new_next_first is still 0, then this run is full, or at least "full"
	 */
	if (new_next_first == 0)
	{
		(*store_run)->run_free_space = 0;
		(*store_run)->run_full = 1;
	}

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
			 "%s: updating next_free_addr to:  0x%08" PRIx64 "\n", 
			 function, new_next_first);
	}
#endif

	(*store_run)->run_first_free_addr = new_next_first;
	
	return 0;
}


/* Finds the total size of all runs in the store
 * This function is not multi-thread safe acquire write lock before call
 * Returns the total size, returns 0 if their are no runs, or an error occurred
 */
size64_t libvshadow_store_run_get_total_size(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error )
{
	static char *function                           = "libvshadow_store_run_get_total_size";
	
	/* Return 0 for error, I know...
	 */
	if( store_run == NULL || *store_run == NULL)
	{
		return 0;
	}
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
			"%s: This run's total space is: %" PRIu64 "\n", 
			function, (*store_run)->run_size);
	}
#endif
	
	/* If this is the last run
	 */
	if ((*store_run)->next_run == NULL)
		return (*store_run)->run_size;
	
	/* If there is another run
	 */
	return (*store_run)->run_size + libvshadow_store_run_get_total_size( &((*store_run)->next_run), error );
}


/* Returns the amount of free space available in the runs
 * This function is not multi-thread safe acquire write lock before call
 * Returns the amount of free space in the runs, returns 0 on error
 */
size64_t libvshadow_store_run_get_free_size(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error )
{
	static char *function                           = "libvshadow_store_run_get_free_size";
	
	/* Return 0 for error, I know...
	 */
	if( store_run == NULL || *store_run == NULL)
	{
		return 0;
	}
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
			"%s: This run's free space is: %" PRIu64 "\n", 
			function, (*store_run)->run_free_space);
	}
#endif
	
	/* If this is the last run
	 */
	if ((*store_run)->next_run == NULL)
	{
		return (*store_run)->run_free_space;
	}
	
	/* If there is another run
	 */
	return (*store_run)->run_free_space + libvshadow_store_run_get_free_size( &((*store_run)->next_run), error );
}


/* Marks the specified part of the disk as used in the runs' bitmaps
 * This function is not multi-thread safe acquire write lock before call
 * Returns 0 if no errors occurred, or -1 otherwise
 */
int libvshadow_store_run_mark_as_used(
     libvshadow_store_run_t **store_run,
	 off64_t used_offset,
     libcerror_error_t **error )
{
	static char *function                           = "libvshadow_store_run_mark_as_used";
	uint64_t bitmap_byte_offset;
	uint8_t bitmap_bit_offset;
	libvshadow_store_run_t *current_run;

 	if( store_run == NULL || *store_run == NULL)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store run.",
		 function );

		return( -1 );
	}
	
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
			 "%s: Was asked to mark 0x%08" PRIx64 " as used\n", 
			 function, used_offset);
	}
#endif
	
	/* Find out which run the offset is in
	 */
	current_run = *store_run;
	while (current_run != NULL)
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
				 "%s: Checking if >= 0x%08" PRIx64 " and < 0x%08" PRIx64 "\n", 
				 function, current_run->run_start_offset, (current_run->run_start_offset + current_run->run_size));
		}
#endif
		/* Check if the offset is in the range of the current run
		 */
		if ( used_offset >= current_run->run_start_offset && used_offset < (current_run->run_start_offset + current_run->run_size) )
			break;
		current_run = current_run->next_run;
	}
	
	/* If we couldn't find an appropriate run
	 */
	if (current_run == NULL)
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
				 "%s: Couldn't find appropriate run, bailing out\n", 
				 function);
		}
#endif
		return -1;
	}

	/* Find out which 16K bitmap bit/byte thing the address is in (16KB per bit)
	 */
	bitmap_byte_offset = (used_offset - current_run->run_start_offset) / (128 * 1024);
	bitmap_bit_offset = ((used_offset - current_run->run_start_offset) % (128 * 1024)) / (16 * 1024);
	
	/* Update the appropriate bit in the bitmap if we need to
	 */
	if (current_run->run_bitmap[bitmap_byte_offset]  != (current_run->run_bitmap[bitmap_byte_offset] | ((uint8_t)128 >> bitmap_bit_offset)))
	{
		current_run->run_bitmap[bitmap_byte_offset] = current_run->run_bitmap[bitmap_byte_offset] | ((uint8_t)128 >> bitmap_bit_offset);
		
		current_run->run_free_space -= (16 * 1024);
		
		if (current_run->run_free_space == 0)
		{
			current_run->run_full = 1;
			current_run->run_first_free_addr = 0;
		}
		
		/* Update the next free address if we just marked it as used
		 * We're finding the byte and bit offsets of the first_free_addr and comparing them to the bitmap offsets for the addr we just marked as used
		 */
		if ( current_run->run_full == 0
			&& bitmap_byte_offset == (current_run->run_first_free_addr - current_run->run_start_offset) / (128 * 1024)
			&& bitmap_bit_offset == ((current_run->run_first_free_addr - current_run->run_start_offset) % (128 * 1024)) / (16 * 1024) )
		{
			libvshadow_store_run_update_next_free(&current_run, error);
		}
	}

	return 0;
}
