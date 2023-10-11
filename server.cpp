#include"Tcp Programming.h"

extern bool runningState;

struct __Client {
    SOCKET client_socket;
};

typedef struct __Client client_entry;

client_entry client_list[max_clients];
HANDLE       client_handle[max_clients];
int          client_num = 0;

SOCKET Server;

DWORD WINAPI client_thread(LPVOID lpParam) {  
    //Create Thread中的lpStartAddress：指向线程函数的指针，函数名称没有限制，但是必须有形式：DWORD WINAPI ThreadProc(LPVOID lpParam)
    while (true) {
        client_entry* client = (client_entry*)lpParam;
        char recv_buf[send_len] = { 0 };

        int recv_bytes = recv(client->client_socket, recv_buf,sizeof(recv_buf), 0);
        if (recv_bytes == SOCKET_ERROR) {
            std::cout << "recieving error!" << std::endl;
            recv_bytes = 0;
            continue;
        }
        
        if (recv_bytes >= send_len) {
            std::cout << "out of reach:" <<recv_bytes<< std::endl;
            recv_bytes = 199;
        }
        std::cout << recv_buf << std::endl;

        for (int i = 0; i < client_num; i++) {
            send(client_list[i].client_socket, recv_buf, recv_bytes, 0);
        }

    }
    return 0;
}

DWORD WINAPI host_send(LPVOID lpParam) {
    
    while (true) {
        char send_buf[send_len] = { 0 };
        /*if (runningState == 0) {
            std::cout << "disconnect!" << std::endl;
            return 0;
        }*/
        std::cin >> send_buf;
        if (send_buf == quit) {
            runningState = 0;
            for (int i = 0; i < client_num; i++) {
                closesocket(client_list[i].client_socket);
            }
            closesocket(Server);
            return 0;
        }
        SOCKET client_ptr;
        for (int i = 0; i < client_num; i++) {
            client_ptr = client_list[i].client_socket;
            send(client_ptr, send_buf, strlen(send_buf), 0);
        }
    }
}

int _Server::StartServer()
{
    std::cout << "Server Started!\n";
    runningState = 1;
     
    WORD socketVer = MAKEWORD(2, 2);  //2.2版本
    WSADATA wsaData;

    if (WSAStartup(socketVer, &wsaData) != 0) {
        printf("socket Error: %s (errno: %d)\n", strerror(errno), errno);
        std::cout << "socket error: " << strerror(errno) << " (error:" << errno << ")" << std::endl;
        return 1;
    }

    Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (Server == INVALID_SOCKET)
    {
       std::cout << "socket error !" << std::endl;
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;       //IPV4
    server_addr.sin_port = htons(8888);     //PORT:8888,htons将主机小端字节转换为网络的大端字节
    //server_addr.sin_addr.S_un.S_addr = INADDR_ANY;     //type(sin_addr) == in_addr
    inet_pton(AF_INET, IP_addr, &server_addr.sin_addr.S_un.S_addr);

    if (bind(Server, (LPSOCKADDR)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) { //将Server与server_addr绑定
        std::cout << "bind error!" << std::endl;
        return 1;
    }

    char host_addr[20] = { 0 };

    std::cout << "Host Address: " 
              <<inet_ntop(AF_INET,&server_addr.sin_addr,host_addr,sizeof(host_addr))
              <<":"<<ntohs(server_addr.sin_port) << std::endl;
    
    if (listen(Server, max_clients) == SOCKET_ERROR)
    {
        std::cout << "listen error !" << std::endl;
        return 1;
    }

    std::cout << "Listening........." << std::endl;

    CreateThread(NULL, 0, host_send, 0,0, NULL);

    while (true) {
        sockaddr_in remote_addr;

        int addrlen = sizeof(remote_addr);

        SOCKET client_socket = accept(Server, (SOCKADDR*)&remote_addr, &addrlen);

        if (client_socket == INVALID_SOCKET) {
            std::cout << "client socket error!" << std::endl;
            continue;
        }

        if (client_num == max_clients) {
            std::cout << "exceeded maximal!" << std::endl;
            continue;
        }

        client_list[client_num].client_socket = client_socket;

        char client_addr[20] = { 0 };
        std::cout << "a new connection from: " << inet_ntop(AF_INET, &remote_addr.sin_addr, client_addr, sizeof(client_addr)) << std::endl;


        client_handle[client_num] = CreateThread(NULL, 0, client_thread
                                                , &(client_list[client_num]), 0, NULL);
        client_num++;

    }


}


