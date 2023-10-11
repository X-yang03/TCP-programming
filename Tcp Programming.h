#pragma once

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
#define IP_addr "127.0.0.1"
#define send_len 200
#define help ".help"
#define quit ".quit"
#define startserver ".host"
#define cntToServer ".connect"

#define commands ".host--------start a server \n fdsa"

class _Server {
private:
	
public:
	
	int StartServer();
};

class _Client {
public:
	int ClientRegister();
	int FreeClient();

};
