#include"Tcp Programming.h"

extern bool runningState;

struct __Client {
    SOCKET client_socket;
    int client_no;
    __Client* self; //指向自身
    char name[21];
};

typedef struct __Client client_entry;

//client_entry client_list[max_clients];

std::list<client_entry> client_list;

//std::list<HANDLE>   client_handle;

int          client_num = 0;    //总共的client数量
int          client_no = 0;     //第i个连接的client，用于标识client

SOCKET Server;

char Server_ID[30];

DWORD WINAPI client_thread(LPVOID lpParam) {  
    
    //Create Thread中的lpStartAddress：指向线程函数的指针，函数名称没有限制，但是必须有形式：DWORD WINAPI ThreadProc(LPVOID lpParam)
    while (true) {
        client_entry* client = (client_entry*)lpParam;
        char recv_buf[send_len] = { 0 };
        char client_msg[send_len] = { 0 };
        strcpy(client_msg, client->name);
        strcat(client_msg, " : ");
        int recv_bytes = recv(client->client_socket, recv_buf,sizeof(recv_buf), 0);
        if (runningState == 0) {    //服务端输入.quit,直接返回
            return 0;
        }

        if (recv_bytes == SOCKET_ERROR) {
            char error_info[100];
            sprintf(error_info, ".dic %s",client->name);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
            printf("\nRecieving error from client %s, which is no longer in the conversation!\n", client->name);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

            for (auto it = client_list.begin(); it != client_list.end(); it++) {    //删去该出错用户
                if (it->client_no == client->client_no) {
                    closesocket(it->client_socket);
                    client_list.erase(it);
                    client_num--;
                    break;
                }
            }

            for (auto it = client_list.begin(); it != client_list.end(); it++) {  //转发至其他客户机
                    send(it->client_socket,error_info, strlen(error_info), 0);
            }
            delete client;
            return 1;
        }
        
        if (recv_bytes >= send_len) {
            std::cout << "out of reach:" <<recv_bytes<< std::endl;
            recv_bytes = send_len-1;
            recv_buf[recv_bytes] = 0;
        }

        if (strcmp(recv_buf, quit) == 0) {   //收到来自该client线程的.quit
            char quit_info[100];
            sprintf(quit_info, ".quit %s", client->name);
            std::cout << quit_info << std::endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED );
            printf("Client %s quit the conversation!\n",client->name);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            for (auto it = client_list.begin(); it != client_list.end(); it++) {
                if (it->client_no == client->client_no) {
                    closesocket(it->client_socket);
                    client_list.erase(it);
                    client_num--;
                    break;
                }
            }

            for (auto it = client_list.begin(); it != client_list.end(); it++) {  //转发至其他客户机
                if (it->client_no != client->client_no)
                    send(it->client_socket, quit_info, strlen(quit_info), 0);
            }
            delete client;
            return 0;
        }


        strcat(client_msg, recv_buf);

        for (auto it = client_list.begin(); it != client_list.end(); it++) {  //转发至其他客户机
            if(it->client_no != client->client_no)
                send(it->client_socket, client_msg, strlen(client_msg), 0);
        }

        //std::cout << "recieve from:" << client->client_no << std::endl;
        std::cout << client_msg << std::endl;

    }
    return 0;
}

DWORD WINAPI host_send(LPVOID lpParam) {
    
    while (true) {
        char send_buf[send_len] = { 0 };
        char send_msg[send_len] = { 0 };

        strcpy(send_msg, Server_ID);
        strcat(send_msg, " : ");
        while (!_kbhit()) {

        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << Server_ID << "(server) : ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        //std::cout << "-----";
        while (strlen(send_buf) == 0) {
            std::cin.getline(send_buf, send_len, '\n');
        }

        if (strcmp(send_buf, quit) == 0) {   //主机输入.quit
            runningState = 0;

            for (auto it = client_list.begin(); it != client_list.end(); it++) {
                send(it->client_socket, send_buf, strlen(send_buf), 0);   //将退出消息传递至各客户机
            }

            for (auto it = client_list.begin(); it != client_list.end(); it++) {
                closesocket(it->client_socket);
                delete it->self;
            }

            closesocket(Server);
            WSACleanup();
            client_list.clear(); //清除所有client
            client_num = 0;
            client_no = 0;
            return 0;
        }
    
        strcat(send_msg, send_buf);
        for (auto it = client_list.begin(); it != client_list.end(); it++) {  
            send(it->client_socket, send_msg, strlen(send_msg), 0);
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

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY  | FOREGROUND_GREEN);

    std::cout << "Host Address: " 
              <<inet_ntop(AF_INET,&server_addr.sin_addr,host_addr,sizeof(host_addr))
              <<":"<<ntohs(server_addr.sin_port) << std::endl;
    
    std::cout << "Server name (less than 20 character):" << std::endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |  FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cin >> std::setw(21) >> Server_ID;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |  FOREGROUND_GREEN );

    if (listen(Server, max_clients) == SOCKET_ERROR)
    {
        std::cout << "listen error !" << std::endl;
        return 1;
    }

    std::cout << "Listening........." << std::endl;

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    CreateThread(NULL, 0, host_send, 0,0, NULL);    //开启一个发送线程

    while (true) {          //Server会不断循环，等待新客户机连接

        sockaddr_in remote_addr;

        int addrlen = sizeof(remote_addr);

        if (client_num == max_clients) {
            std::cout << "Exceeded maximum!" << std::endl;
            return 0;
        }

        SOCKET client_socket = accept(Server, (SOCKADDR*)&remote_addr, &addrlen);

        if (runningState == 0) {  //当服务端关闭后，client_socket == INVALID_SOCKET,会运行至此，成功退出
            std::cout << "Host Aborted!" << std::endl;
            WSACleanup();
            return 0;
        }

        if (client_socket == INVALID_SOCKET) {
            std::cout << "client socket error!" << std::endl;
            continue;
        }

        char name[21] = { 0 };
        char ans[10] = { 0 };
        while (true)
        {
            int name_len =  recv(client_socket, name, sizeof(name), 0);  //建立连接后先接受ID
            name[name_len] = 0;
            strcpy(ans, allow);
            for (auto it = client_list.begin(); it != client_list.end(); it++) {
                if (strcmp(it->name, name) == 0) {  //ID重复
                    strcpy(ans, reject);
                    std::cout << ans << std::endl;
                    send(client_socket, ans, strlen(ans), 0);
                    break;
                }
            }
            if (strcmp(name, Server_ID) == 0) { //与主机的Server_ID重复
                strcpy(ans, reject);
                std::cout << ans << std::endl;
                send(client_socket, ans, strlen(ans), 0);
            }

            if (strcmp(ans, allow) == 0) {  //通过所有检测，用户名称合法
                break;
            }

        }
        std::cout << ans << std::endl;
        if (send(client_socket, ans, strlen(ans), 0) != SOCKET_ERROR) {
            char client_addr[100] = { 0 };
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
            std::cout << "A new connection from: " << inet_ntop(AF_INET, &remote_addr.sin_addr, client_addr, sizeof(client_addr)) << std::endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        }
        else {
            continue;
        }
       
        client_entry* entry = new client_entry;
        entry->client_socket = client_socket;
        entry->client_no = client_no++;
        entry->self = entry;
        strcpy(entry->name, name);
        client_list.push_back(*entry);
     

        HANDLE handle = CreateThread(NULL, 0, client_thread
                                    , entry, 0, NULL);

        CloseHandle(handle);

        /*client_handle.push_back(handle);*/
        client_num++;

        char join_info[100];
        sprintf(join_info, ".join %s", entry->name);
        for (auto i = client_list.begin(); i != client_list.end(); i++) {
            send(i->client_socket, join_info, strlen(join_info), 0);
        }

    }


}


