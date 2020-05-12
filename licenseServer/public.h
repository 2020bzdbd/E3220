#ifndef PUBLIC_H
#define PUBLIC_H

#include<iostream>
#include<winsock2.h>  
#include<stdio.h>
#include<vector>
#include<io.h>
#include<string>
#include<fstream>
#include<sstream>
#include<ctime>
#include<ws2tcpip.h>//定义socklen_t
#pragma comment(lib, "WS2_32")  // 链接到WS2_32.lib

using namespace std;

class server
{
public:
	server();
	void init();
	void process();
	void GetLicense(string path, vector<string>& files, vector<int>& is_file_packet, int hierarchy);
	vector<string> split(char *str);

private:
	int listener;//监听套接字
	sockaddr_in  serverAddr;//IPV4的地址方式
	vector <int> socnum;//存放创建的套接字，处理多个客户端的情况
	
};


#endif // !PUBLIC_H
