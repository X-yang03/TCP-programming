#include"Tcp Programming.h"
#include <WinSock.h>
#include <IPHlpApi.h>
#pragma comment(lib, "Iphlpapi.lib")

extern bool runningState;

struct __Client {
    SOCKET client_socket;
    int client_no;
    __Client* self; //指向自身
    char name[21];
};

typedef struct __Client client_entry;


std::list<client_entry> client_list;


int          client_num = 0;    //总共的client数量
int          client_no = 0;     //第i个连接的client，用于标识client

SOCKET Server;

char Server_ID[30];

char IPV4[20] = { 0 };

//获取ipv4地址
void get_IPV4() {
    IP_ADAPTER_ADDRESSES* adapterAddresses = NULL;
    ULONG bufferSize = 0;

    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
        adapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize) == NO_ERROR) {
            IP_ADAPTER_ADDRESSES* adapter = adapterAddresses;
            while (adapter) {
                if (adapter->OperStatus == IfOperStatusUp && adapter->IfType == IF_TYPE_IEEE80211) {
                    IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress;
                    while (unicast) {
                        sockaddr* sockaddr = unicast->Address.lpSockaddr;
                        if (sockaddr->sa_family == AF_INET) {
                            char ip[INET_ADDRSTRLEN];
                            sockaddr_in* ipv4 = (sockaddr_in*)sockaddr;
                            inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
                            //std::cout << "IPv4 Address: " << ip << std::endl;
                            sprintf(IPV4, ip);
                        }
                        unicast = unicast->Next;
                    }
                }
                adapter = adapter->Next;
            }
        }
        free(adapterAddresses);
    }
}


DWORD WINAPI client_thread(LPVOID lpParam) {  
    
    //Create Thread中的lpStartAddress：指向线程函数的指针，函数名称没有限制，但是必须有形式：DWORD WINAPI ThreadProc(LPVOID lpParam)
    while (true) {
        client_entry* client = (client_entry*)lpParam;
        char recv_buf[send_len] = { 0 };
        char client_msg[send_len] = { 0 };
        strcpy(client_msg, client->name);   //msg头
        strcat(client_msg, " : \t");
        int recv_bytes = recv(client->client_socket, recv_buf,sizeof(recv_buf), 0);
        if (runningState == 0) {    //服务端输入.quit,直接返回，销毁线程
            return 0;
        }

        //两种情况，一种是直接客户端关闭，一种是被踢出
        if (recv_bytes == SOCKET_ERROR) {  
            char error_info[100];
            sprintf(error_info, ".dic %s",client->name);

            int flag = 0;  //flag 用于判断该client是否被踢出
            for (auto it = client_list.begin(); it != client_list.end(); it++) {    //删去该出错用户

                if (it->self->client_no == client->client_no) {
                    if (it->client_no == -1) {  //被剔除用户，对应的list中的项的no是-1
                        flag = 1;
                    }
                    closesocket(it->client_socket);
                    client_list.erase(it);
                    client_num--;
                    break;
                }
            }
            if (flag) {  //若是被踢出，则直接返回
                
                return 0;
            }

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
            printf("\nRecieving error from client %s, which is no longer in the channel!\n", client->name); //异常断开
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

            for (auto it = client_list.begin(); it != client_list.end(); it++) {  //转发至其他客户机
                    send(it->client_socket,error_info, strlen(error_info), 0);
            }
            delete client;
            return 1;
        }
        
        if (recv_bytes >= send_len) {
            std::cout << "Exceeded Maximum:" <<recv_bytes<< std::endl;
            recv_bytes = send_len-1;
            recv_buf[recv_bytes] = 0;
        }

        //收到来自该client线程的.quit
        if (strcmp(recv_buf, quit) == 0) {   
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

        //用户端想查询state
        if (strcmp(recv_buf, state) == 0) { 
            char state_info[send_len] = { 0 };
            char server_info[50] = { 0 };
            sprintf(server_info, "Server\t\t%s\n", Server_ID);
            strcpy(state_info, server_info);
            strcat(state_info, "-----------------------------\n");
            strcat(state_info, "Client Code\t\tClient ID\n");
            for (auto it = client_list.begin(); it != client_list.end(); it++) {
                //std::cout << it->client_no << "\t\t\t" << it->name << std::endl;
                char single_info[50] = { 0 };
                sprintf(single_info, "%d\t\t\t%s\n", it->client_no, it->name);
                strcat(state_info, single_info);
            }
            send(client->client_socket, state_info, strlen(state_info), 0);  //将信息返回给client
            continue;
        }

        strcat(client_msg, recv_buf);

        for (auto it = client_list.begin(); it != client_list.end(); it++) {  //转发至其他客户机
            if(it->client_no != client->client_no)
                send(it->client_socket, client_msg, strlen(client_msg), 0);
        }
        printf("\n%s\n", client_msg);

        //std::cout<<std::endl << client_msg << std::endl;

    }
    return 0;
}

DWORD WINAPI host_send(LPVOID lpParam) {
    
    while (true) {
        char send_buf[send_len] = { 0 };
        char send_msg[send_len] = { 0 };

        strcpy(send_msg, Server_ID);
        strcat(send_msg, " : \t");
        while (!_kbhit()) {

        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << Server_ID << "(server) : ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        //std::cout << "-----";
        while (strlen(send_buf) == 0) {
            std::cin.getline(send_buf, send_len, '\n');
        }

        //输入超限
        if (strlen(send_buf) >= send_len) {
            std::cout << "Exceeded Maximal Length!" << std::endl;
            continue;
        }

        //主机输入.quit
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

        //主机输入指令
        if (send_buf[0] == '.') { //主机输入指令
            char com[10] = { 0 };
            char target[21] = { 0 };
            if (strcmp(send_buf, state) == 0) {   //.state
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
                std::cout << "Client code\t\tClient ID" << std::endl;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);

                for (auto it = client_list.begin(); it != client_list.end(); it++) {
                    std::cout << it->client_no << "\t\t\t" << it->name << std::endl;
                }
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
                continue;  //.state不需要传递给客户机
            }
            else if (sscanf(send_buf, "%s %s", com, target) && !strcmp(com, kick)) { //输入.kick sb.
                int flag = 1;
                for (auto it = client_list.begin(); it != client_list.end(); it++) {
                    if (strcmp(it->name, target) == 0) {
                        sprintf(send_msg, ".kick %s", it->name);
                        std::cout << send_msg << std::endl;
                        strcpy(send_buf, "\0");
                        it->client_no = -1; //被踢掉的序号设为-1
                        closesocket(it->client_socket);  //通过关闭socket，该socket对应的recv_thread会进入SOCKET_ERROR处理函数，进一步从链表中删去client_entry,并释放资源
                        flag = 0;
                    }
                }
                if (flag) {
                    printf("No such client with ID %s\n", target);
                    continue;
                }
                

            }
            else if (!strcmp(send_buf,help)) {
                std::cout << helps << std::endl;
                continue;

            }
            else if (!strcmp(send_buf, IP)) {
                //system("ipconfig");
                printf("your IPV4 address: %s\n", IPV4);
                continue;
            }
        }
        
         
        strcat(send_msg, send_buf);
        for (auto it = client_list.begin(); it != client_list.end(); it++) {  
            if (it->client_no == -1)
                continue;
            send(it->client_socket, send_msg, strlen(send_msg), 0);
        }

        
    }
}

int _Server::StartServer()
{
    runningState = 1;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
    get_IPV4();
    std::cout<<"Your IPV4 address:" << IPV4 << std::endl;
    //Clog.open("./Channel Log.log");
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

    //选择服务端IP
    while (true) {
        char T;
        std::cout << "Input the address of the channel('d' for default address and 'i' for IPV4 address):" << std::endl;
        std::cin >> T;
        if (T == 'd') {
            server_addr.sin_addr.S_un.S_addr = INADDR_ANY;     //type(sin_addr) == in_addr
        }
        else if (T == 'i') {
            inet_pton(AF_INET, IPV4, &server_addr.sin_addr.S_un.S_addr);
        }
        else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
            std::cout << "Invalid input, using default address!" << std::endl;
            inet_pton(AF_INET, IPV4, &server_addr.sin_addr.S_un.S_addr);
        }
        break;
    }
    //server_addr.sin_addr.S_un.S_addr = INADDR_ANY;     //type(sin_addr) == in_addr
    //inet_pton(AF_INET, IP_addr, &server_addr.sin_addr.S_un.S_addr);

    if (bind(Server, (LPSOCKADDR)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) { //将Server与server_addr绑定
        std::cout << "bind error!" << std::endl;
        return 1;
    }

    char host_addr[20] = { 0 };

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY  | FOREGROUND_GREEN);

    std::cout << "Host Address: " 
              <<inet_ntop(AF_INET,&server_addr.sin_addr,host_addr,sizeof(host_addr))
              <<":"<<ntohs(server_addr.sin_port) << std::endl;

    
    
    std::cout << "Server name (less than 20 character):" ;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |  FOREGROUND_GREEN | FOREGROUND_BLUE);
    while (true) {
        std::cin >> std::setw(21) >> Server_ID;
        if (Server_ID[0] == '.') {
            std::cout << "Invalid ID, input again!" << std::endl;//不能以.开头
            continue;
        }
        break;
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |  FOREGROUND_GREEN );

    if (listen(Server, max_clients) == SOCKET_ERROR)
    {
        std::cout << "listen error !" << std::endl;
        return 1;
    }

    std::cout << "Listening........." << std::endl;

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    CreateThread(NULL, 0, host_send, 0,0, NULL);    //开启一个发送线程

    //Server会不断循环，等待新客户机连接
    while (true) {          

        sockaddr_in remote_addr;

        int addrlen = sizeof(remote_addr);


        SOCKET client_socket = accept(Server, (SOCKADDR*)&remote_addr, &addrlen);

        if (runningState == 0) {  //当服务端关闭后，client_socket == INVALID_SOCKET,会运行至此，成功退出
            std::cout << "Host Aborted!" << std::endl;
            WSACleanup();
            return 0;
        }

        if (client_num == max_clients) {
            std::cout << "Exceeded maximum!" << std::endl;
            continue;
        }

        if (client_socket == INVALID_SOCKET) {
            std::cout << "client socket error!" << std::endl;
            continue;
        }

        char name[21] = { 0 };
        char ans[10] = { 0 };
        while (true)  //直到用户名合法
        {
            int name_len =  recv(client_socket, name, sizeof(name), 0);  //建立连接后先接受ID
            name[name_len] = 0;
            strcpy(ans, allow);
            for (auto it = client_list.begin(); it != client_list.end(); it++) {
                if (strcmp(it->name, name) == 0) {  //ID重复
                    strcpy(ans, reject);
                    send(client_socket, ans, strlen(ans), 0);
                    break;
                }
            }
            if (strcmp(name, Server_ID) == 0) { //与主机的Server_ID重复
                strcpy(ans, reject);
                send(client_socket, ans, strlen(ans), 0);
            }
            if (name[0] == '.') { //不可以.开头
                strcpy(ans, reject);
                send(client_socket, ans, strlen(ans), 0);
            }
            if (strcmp(ans, allow) == 0) {  //通过所有检测，用户名称合法
                break;
            }

        }

        if (send(client_socket, ans, strlen(ans), 0) != SOCKET_ERROR) {
            char client_addr[100] = { 0 };
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
            std::cout << "A new connection from: " << inet_ntop(AF_INET, &remote_addr.sin_addr, client_addr, sizeof(client_addr)) << ":" << ntohs(remote_addr.sin_port) << std::endl; 
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        }
        else {
            continue;
        }
        //send(client_socket, helps, strlen(helps), 0);
       
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


