#include "linux_driver.h"

int copy_to_user(void* dest, void* src, int size) {
    memcpy(dest, src, size);
    return 0;
}

int copy_from_user(void* dest, void* src, int size) {
    memcpy(dest, src, size);
    return 0;
}

void* dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flag) {
    void* ptr;

    ptr = malloc(size);

    return ptr;
}