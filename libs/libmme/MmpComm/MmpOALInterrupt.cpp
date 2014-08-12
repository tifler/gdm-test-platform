/****************************************************************************
 * TokiPlayer Corp.
 * Copyright (C) 2006
 *
 * All rights reserved. This file contains information that is proprietary to
 * TokiPlayer Corp. and may not be distributed or copied without
 * written consent from Mtekvison Corp.
 *
 *   $Id: Class≈€«√∏¥.cpp,v 1.1.2.1 2007/06/07 23:55:13 hwanght Exp $
 *
 */


#include"../MmpGlobal/MmpDefine.h"


#include"MMPOALInterrupt.hpp"

CMmpOALInterrupt::CMmpOALInterrupt()
{
    int i;

    for(i=0;i<MMPOAL_INT_IRQ_MAX;i++)
    {
        m_bUsedIrq[i]=FALSE;
    }
}

CMmpOALInterrupt::~CMmpOALInterrupt()
{

}

