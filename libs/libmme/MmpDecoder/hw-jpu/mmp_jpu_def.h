/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MMP_JPU_DEF_H__
#define MMP_JPU_DEF_H__

#include "MmpDefine.h"

#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "jpuapi.h"
#include "jpuapifunc.h"
#include "jpuhelper.h"

#define MMP_JPU_CNM     1 
#define MMP_JPU_ANAPASS 2 
#define MMP_JPU MMP_JPU_ANAPASS
#define MMP_JPU_DEVICE_NAME "/dev/jpu"

#define MMP_JPU_BIT_REG_SIZE		0x300
#define MMP_JPU_BIT_REG_BASE		(0x10000000 + 0x3000)
#define MMP_JDI_DRAM_PHYSICAL_BASE	0x00
#define MMP_JDI_DRAM_PHYSICAL_SIZE	(128*1024*1024)

#endif
