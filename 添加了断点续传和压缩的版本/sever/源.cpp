#include"Server.h"

int main()
{
	int flag = 0;
	Server::__start();
	while (flag == 0)Server::__trans(flag);
	Server::__end();
	system("pause");
}