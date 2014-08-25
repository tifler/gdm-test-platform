#ifndef __GDM_BUFFER_H__
#define __GDM_BUFFER_H__

/*****************************************************************************/

#define GDM_MAX_PLANES              (3)

// if this flag is set, fd duplicate via ion_map. If not, via ion_share
#define GDM_BUF_FLAG_MAP            (0x01)

/*****************************************************************************/

struct GDMPlane {
    int fd;
    unsigned int length;
    void *base; // if GDM_BUF_FLAG_MAP is used, base has it's user virt. addr.
    void *user; // generic container for user like GDMBuffer.user
};

struct GDMBuffer {
    int planeCount;
    struct GDMPlane plane[GDM_MAX_PLANES];
    int acquireFenceFd; // for sync fence
    int releaseFenceFd; // for sync fence
    void *priv; // DO NOT TOUCH THIS MEMBER. INTERNALLY USED.
    void *user; // USE USER INSTEAD OF PRIV.
};

/*****************************************************************************/

struct GDMBuffer *allocContigMemory(
        int planes, const unsigned int *planeSizes, unsigned long flags);
void freeContigMemory(struct GDMBuffer *buffer);

#endif  /*__GDM_BUFFER_H__*/
