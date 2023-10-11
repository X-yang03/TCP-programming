#include"Tcp Programming.h"

extern bool runningState = 1;

int main() {
	std::cout << "a" << std::endl;
	char command[10];
	while (true) {
		std::cin >> command;
		if (strcmp(command, ".host") == 0) {
			//StartServer();
			_Server server;
			server.StartServer();
		}
		else if (strcmp(command, ".connect") == 0) {
			_Client client;
			client.ClientRegister();
		}
		else if (strcmp(command, ".quit") == 0) {
			return 0;
		}
		else if (strcmp(command, ".")) {

		}
	}
}

