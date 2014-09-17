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

#include "mme_c_api.h"
#include "mmp_env_mgr.hpp"
#include "MmpUtil.hpp"

int mme_util_get_vpu_fd(void) {
    return mmp_env_mgr::get_instance()->get_vpu_fd();
}

unsigned char* mme_util_get_vpu_instance_pool_buffer(void) {
    return mmp_env_mgr::get_instance()->get_vpu_instance_pool_buffer();
}

unsigned int mme_util_get_vpu_reg_vir_addr(void) {
    return mmp_env_mgr::get_instance()->get_vpu_reg_vir_addr();
}

void* mme_util_get_vpu_common_buffer(void) {
    return mmp_env_mgr::get_instance()->get_vpu_common_buffer();
}

void mme_util_sleep(int milesec) {
    CMmpUtil::Sleep(milesec);
}

unsigned int mme_util_get_tick_count() {
    return CMmpUtil::GetTickCount();
}

long long mme_util_get_tick_count_us() {
    return CMmpUtil::GetTickCountUS();
}

/* e.g  IN:/mnt/abcde.dat OUT: abcde */
void mme_util_split_file_name(char* file_path_name, char* filename) {
    CMmpUtil::SplitFileName(file_path_name, filename);
}