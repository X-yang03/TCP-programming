#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include<WinSock2.h>
#include<Windows.h>
#include<WS2tcpip.h>
#include<time.h>
#include<list>
#include <conio.h>
#pragma comment(lib,"ws2_32.lib")

#define max_clients 100
#define IP_addr "127.0.0.1"
#define send_len 200
#define help ".help"
#define quit ".quit" //服务端：.quit代表所有客户机退出，.quit %s指用户s退出
#define startserver ".host"
#define cntToServer ".connect"
#define disconnect ".dic"
#define join	".join"
#define reject	".reject"
#define allow	".allow"
#define intro	"Welcome to this chat Channel! Type '.help' to see the commands!"
#define helps	".host\t\tStart a server\n.connect\tJoin the channel\n.quit\t\tQuit the channel"

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
