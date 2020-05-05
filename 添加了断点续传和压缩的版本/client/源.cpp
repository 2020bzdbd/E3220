#include"Client.h"

int main()
{
	int flag = 0;//定义传输结束标志
	Client::__connect();
	while (flag == 0)Client::__trans(flag);
	Client::__exit();
	system("pause");
	return 0;
}