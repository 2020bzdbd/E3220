#pragma once
#include"public_using_head.h"

class Server
{
private:
	//static SOCKET s_client;
	//static SOCKADDR_IN client_addr;
	static SOCKET s_server;
	static SOCKET s_accept;
	//服务端地址客户端地址
	static SOCKADDR_IN server_addr;
	static SOCKADDR_IN accept_addr;
	static string file_path;

	static queue<char*> infor_buf;
	static queue<int> infor_len_buf;
	static bool is_end;
	static enum State { complete_end, infor_break,order_break };

	static int get_operate_number(const char* buf);
	static void initialization();
	static int get_messeage_len(const char* recv_current_m_len);
	static bool recv_from_client();
public:
	static void __start();
	static void re__start();
	static void __trans(int &flag);
	static void __end();
};