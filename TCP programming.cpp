// TCP programming.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include<WinSock2.h>
#include<Windows.h>
#include<WS2tcpip.h>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")

#define max_clients 100

struct Client {
    SOCKET client_socket;
};

typedef struct Client client_entry;

client_entry client_list[max_clients];
HANDLE  client_handle[max_clients];
int client_num = 0;

DWORD WINAPI client_thread(LPVOID lpParam) {  
    //Create Thread中的lpStartAddress：指向线程函数的指针，函数名称没有限制，但是必须有形式：DWORD WINAPI ThreadProc(LPVOID lpParam)
    while (true) {
        client_entry* client = (client_entry*)lpParam;
        char recv_buf[200] = { 0 };

        int recv_bytes = recv(client->client_socket, recv_buf, 200, 0);
        if (recv_bytes == SOCKET_ERROR) {
            std::cout << "recieving error!" << std::endl;
            continue;
        }
        if (recv_bytes >= 200) {
            std::cout << "out of reach!" << std::endl;
            recv_bytes = 199;
        }
        recv_buf[recv_bytes] = 0;
        std::cout << recv_buf << std::endl;

    }
    return 0;
}

int main()
{
    std::cout << "Hello World!\n";

    WORD socketVer = MAKEWORD(2, 2);
    WSADATA wsaData;

    if (WSAStartup(socketVer, &wsaData) != 0) {
        printf("socket Error: %s (errno: %d)\n", strerror(errno), errno);
        std::cout << "socket error: " << strerror(errno) << " (error:" << errno << ")" << std::endl;
        return 1;
    }

    SOCKET Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (Server == INVALID_SOCKET)
    {
       std::cout << "socket error !" << std::endl;
        return 1;
    }

    sockaddr_in sin;
    sin.sin_family = AF_INET;       //IPV4
    sin.sin_port = htons(8888);     //PORT:8888
    sin.sin_addr.S_un.S_addr = INADDR_ANY;     //type(sin_addr) == in_addr

    if (bind(Server, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) {
        std::cout << "bind error!" << std::endl;
        return 1;
    }

    char host_name[20] = { 0 };
    char host_addr[20] = { 0 };

    std::cout << "Host Address: " 
              <<inet_ntop(AF_INET,&sin.sin_addr,host_addr,sizeof(host_addr))
              <<":"<<sin.sin_port << std::endl;


    return 0;
    if (listen(Server, 50) == SOCKET_ERROR)
    {
        std::cout << "listen error !" << std::endl;
        return 1;
    }

    std::cout << "Listening........." << std::endl;

    while (true) {
        sockaddr_in remote;

        int addrlen = sizeof(remote);

        SOCKET client_socket = accept(Server, (SOCKADDR*)&remote, &addrlen);

        if (client_socket == INVALID_SOCKET) {
            std::cout << "client socket error!" << std::endl;
            continue;
        }

        if (client_num == max_clients) {
            std::cout << "exceeded maximal!" << std::endl;
            continue;
        }

        client_list[client_num].client_socket = client_socket;

        char client_addr[20];
        std::cout << "Host Address: " << inet_ntop(AF_INET, &remote.sin_addr, client_addr, sizeof(client_addr)) << std::endl;

        std::cout << "a new connection built up!" << std::endl;

        client_handle[client_num] = CreateThread(NULL, 0, client_thread
                                                , &(client_list[client_num]), 0, NULL);
        client_num++;


    }



}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
