/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 *   Copyright 2011 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __SYS_CORE_ION_H
#define __SYS_CORE_ION_H

#include <ion.h>

enum ion_gdm_heap_type {
/*
    ION_HEAP_TYPE_SYSTEM,
    ION_HEAP_TYPE_SYSTEM_CONTIG,
    ION_HEAP_TYPE_CARVEOUT,
    ION_HEAP_TYPE_CUSTOM,
*/
    ION_DISPLAY_CARVEOUT = ION_HEAP_TYPE_CUSTOM + 1, 	/* 6 */
    ION_VIDEO_CARVEOUT = ION_HEAP_TYPE_CUSTOM + 2, 		/* 7 */
    ION_TEST_DMA = ION_HEAP_TYPE_CUSTOM + 3, 			/* 8 */
};

#define ION_DISPLAY_CARVEOUT_MASK (1<<ION_DISPLAY_CARVEOUT)
#define ION_VIDEO_CARVEOUT_MASK (1<<ION_VIDEO_CARVEOUT)
#define ION_TEST_DMA_MASK (1<<ION_TEST_DMA)


#ifdef __cplusplus
extern "C"
{
#endif

int ion_open();
int ion_close(int fd);
int ion_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
	      unsigned int flags, struct ion_handle **handle);
int ion_alloc_fd(int fd, size_t len, size_t align, unsigned int heap_mask,
		 unsigned int flags, int *handle_fd);
int ion_sync_fd(int fd, int handle_fd);
int ion_free(int fd, struct ion_handle *handle);
int ion_map(int fd, struct ion_handle *handle, size_t length, int prot,
            int flags, off_t offset, unsigned char **ptr, int *map_fd);
int ion_share(int fd, struct ion_handle *handle, int *share_fd);
int ion_import(int fd, int share_fd, struct ion_handle **handle);


#ifdef __cplusplus
}
#endif

#endif /* __SYS_CORE_ION_H */
