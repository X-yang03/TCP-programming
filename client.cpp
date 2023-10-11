#include"Tcp Programming.h"
extern bool runningState;

DWORD WINAPI recv_from_server(LPVOID lpParam) {
	SOCKET* sock = (SOCKET*)lpParam;
	char recv_buf[send_len];
	while (true) {
		if (runningState == 0) {
			std::cout << "disconnect!" << std::endl;
			return 0;
		}
		int recv_bytes = recv(*sock, recv_buf, sizeof(recv_buf), 0);
		if (recv_bytes == SOCKET_ERROR) {
			std::cout << "recieving error!" << std::endl;
			continue;
		}
		if (recv_bytes >= send_len) {
			std::cout << "out of reach!" << std::endl;
			recv_bytes = send_len-1;
		}
		recv_buf[recv_bytes] = 0;
		std::cout << recv_buf << std::endl;
	}

}

DWORD WINAPI send_to_server(LPVOID lpParam) {
	SOCKET* sock = (SOCKET*)lpParam;
	char send_buf[send_len] = { 0 };
	while (true) {
		if (runningState == 0) {
			std::cout << "disconnect!" << std::endl;
			return 0;
		}
		std::cin >> send_buf;
		

		send(*sock, send_buf, strlen(send_buf), 0);
	}
}

int _Client::ClientRegister() {
	runningState = 1;
	WORD socketVer = MAKEWORD(2, 2);
	WSADATA wsaData;

	char send_buf[send_len] = { 0 };

	if (WSAStartup(socketVer, &wsaData) != 0) {
		printf("socket Error: %s (errno: %d)\n", strerror(errno), errno);
		std::cout << "socket error: " << strerror(errno) << " (error:" << errno << ")" << std::endl;
		return 1;
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8888);
	//server_addr.sin_addr.S_un.S_addr = inet_addr(IP_addr);
	inet_pton(AF_INET, IP_addr, &server_addr.sin_addr.S_un.S_addr);

	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket == INVALID_SOCKET)
	{
		std::cout << "socket error !" << std::endl;
		return 1;
	}

	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == INVALID_SOCKET) {
		std::cout << "fail to connect" << std::endl;
		return 1;
	}
	std::cout << "connection set up!" << std::endl;
	HANDLE recv_handle, send_handle;
	recv_handle = CreateThread(NULL, 0, recv_from_server
		, &(client_socket), 0, NULL);

	send_handle = CreateThread(NULL, 0, send_to_server
		, &(client_socket), 0, NULL);


	while (true) {
		if (runningState == 0) {
			break;
		}
	}
	closesocket(client_socket);
	WSACleanup();


}