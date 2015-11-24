#ifndef __FILE_IO_H__
#define __FILE_IO_H__

#include <sys/types.h>

/*****************************************************************************/

int safeWrite(int fd, void *buffer, size_t size);

#endif  /*__FILE_IO_H__*/
