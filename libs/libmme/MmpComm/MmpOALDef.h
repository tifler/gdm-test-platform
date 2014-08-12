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

#ifndef	__MMPOALDEF_H
#define	__MMPOALDEF_H

#include "../MmpGlobal/MmpDefine.h"


//Handle Define
typedef void* MMPOALTASK_HANDLE;
typedef void* MMPOALQUEUE_HANDLE;
typedef void* MMPOALSEMAPHORE_HANDLE;
typedef void* MMPOALISR_HANDLE;
typedef void* MMPOALEVENT_HANDLE;
typedef void* MMPOALCS_HANDLE;

//Task Start Mode
#define MMPOAL_TASK_STARTED_ON_CREATE     1
#define MMPOAL_TASK_PAUSED_ON_CREATE      0

//Semaphore Mode
#define MMPOAL_SEMAPHORE_COUNTING  0
#define MMPOAL_SEMAPHORE_BINARY    1

//Queue Mode
enum MMPOAL_QUEUE_MODE
{
    MMPOAL_QUEUE_SUSPEND=0,
    MMPOAL_QUEUE_NO_SUSPEND
};

//File system Mode
typedef void* MMPOAL_FILE_HANDLE;
#define MMPOAL_FS_MAXFILENAME_SIZE 256

// file open mode flag definition
#define MMPOAL_FS_ACCESS_UNKNOWN (0 << 0)
#define MMPOAL_FS_ACCESS_APPEND (1 << 0)
#define MMPOAL_FS_ACCESS_BINARY (1 << 1)
#define MMPOAL_FS_ACCESS_CREATE (1 << 2)
#define MMPOAL_FS_ACCESS_READ (1 << 3)
#define MMPOAL_FS_ACCESS_WRITE (1 << 4)

/* Combined access modes which are frequently used */
#define MMPOAL_FS_ACCESS_BR     (MMPOAL_FS_ACCESS_BINARY | MMPOAL_FS_ACCESS_READ)
#define MMPOAL_FS_ACCESS_CW     (MMPOAL_FS_ACCESS_CREATE | MMPOAL_FS_ACCESS_WRITE)
#define MMPOAL_FS_ACCESS_RW     (MMPOAL_FS_ACCESS_READ | MMPOAL_FS_ACCESS_WRITE)
#define MMPOAL_FS_ACCESS_RWC    (MMPOAL_FS_ACCESS_READ | MMPOAL_FS_ACCESS_WRITE | MMPOAL_FS_ACCESS_CREATE)
#define MMPOAL_FS_ACCESS_RWA    (MMPOAL_FS_ACCESS_READ | MMPOAL_FS_ACCESS_WRITE | MMPOAL_FS_ACCESS_APPEND)
#define MMPOAL_FS_ACCESS_ACW    (MMPOAL_FS_ACCESS_CREATE | MMPOAL_FS_ACCESS_CREATE | MMPOAL_FS_ACCESS_WRITE)
#define MMPOAL_FS_ACCESS_BCW    (MMPOAL_FS_ACCESS_BINARY | MMPOAL_FS_ACCESS_CW)
#define MMPOAL_FS_ACCESS_BRW    (MMPOAL_FS_ACCESS_WRITE | MMPOAL_FS_ACCESS_BR)
#define MMPOAL_FS_ACCESS_ABCW   (MMPOAL_FS_ACCESS_BINARY | MMPOAL_FS_ACCESS_ACW)
   
#define MMPOAL_FS_START_MARK             0          ///< File start marker
#define MMPOAL_FS_END_MARK                1          ///< File end marker
#define MMPOAL_FS_CURRENT_MARK         2          ///< File current position marker


#endif // __MEDIAPLAYER_HPP

