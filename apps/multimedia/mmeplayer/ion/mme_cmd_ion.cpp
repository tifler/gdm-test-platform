#include "mme_shell.h"
#include "ion_api.h"
#include "vpu.h"
#include <sys/mman.h>
#include <sys/ioctl.h>		/* fopen/fread */
#include <fcntl.h>		/* fcntl */

#define VPU_DEVICE_NAME "/dev/vpu"
#define ION_BUFFER_SIZE (1024*1024)

int mme_command_ion_test1(int argc, char* argv[]) {

    int fd;
    int shared_fd;
    int ret;
    unsigned char* pbuf;

    fd = ion_open();
    if(fd >= 0) {

        ret = ion_alloc_fd(fd, ION_BUFFER_SIZE, 0, ION_HEAP_CARVEOUT_MASK, 0, &shared_fd);
        ion_close(fd);

        if(shared_fd > 0) {

            pbuf = (unsigned char*)MMP_DRIVER_MMAP(NULL, ION_BUFFER_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, shared_fd, 0);
            if(pbuf != NULL) {
                memset(pbuf, 0xAA, ION_BUFFER_SIZE);
            }

            ion_close(shared_fd);
        }
    }

    return 0;
}


int mme_command_ion_alloc_fd(int argc, char* argv[]) {

    int fd, vpu_fd;
    int shared_fd;
    int ret;
    unsigned int* pbuf_vir;
    unsigned int phy_addr = 0;
    FILE* fp;

    fd = ion_open();
    if(fd >= 0) {

        ret = ion_alloc_fd(fd, ION_BUFFER_SIZE, 0, ION_HEAP_CARVEOUT_MASK, 0, &shared_fd);
        ion_close(fd);

        pbuf_vir = (unsigned int*)MMP_DRIVER_MMAP(NULL, ION_BUFFER_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, shared_fd, 0);
        if(pbuf_vir != NULL) {
            pbuf_vir[0] = 0xAAAA9829;
            pbuf_vir[1] = 0xBBBB9829;

            MMP_DRIVER_MUNMAP(pbuf_vir, ION_BUFFER_SIZE);
        }


        vpu_fd = MMP_DRIVER_OPEN(VPU_DEVICE_NAME, O_RDWR);
        if(vpu_fd >= 0) {

            unsigned int int_array[3];

            int_array[0] = (unsigned int)shared_fd;
            ret = MMP_DRIVER_IOCTL(vpu_fd, VDI_IOCTL_GET_ION_PHY_ADDR_WITH_SHARED_FD, int_array);
            if(ret >= 0) {
                phy_addr = int_array[1];
            }
            
            MMP_DRIVER_CLOSE(vpu_fd);
        }
        
        MMESHELL_PRINT(MMESHELL_ERROR, ("ret=%d shared_fd = %d  phy_addr=0x%08x \n", ret, shared_fd, phy_addr));

        fp = fopen("/tmp/phy_addr.ion", "wb");
        if(fp!=NULL) {
            fwrite(&phy_addr, sizeof(unsigned int), 1, fp);
            fclose(fp);
        }
    }

    return 0;
}

int mme_command_ion_free_fd(int argc, char* argv[]) {

    int shared_fd;
    int ret;
    
    if(argc != 2) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR: type shared_fd number \n"));
        return -1;
    }

    shared_fd = atoi(argv[1]);
    ret = ion_close(shared_fd);
    
    MMESHELL_PRINT(MMESHELL_ERROR, ("ret=%d shared_fd = %d \n", ret, shared_fd));

    return 0;
}

int mme_command_ion_import(int argc, char* argv[]) {

    int fd;
    int shared_fd;
    int ret;
    struct ion_handle *ion_handle;

    if(argc != 2) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR: type shared_fd number \n"));
        return -1;
    }

    shared_fd = atoi(argv[1]);

    MMESHELL_PRINT(MMESHELL_ERROR, ("shared_fd = %d \n", shared_fd));

    fd = ion_open();
    if(fd >= 0) {

        ret = ion_import(fd, shared_fd, &ion_handle);

        MMESHELL_PRINT(MMESHELL_ERROR, ("ret=%d ion_handle=0x%08x \n", ret, ion_handle));
        if(ret >= 0) {
#if (MMP_OS == MMP_OS_WIN32)
            ion_free_USER_MODE(fd, ion_handle);
#else
            ion_free(fd, ion_handle);
#endif
        }
        
        ion_close(fd);
    }

    return 0;
}

int mme_command_ion_phy_to_vir(int argc, char* argv[]) {

    FILE* fp;
    unsigned int phy_addr;
    unsigned int *pbuf_vir;
    int vpu_fd;

    fp = fopen("/tmp/phy_addr.ion", "rb");
    if(fp!=NULL) {
        fread(&phy_addr, sizeof(unsigned int), 1, fp);
        fclose(fp);
    }
    MMESHELL_PRINT(MMESHELL_ERROR, ("phy_addr=0x%08x \n", phy_addr));


    vpu_fd = MMP_DRIVER_OPEN(VPU_DEVICE_NAME, O_RDWR);
    if(vpu_fd >= 0) {

        pbuf_vir = (unsigned int*)MMP_DRIVER_MMAP(NULL, ION_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vpu_fd, phy_addr);
        
        MMESHELL_PRINT(MMESHELL_ERROR, ("vir_addr[0] = 0x%08x \n", pbuf_vir[0]));
        MMESHELL_PRINT(MMESHELL_ERROR, ("vir_addr[1] = 0x%08x \n", pbuf_vir[1]));

        MMP_DRIVER_MUNMAP(pbuf_vir, ION_BUFFER_SIZE);
        
        MMP_DRIVER_CLOSE(vpu_fd);
    }
    

    return 0;
}
