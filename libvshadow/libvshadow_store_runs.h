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

#if !defined( _LIBVSHADOW_STORE_RUN_H )
#define _LIBVSHADOW_STORE_RUN_H

#include <common.h>
#include <types.h>

#include "libvshadow_libcerror.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libvshadow_store_run libvshadow_store_run_t;

struct libvshadow_store_run
{
	/* Offset (in bytes) of the start of the run from the start of the volume
	 */
	off64_t			run_start_offset;
	
	/* Size of the run (in bytes)
	 */
	size64_t		run_size;
	
	/* Available space
	 */
	size64_t		run_free_space;
	
	/* The bitmap for this run
	 */
	uint8_t			*run_bitmap;
	
	/* The size of the bitmap (in bytes)
	 */
	size64_t		run_bitmap_size;
	
	/* Whether this run is full or not (0 = full, 1 = not full)
	 */
	int				run_full;
	
	/* The first free 16K block available for writing (as an offset from the start of the volume)
	 */
	off64_t			run_first_free_addr;
	
	/* Pointer to the next run struct
	 */
	libvshadow_store_run_t *next_run;
};


int libvshadow_store_run_add_run(
     libvshadow_store_run_t **store_run,
	 off64_t start_offset,
	 size64_t size,
     libcerror_error_t **error );

int libvshadow_store_run_free_runs(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error );
	 
off64_t libvshadow_store_run_get_next_free(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error );

int libvshadow_store_run_update_next_free(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error );

size64_t libvshadow_store_run_get_total_size(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error );

size64_t libvshadow_store_run_get_free_size(
     libvshadow_store_run_t **store_run,
     libcerror_error_t **error );

int libvshadow_store_run_mark_as_used(
     libvshadow_store_run_t **store_run,
	 off64_t used_offset,
     libcerror_error_t **error );

#if defined( __cplusplus )
}
#endif

#endif