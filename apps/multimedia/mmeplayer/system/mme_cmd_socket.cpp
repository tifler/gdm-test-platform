#include "mme_shell.h"
#include "TemplateList.hpp"
#include "MmpUtil.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP_LINUX_SERVER "205.239.162.120"
#define SERVER_IP_MY_LAPTOP    "210.219.209.222"

#define SERVER_IP SERVER_IP_MY_LAPTOP
#define SERVER_PORT 6516

int mme_command_socket_connect_to_server(int argc, char* argv[]) {

    struct sockaddr_in c_addr;
    int c_socket;
    int iret;
    char buffer[256];
    int readsz;
    
    c_socket= socket(PF_INET, SOCK_STREAM, 0);
    if(c_socket < 0) {
        MMESHELL_PRINT(MMESHELL_INFO, ("[mme_command_socket_connect_to_server] FAIL: alloc socket \n"));
        return -1;
    }

    memset(&c_addr, 0, sizeof(c_addr));
    c_addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(SERVER_PORT); 

    iret = connect(c_socket, (struct sockaddr *)&c_addr, sizeof(c_addr));
    if(iret < 0) {
        MMESHELL_PRINT(MMESHELL_INFO, ("[mme_command_socket_connect_to_server] FAIL: connect to server \n"));
        return -1;
    }
    else {
        MMESHELL_PRINT(MMESHELL_INFO, ("[mme_command_socket_connect_to_server] SUCCESS: connect to server \n"));
    }

    write(c_socket, "AT", 2);

    while(1) {
        readsz = read(c_socket, buffer, 256);
        MMESHELL_PRINT(MMESHELL_INFO, ("read size : %d  \n", readsz));

    }


    close(c_socket);

    return 0;
}
