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

#ifndef MMPRENDERER_YUVWRITER_HPP__
#define MMPRENDERER_YUVWRITER_HPP__

#include "MmpRenderer.hpp"

class CMmpRenderer_YUVWriter : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    FILE* m_fp;
    static MMP_U32 m_render_file_id;
	
protected:
    CMmpRenderer_YUVWriter(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_YUVWriter();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

protected:
    //virtual MMP_RESULT Render_Ion(CMmpMediaSampleDecodeResult* pDecResult);

public:
    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe);
    virtual MMP_RESULT Render(class mmp_buffer_imageframe* p_buf_imageframe);
    
    
    
};


#endif
