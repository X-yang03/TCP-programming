#include"Tcp Programming.h"

extern bool runningState;

HANDLE recv_handle, send_handle;

sockaddr_in server_addr;

char ID[21] = { 0 };

bool check_valid(char* str) {  //检查ip地址是否合法
	int a, b, c, d;
	if (4 == sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d)) {
		if (0 <= a && a <= 255
			&& 0 <= b && b <= 255
			&& 0 <= c && c <= 255
			&& 0 <= d && d <= 255) {
			return true;
		}
		
	}
	return false;
}

DWORD WINAPI recv_from_server(LPVOID lpParam) {
	SOCKET* sock = (SOCKET*)lpParam;
	while (true) {
		char recv_buf[send_len] = { 0 };
		int recv_bytes = recv(*sock, recv_buf, sizeof(recv_buf), 0);

		if (strcmp(recv_buf, quit) == 0) {			//收到服务端的.quit
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);  //提示字体为红色
			std::cout << "Server Shut Down! You will be reloacated to the Menu in 3 seconds" << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);		//还原
			Sleep(3000);
			runningState = 0;	//置零
			closesocket(*sock);
		}
		if (runningState == 0) {	//客户端输入.quit，会导致recv_bytes == SOCKET_ERROR
			CloseHandle(recv_handle);
			recv_handle = NULL;
			return 0;
		}

		if (recv_buf[0] == '.') {	//服务端发送指令
			char com[100];
			char name[21];
			sscanf(recv_buf, "%s %s", com, name);
			if (strcmp(com, disconnect) == 0) {		//异常断开信息
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
				printf("Client %s disconnected abnormally!\n", name);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				continue;
			}
			else if (strcmp(com,quit) == 0) {	//客户退出信息
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
				printf("Client %s quit the conversation!\n", name);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				continue;
			}
			else if (strcmp(com, join) == 0) {	//用户加入信息
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
				printf("Client %s  join the conversation!\n", name);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				continue;
			}
			else if (strcmp(com, kick) == 0) {	//用户被踢出
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
				printf("Client %s is kicked out of the channel!\n", name);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				continue;
			}

		}

		if (recv_bytes == SOCKET_ERROR) {	//服务器异常断开
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);  //提示字体为红色
			std::cout << "Fail to receive！" << std::endl;
			std::cout << "Retrying...." << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			
			if (connect(*sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == INVALID_SOCKET) {
				std::cout << "Fail! You will be reloacated to the Menu in 3 seconds" << std::endl;
				Sleep(3000);
				runningState = 0;
				CloseHandle(recv_handle);
				closesocket(*sock);
				
				return 1;	
			}

			continue;
		}
		if (recv_bytes >= send_len) {	//输入超限
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
			std::cout << "Exceeded the maximal length! Type again." << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			continue;
		}
		

		recv_buf[recv_bytes] = 0;
		std::cout <<std::endl<< recv_buf << std::endl;
	}

}

DWORD WINAPI send_to_server(LPVOID lpParam) {  //读线程
	SOCKET* sock = (SOCKET*)lpParam;
	
	while (true) {
		char send_buf[send_len] = { 0 };
		while (!_kbhit()) {			//非阻塞监听
			if (runningState == 0) { //当接收到主机的.quit时运行此段
				CloseHandle(send_handle);
				send_handle = NULL;
				std::cout << "quit!!" << std::endl;
				return 0;
			}
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
		std::cout << ID << "(me) : ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		while (strlen(send_buf) == 0) {
			std::cin.getline(send_buf, send_len, '\n');	//读入整行
		}

		if (strlen(send_buf) >= send_len) {
			std::cout << "Exceeded Maximal Length!" << std::endl;
			continue;
		}

		if (!strcmp(send_buf, help)) {	//.help
			std::cout << helps << std::endl;
			continue;
		}
		else if (!strcmp(send_buf, IP)) {	//.ip
			system("ipconfig");
			continue;
		}
		
		if (send(*sock, send_buf, strlen(send_buf), 0) == SOCKET_ERROR) {
			std::cout << "Sending error!" << std::endl;
		}

		if (strcmp(send_buf, quit) == 0) {   //客户端输入.quit退出,也会先将.quit传送至Server，说明该client断开
			CloseHandle(send_handle);
			send_handle = NULL;
			runningState = 0;	//置零
			closesocket(*sock);

			return 0;	//线程返回
		}
	}
}

int _Client::ClientRegister() {
	runningState = 1;
	WORD socketVer = MAKEWORD(2, 2);
	WSADATA wsaData;

	char send_buf[send_len] = { 0 };

	if (WSAStartup(socketVer, &wsaData) != 0) {
		std::cout << "socket error: " << strerror(errno) << " (error:" << errno << ")" << std::endl;
		return 1;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY| FOREGROUND_GREEN );
	std::cout << "Input the address of the channel (or type 'd' to use the default address):";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
	char addr[20] = { 0 };
	std::cin >> addr;

	if (strcmp(addr, "d") == 0) {
		strcpy(addr, IP_addr);
	}
	else if (!check_valid(addr)) { //地址非法，将用默认地址
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED );
		std::cout << "Invalid address! Using default address to connect." << std::endl;
		strcpy(addr, IP_addr);
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8888);
	inet_pton(AF_INET, addr, &server_addr.sin_addr.S_un.S_addr);

	SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket == INVALID_SOCKET)
	{
		std::cout << "socket error !" << std::endl;
		return 1;
	}

	//向server_socket发起连接
	if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == INVALID_SOCKET) { 
		std::cout << "Fail to connect. You will be relocated to the menu in 3 seconds" << std::endl;
		Sleep(3000);
		return 1;
	}
	
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	
	while (true)  //输入用户名，直到用户名合法
	{
		std::cout << "Your ID (less than 20 character):" << std::endl;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
		std::cin >>std::setw(21)>> ID;
		send(server_socket, ID, strlen(ID), 0);
		char ans[10] = { 0 };
		recv(server_socket, ans, sizeof(ans), 0);
		if (strcmp(ans, allow) == 0) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
			std::cout << "Connection set up!" << std::endl;
			break;
		}
		else if (strcmp(ans, reject) == 0) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED );
			std::cout << "This ID is already used by others or this is an illegal ID!" << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	recv_handle = CreateThread(NULL, 0, recv_from_server
		, &(server_socket), 0, NULL);

	send_handle = CreateThread(NULL, 0, send_to_server
		, &(server_socket), 0, NULL);


	while (true) {
		if (runningState == 0) {
			break;
		}
	}
	
	WSACleanup();
	return 0;

}