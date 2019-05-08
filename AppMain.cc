#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./Header/TaskManager.h"
#include "./Header/config.h"


void* clientConnHandler(void* arg);

int main(int argc, char** argv)
{
    int nExitCode = 0;

    printf("Server starting....\r\n");
    printf("Server addr [%s:%d]\r\n", server_ip, server_port);

    // Create server listenning socket
    int sockListen = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockListen)
    {
        printf("Create listenning socket failed with errno=%d\r\n", errno);
        nExitCode = 1;
        goto End;
    }

    // Bind socket
    struct sockaddr_in addrServer;
    addrServer.sin_addr.s_addr = inet_addr(server_ip);
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(server_port);
    if (-1 == bind(sockListen, (sockaddr*)&addrServer, sizeof(struct sockaddr_in)))
    {
        printf("Server bind socket failed with errno=%d\r\n", errno);
        nExitCode = 1;
        goto End;
    }

    // Listen
    if (-1 == listen(sockListen, SOMAXCONN))
    {
        printf("Server listen function failed with errno=%d\r\n", errno);
        nExitCode = 1;
        goto End;
    }

    // Wait for the clients connect
    while(1)
    {
        struct sockaddr_in addrClient;
        socklen_t nLen = sizeof(struct sockaddr_in);
        int sockClient = accept(sockListen, (sockaddr*)&addrClient, &nLen);
        if (sockClient < 0)
        {
            printf("Client connect failed with errno=%d\r\n", errno);
            goto End;
        }        

        TSingleton<TaskManager>::Instance()->addTask(sockClient, addrClient.sin_addr.s_addr, addrClient.sin_port);
    }    

End:
    printf("Server exit(%d)\r\n", nExitCode);
    return 0;
}
