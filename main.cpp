#include"Tcp Programming.h"

extern bool runningState = 1;

void print_menu() {
	char menu[100] = { 0 };
	strcpy(menu, intro);
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(hOutput, &bInfo);//获取窗口长度
	int len = bInfo.dwSize.X / 2 - strlen(menu) / 2;//空多少个格
	std::cout << std::setw(len)<<" " << menu;
}

int main() {
	print_menu();
	std::cout << "" << std::endl;
	char command[10];
	while (true) {
		std::cin >> command;
		if (strcmp(command, ".host") == 0) {
			//StartServer();
			_Server server;
			server.StartServer();
			system("cls");
			print_menu();
		}
		else if (strcmp(command, ".connect") == 0) {
			_Client client;
			client.ClientRegister();
			system("cls");
			print_menu();
		}
		else if (strcmp(command, ".quit") == 0) {
			return 0;
		}
		else if (strcmp(command, ".help") == 0) {
			std::cout << helps<<std::endl;
		}
	}
}

