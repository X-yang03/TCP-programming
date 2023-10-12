#include"Tcp Programming.h"

extern bool runningState = 1;

void print_menu() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	char menu[100] = { 0 };
	strcpy(menu, intro);
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(hOutput, &bInfo);//获取窗口长度
	int len = bInfo.dwSize.X / 2 - strlen(menu) / 2;//空多少个格
	std::cout << std::setw(len)<<" " << menu<<std::endl;
}

int main() {
	print_menu();

	char command[10];
	while (true) {
		std::cin >> command;
		system("cls");
		print_menu();
		if (strcmp(command,startserver) == 0) {
			//StartServer();
			_Server server;
			server.StartServer();
			std::cout << "done!" << std::endl;
			system("cls");
			print_menu();
			
		}
		else if (strcmp(command, cntToServer) == 0) {
			_Client client;
			client.ClientRegister();
			system("cls");
			print_menu();
			
		}
		else if (strcmp(command, quit) == 0) {
			return 0;
		}
		else if (strcmp(command, help) == 0) {
			std::cout << helps<<std::endl;
		}
		else if (strcmp(command, IP) == 0) {
			system("ipconfig");
		}
	}
}

