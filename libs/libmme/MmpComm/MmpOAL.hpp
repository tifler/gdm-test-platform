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

#ifndef	__MmpOAL_HPP
#define	__MmpOAL_HPP

#include "../MmpGlobal/MmpDefine.h"


#include "MmpObject.hpp"
//#include "MmpOALFileSystem.hpp"
#include "MmpOALTask.hpp"
#include "MmpOALSemaphore.hpp"
//#include "MmpOALQueue.hpp"
//#include "MmpOALHeap.hpp"
//#include "MmpOALEvent.hpp"
#include "MmpOALCriticalSection.hpp"
//#include "MmpOALInterrupt.hpp"

class CMmpOAL : public CMmpObject
{
private:
    static CMmpOAL* m_pMmpOAL;  //Singleton Pattern Instance
    static unsigned int m_nRefCount;

    //CMmpOALFileSystem* m_pMmpOALFileSystem;
    CMmpOALTask* m_pMmpOALTask;
    CMmpOALSemaphore* m_pMmpOALSemaphore;
    CMmpOALCriticalSection* m_pMmpOALCs;
    //CMmpOALQueue* m_pMmpOALQueue;
    //CMmpOALHeap* m_pMmpOALHeap;
    //CMmpOALEvent* m_pMmpOALEvent;
    //CMmpOALInterrupt* m_pMmpOALInterrupt;


private:   
    CMmpOAL();
    virtual ~CMmpOAL();

    MMP_RESULT Open();
    MMP_RESULT Close();

public:
    //Singleton Pattern 
    static MMP_RESULT __MmpApiCall CreateInstance();
    static MMP_RESULT __MmpApiCall DestroyInstance();
    static inline CMmpOAL* GetInstance() { return CMmpOAL::m_pMmpOAL; }
    static inline CMmpOALTask* GetTaskInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALTask;}
    static inline CMmpOALSemaphore* GetSemaphoreInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALSemaphore;}
    static inline CMmpOALCriticalSection* GetCsInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALCs;}
    //static inline CMmpOALFileSystem* GetFileSystemInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALFileSystem;}
    //static inline CMmpOALQueue* GetQueueInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALQueue;}
    //static inline CMmpOALHeap* GetHeapInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALHeap;}
    //static inline CMmpOALEvent* GetEventInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALEvent;}
    //static inline CMmpOALInterrupt* GetInterruptInstance() {return CMmpOAL::m_pMmpOAL->m_pMmpOALInterrupt;}

};

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )

#define CMmpOAL_Sleep(x) Sleep(x)
#define CMmpOAL_GetTickCount GetTickCount

#define CMmpOAL_StrCopy(strDest, strSrc) _tcscpy(strDest, strSrc)
#define CMmpOAL_MemSet(dst, val, sz) memset(dst, val, sz)
#define CMmpOAL_MemCopy(dst, src, sz) memcpy(dst, src, sz)

#elif (MMP_OS==MMP_OS_LINUX)

#define CMmpOAL_Sleep(x) usleep(x*1000)
//#define CMmpOAL_GetTickCount GetTickCount

#define CMmpOAL_StrCopy(strDest, strSrc) strcpy(strDest, strSrc)
#define CMmpOAL_MemSet(dst, val, sz) memset(dst, val, sz)
#define CMmpOAL_MemCopy(dst, src, sz) memcpy(dst, src, sz)

#else
#error "ERROR: Select OS in MmpOAL.hpp"
#endif

#endif // __MEDIAPLAYER_HPP

