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
#include<ws2tcpip.h>//����socklen_t
#pragma comment(lib, "WS2_32")  // ���ӵ�WS2_32.lib

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
	int listener;//�����׽���
	sockaddr_in  serverAddr;//IPV4�ĵ�ַ��ʽ
	vector <int> socnum;//��Ŵ������׽��֣��������ͻ��˵����
	
};


#endif // !PUBLIC_H
