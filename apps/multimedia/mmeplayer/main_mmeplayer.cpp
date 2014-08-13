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


#include "main_mmeplayer.h"
#include "mme_shell.h"


static inline int MULH(int a, int b)
{
    int r;
    __asm__ ("smmul %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
    return r;
}

void mmeplayer_sig_int(int sig);

#if (MMP_OS == MMP_OS_LINUX)
int main(int argc, char* argv[]) {
#else
int main_mmeplayer(int argc, char* argv[]) {
#endif

#if (MMP_OS == MMP_OS_LINUX)
    signal(SIGINT, mmeplayer_sig_int);	/* register the signal */
#endif
	
    CMmpOAL::CreateInstance();

	mme_shell_main(argc, argv);

    CMmpOAL::DestroyInstance();

    return 0;
}



void mmeplayer_sig_int(int sig)
{
    printf("[mmeplayer] mmeplayer received the KILL signal. Bye!! \n\r");

    mme_shell_deinit();

    CMmpOAL::DestroyInstance();
    exit(0);
}

