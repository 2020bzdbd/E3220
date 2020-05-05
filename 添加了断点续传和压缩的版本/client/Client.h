#pragma once
#include"public_using_head.h"
class Client
{
private:
	static SOCKET s_server;
	//static SOCKET s_accept;
	//服务端地址客户端地址
	static SOCKADDR_IN server_addr;
	//static SOCKADDR_IN accept_addr;
	static string file_path;

	static int mother_file_index_from_start;
	static int mother_file_length;
	static int p_counter;

	static void initialization();
	static bool file_transmision(string file_name);
	static bool sent_create_file_messege(string file_path);
	static bool sent_file_name(string file_name);
	static bool sent_end_transmission();
	static void getAllFiles(string path, vector<string>& files, vector<int>& is_file_packet, int hierarchy);
	static void set_file_length(char* ch, int len);
public:
	static void __connect();
	static void re__connect();
	static void __trans(int &flag);
	static void __exit();
};