#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
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
#define IP	".ip"
#define send_len 300
#define help ".help"
#define quit ".quit" //服务端：.quit代表所有客户机退出，.quit %s指用户s退出
#define startserver ".host"
#define cntToServer ".connect"
#define disconnect ".dic"
#define join	".join"
#define reject	".reject"
#define allow	".allow"
#define state	".state"
#define kick	".kick"
#define Private	".p"
#define Return	".r"
#define intro	"Welcome to this chat Application! Type '.help' to see the commands!"
#define helps	".host\t\tStart a server(Use in menu)\n\
.connect\tJoin the channel(Use in menu)\n\
.quit\t\tQuit the channel or Close the server\n\
.state\t\tcheck the members in this conversation(Use in channel)\n\
.kick\t\tKick someone out of the channel(Only Server could use this command)\n\
.ip\t\tCheck your IP status(Use in menu)\n"



class _Server {
private:
	
public:
	
	int StartServer();
};

class _Client {
public:
	int ClientRegister();

};
