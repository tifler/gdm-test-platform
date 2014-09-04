#include "mme_shell.h"
#include "TemplateList.hpp"
#include "MmpUtil.hpp"
#include <fcntl.h>
#include <unistd.h>

static CMmpPlayer* s_pMmpPlayer = NULL;

struct malloc_obj {
    void* ptr;
    int size;
};

class system_test {

public:
    TList<struct malloc_obj> m_list_malloc;

    system_test() {}

};

static class system_test* s_p_system_test = NULL;

int mme_command_system_init(int argc, char* argv[]) {

    //s_p_system_test = new class system_test;

    return 0;
}

int mme_command_system_deinit(int argc, char* argv[]) {

    bool flag;
    struct malloc_obj mallocobj;
    class system_test* p_system_test = s_p_system_test;

    if(s_p_system_test != NULL) {

        flag = p_system_test->m_list_malloc.GetFirst(mallocobj);
        while(flag) {
            if(mallocobj.ptr!=NULL) {
                free(mallocobj.ptr);
            }
            flag = p_system_test->m_list_malloc.GetNext(mallocobj);
        }
       // delete s_p_system_test;
        s_p_system_test = NULL;
    }

    return 0;
}

int mme_command_system_reset(int argc, char* argv[]) {

    MMESHELL_PRINT(MMESHELL_ERROR, ("[mme_command_system_reset] +++ \n"));

    mme_command_system_deinit(0, NULL);
    mme_command_system_init(0, NULL);
    return 0;
}

int mme_command_system_memalloc(int argc, char* argv[]) {

    int alloc_unit = 1;
    int alloc_size = 0;
    int len;
    char szptr[64];
    void* ptr;
    struct malloc_obj mallocobj;
    class system_test* p_system_test = s_p_system_test;

    MMESHELL_PRINT(MMESHELL_ERROR, ("[mme_command_system_memalloc] +++ \n"));
    if(argc != 2) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("FAIL: syst_malloc 1M/10M/100M.. \n"));
        return -1;
    }

    strcpy(szptr, argv[1]);
    len = strlen(szptr);
    if( (szptr[len-1] == 'M') ||  (szptr[len-1] == 'm') ) {
        alloc_unit = 1024*1024;
    }
    else if( (szptr[len-1] == 'K') ||  (szptr[len-1] == 'k') ) {
        alloc_unit = 1024;
    }
    else if( (szptr[len-1] == 'B') ||  (szptr[len-1] == 'b') ) {
        alloc_unit = 1;
    }
    else {
        MMESHELL_PRINT(MMESHELL_ERROR, ("FAIL: syst_malloc 1M/10M/100M.. \n"));
        return -1;
    }

    szptr[len-1] = '\0';

    alloc_size = atoi(szptr);
    alloc_size *= alloc_unit;

    ptr = malloc(alloc_size);

    if(ptr != NULL) {

        memset(ptr, 0x00, alloc_size);

        mallocobj.ptr = ptr;
        mallocobj.size = alloc_size;
        p_system_test->m_list_malloc.Add(mallocobj);
    }

    MMESHELL_PRINT(MMESHELL_INFO, (" malloc ptr=0x%08x sz=(%d %dKByte %dMByte) \n", (unsigned int)ptr, alloc_size, alloc_size/1024, alloc_size/(1024*1024) ));


    return 0;
}

int mme_command_system_hardwork(int argc, char* argv[]) {

    unsigned int dur_second, dur_tick, cur_tick, before_tick, start_tick;
    bool flag;
    struct malloc_obj mallocobj;
    class system_test* p_system_test = s_p_system_test;
    static char p = 0;
    p++;


    MMESHELL_PRINT(MMESHELL_ERROR, ("[mme_command_system_hardwork] +++ \n"));

    if(argc != 2) {
        
        MMESHELL_PRINT(MMESHELL_ERROR, ("FAIL: syst_hardwork [dur second] \n"));
        return -1;
    }

    dur_second = atoi(argv[1]);
    dur_tick = dur_second*1000;

    start_tick = CMmpUtil::GetTickCount();
    before_tick = start_tick;

    MMESHELL_PRINT(MMESHELL_INFO, (" Work Start.. dur_sec=%d  \n", dur_second ));

    while(1) {

        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick - start_tick) > dur_tick ) {
            break;
        }

        if( (cur_tick - before_tick) > 1000 ) {
            MMESHELL_PRINT(MMESHELL_INFO, (" Work %d/%d  \n", (cur_tick-start_tick)/1000, dur_second ));
            before_tick = cur_tick;
        }

        if(s_p_system_test != NULL) {

            flag = p_system_test->m_list_malloc.GetFirst(mallocobj);
            while(flag) {
                if(mallocobj.ptr!=NULL) {
                    memset(mallocobj.ptr, p, mallocobj.size);
                    MMESHELL_PRINT(MMESHELL_INFO, (" ptr=0x%x sz=%d  ", (unsigned int)mallocobj.ptr, mallocobj.size));
                }
                flag = p_system_test->m_list_malloc.GetNext(mallocobj);
            }
        }

    }

    MMESHELL_PRINT(MMESHELL_INFO, (" Work Finished!!  \n" ));

    return 0;
}

#if 1
int mme_command_system_meminfo(int argc, char* argv[]) {

    struct mmp_system_meminfo meminfo;
    
    CMmpUtil::system_meminfo(&meminfo);

    MMESHELL_PRINT(MMESHELL_INFO, (" MemTotal:     %d KB\n\r", (int)meminfo.MemTotal));
    MMESHELL_PRINT(MMESHELL_INFO, (" MemFree:      %d KB\n\r", (int)meminfo.MemFree));
    MMESHELL_PRINT(MMESHELL_INFO, (" Buffers:      %d KB\n\r", (int)meminfo.Buffers));
    MMESHELL_PRINT(MMESHELL_INFO, (" Cached:       %d KB\n\r", (int)meminfo.Cached));
    MMESHELL_PRINT(MMESHELL_INFO, (" SwapCached:   %d KB\n\r", (int)meminfo.SwapCached));
    MMESHELL_PRINT(MMESHELL_INFO, (" Active:       %d KB\n\r", (int)meminfo.Active));
    MMESHELL_PRINT(MMESHELL_INFO, (" Inactive:     %d KB\n\r", (int)meminfo.Inactive));
    MMESHELL_PRINT(MMESHELL_INFO, (" Active(anon): %d KB\n\r", (int)meminfo.Active_anon));
    MMESHELL_PRINT(MMESHELL_INFO, (" Inactive(anon): %d KB\n\r", (int)meminfo.Inactive_anon));
    MMESHELL_PRINT(MMESHELL_INFO, (" Active(file):   %d KB\n\r", (int)meminfo.Active_file));
    MMESHELL_PRINT(MMESHELL_INFO, (" Inactive(file): %d KB\n\r", (int)meminfo.Inactive_file));
    MMESHELL_PRINT(MMESHELL_INFO, (" Unevictable:    %d KB\n\r", (int)meminfo.Unevictable));
    MMESHELL_PRINT(MMESHELL_INFO, (" Mlocked:        %d KB\n\r", (int)meminfo.Mlocked));
    MMESHELL_PRINT(MMESHELL_INFO, (" HighTotal:      %d KB\n\r", (int)meminfo.HighTotal));
    MMESHELL_PRINT(MMESHELL_INFO, (" HighFree:       %d KB\n\r", (int)meminfo.HighFree));
    MMESHELL_PRINT(MMESHELL_INFO, (" LowTotal:       %d KB\n\r", (int)meminfo.LowTotal));
    MMESHELL_PRINT(MMESHELL_INFO, (" LowFree:        %d KB\n\r", (int)meminfo.LowFree));
    MMESHELL_PRINT(MMESHELL_INFO, (" SwapTotal:      %d KB\n\r", (int)meminfo.SwapTotal));
    MMESHELL_PRINT(MMESHELL_INFO, (" SwapFree:       %d KB\n\r", (int)meminfo.SwapFree));
    MMESHELL_PRINT(MMESHELL_INFO, (" Dirty:          %d KB\n\r", (int)meminfo.Dirty));
    MMESHELL_PRINT(MMESHELL_INFO, (" Writeback:      %d KB\n\r", (int)meminfo.Writeback));
    MMESHELL_PRINT(MMESHELL_INFO, (" AnonPages:      %d KB\n\r", (int)meminfo.AnonPages));
    MMESHELL_PRINT(MMESHELL_INFO, (" Mapped:         %d KB\n\r", (int)meminfo.Mapped));
    MMESHELL_PRINT(MMESHELL_INFO, (" Shmem:          %d KB\n\r", (int)meminfo.Shmem));
    MMESHELL_PRINT(MMESHELL_INFO, (" Slab:           %d KB\n\r", (int)meminfo.Slab));
    MMESHELL_PRINT(MMESHELL_INFO, (" SReclaimable:   %d KB\n\r", (int)meminfo.SReclaimable));
    MMESHELL_PRINT(MMESHELL_INFO, (" SUnreclaim:     %d KB\n\r", (int)meminfo.SUnreclaim));
    MMESHELL_PRINT(MMESHELL_INFO, (" KernelStack:    %d KB\n\r", (int)meminfo.KernelStack));
    MMESHELL_PRINT(MMESHELL_INFO, (" PageTables:     %d KB\n\r", (int)meminfo.PageTables));
    MMESHELL_PRINT(MMESHELL_INFO, (" NFS_Unstable:   %d KB\n\r", (int)meminfo.NFS_Unstable));
    MMESHELL_PRINT(MMESHELL_INFO, (" Bounce:         %d KB\n\r", (int)meminfo.Bounce));
    MMESHELL_PRINT(MMESHELL_INFO, (" WritebackTmp:   %d KB\n\r", (int)meminfo.WritebackTmp));
    MMESHELL_PRINT(MMESHELL_INFO, (" CommitLimit:    %d KB\n\r", (int)meminfo.CommitLimit));
    MMESHELL_PRINT(MMESHELL_INFO, (" Committed_AS:   %d KB\n\r", (int)meminfo.Committed_AS));
    MMESHELL_PRINT(MMESHELL_INFO, (" VmallocTotal:   %d KB\n\r", (int)meminfo.VmallocTotal));
    MMESHELL_PRINT(MMESHELL_INFO, (" VmallocUsed:    %d KB\n\r", (int)meminfo.VmallocUsed));
    MMESHELL_PRINT(MMESHELL_INFO, (" VmallocChunk:   %d KB\n\r", (int)meminfo.VmallocChunk));

    return 0;
}
#endif

int mme_command_system_checktick(int argc, char* argv[]) {

    MMP_U32 cur_tick, before_tick, start_tick;
    int durtick;

    if(argc != 2) {
        MMESHELL_PRINT(MMESHELL_INFO, (" ERROR :  arg is 2  e.g) syst_tick <dur>  \n\r"));
        return -1;
    }

    durtick = atoi(argv[1])*1000;

    start_tick = CMmpUtil::GetTickCount();
    before_tick = start_tick;
    cur_tick = before_tick;
    while( (int)(cur_tick-start_tick) < durtick ) {
    
        cur_tick = CMmpUtil::GetTickCount();
        if((cur_tick-before_tick) > 1000) {
            MMESHELL_PRINT(MMESHELL_INFO, ("%d. cur_tick=%d \n\r", (int)((cur_tick-start_tick)/1000), (int)cur_tick ));

            before_tick = cur_tick;
        }
        CMmpUtil::Sleep(10);
    }

    return 0;
}

int mme_command_system_struct_align(int argc, char* argv[]) {

    struct A {
        char a;
        int i;
    };
    struct B {
        int i;
        char a;
    };

    struct C {
        struct A a;
        struct B b;
    };


    MMESHELL_PRINT(MMESHELL_INFO, ("struct A size = %d \n\r", sizeof(struct A) ));
    MMESHELL_PRINT(MMESHELL_INFO, ("struct B size = %d \n\r", sizeof(struct B) ));
    MMESHELL_PRINT(MMESHELL_INFO, ("struct C size = %d \n\r", sizeof(struct C) ));

    return 0;
}