#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "public.h"

vector<string> SerialNumber;//存储序列号
vector<string> Usernames;//存储用户名
vector<string> Password;//存储密码
vector<int> Maximum;//每个序列号的最大使用人数
vector<int> CurrentNum;//该序列号当前使用人数
vector<vector<int>> UserID;//存储使用该序列号的客户端的ID
vector<bool> Is_Fisrt_Use;//记录该序列号是否第一次被用户使用
vector<int> LastLogedID;//用于存储服务器崩溃前登录过的ID
server ser;

clock_t Start, End;//用于计时
int TimeFlag = 0;//为0是不需要计时，为1时需要计时

#define MaxTime 100//如果上次登录过的用户在崩溃后100秒内没有申请重新登录，就踢下线
#define FILEPATH "license"

string sourcePath = FILEPATH;

//字符串是否全部由数字组成，如果是，返回true，否则返回false
bool isNum(string str)
{
	stringstream sin(str);
	double d;
	char c;
	if (!(sin >> d))return false;
	if (sin >> c)return false;
	return true;
}

//将字符串转化为数字
int StrToNum(string str)
{
	int num;
	stringstream ss(str);
	ss >> num;
	return num;
}

//将许可证信息读入
void GetLicenseInfo(server ser)
{
	//读取信息前先将所有的容器内信息清除
	SerialNumber.clear();
	Maximum.clear();
	CurrentNum.clear();
	UserID.clear();
	Usernames.clear();
	Password.clear();
	Is_Fisrt_Use.clear();
	
	vector<string> files;
	vector<int> is_file_packet;
	ser.GetLicense(FILEPATH, files, is_file_packet, 0);
	
	if (files.size() == 1)cout << "没有任何许可证" << endl;
	else
	{
		vector<string> license;
		string last_username;
		string last_user_password;
		for (int i = 1;i < files.size();i++)
		{
			int pos = 0;
			pos = files[i].rfind("/");
			string str = files[i].substr(pos + 1, files[i].length());
			if (!isNum(str) && str != "info.txt")
			{
				last_username = str;//获取用户名
				//获取该用户名的密码
				string password_filepath = sourcePath + '/' + last_username + "/info.txt";
				ifstream fin;
				fin.open(password_filepath, ios::in);
				string password;
				getline(fin, password);
				last_user_password = password;
				fin.close();
			}
			if (is_file_packet[i] == 1 && isNum(str))
			{
				SerialNumber.push_back(str);//存入序列号
				Usernames.push_back(last_username);//存入用户名
				Password.push_back(last_user_password);//存入密码
				i++;
				ifstream fin;
				fin.open(files[i], ios::in);
				string s;
				int num;
				//获得最大使用人数
				getline(fin, s);
				num = StrToNum(s);
				Maximum.push_back(num);
				string current;
				getline(fin, current);//第二行记录是否是第一次使用
				if (current == "TRUE")Is_Fisrt_Use.push_back(true);
				else Is_Fisrt_Use.push_back(false);
				getline(fin, current);//第三行记录当前使用人数
				num = StrToNum(current);
				CurrentNum.push_back(num);
				if (num > 0)
				{
					getline(fin, current);
					vector<string> IDS = ser.split((char*)current.c_str());
					vector<int> ID;
					for (int a = 0;a < IDS.size();a++)
					{
						num = StrToNum(IDS[a]);
						ID.push_back(num);
					}
					UserID.push_back(ID);
				}
				else
				{
					vector<int> ID;
					UserID.push_back(ID);
				}
				fin.close();
			}
		}
	}
}

//输出许可证的所有信息
void ShowLicenseInfo()
{	
	if (SerialNumber.size() > 0)
	{	
		cout << endl << "{";
		for (int i = 0;i < SerialNumber.size();i++)
		{
			cout << endl << "序列号" << SerialNumber[i] << "相关信息如下：" << endl;
			cout << "该许可证用户：" << Usernames[i];
			cout << "   用户密码：" << Password[i] << endl;
			cout << "是否使用过：";
			if (Is_Fisrt_Use[i] == true)cout << "否" << endl;
			else cout << "是" << endl;
			cout << "人数上限：" << Maximum[i] << " 当前使用人数：" << CurrentNum[i] << endl;
			if (CurrentNum[i] > 0)
			{
				cout << "使用该许可证的客户端ID为：";
				for (int j = 0;j < UserID[i].size();j++)
				{
					if (j == 0)cout << UserID[i][j];
					else cout << " " << UserID[i][j];
				}
				cout << endl;
			}
		}
		cout << "}" << endl << endl;
	}
}

//更新许可证文件
void UpdateLicenseFile(int ser_count)
{
	string filepath;
	filepath = sourcePath + '/' + Usernames[ser_count] + '/' +
		SerialNumber[ser_count] + "/info.txt";
	ofstream fout;
	fout.open(filepath, ios::out);
	fout << Maximum[ser_count] << endl;
	fout << "FALSE" << endl;
	fout << CurrentNum[ser_count] << endl;
	if (UserID[ser_count].size() == 0)fout << "No user";
	for (int j = 0;j < UserID[ser_count].size();j++)
	{
		if (j == 0)fout << UserID[ser_count][j];
		else fout << " " << UserID[ser_count][j];
	}
	fout.close();
}

int main()
{	
	//开启服务器时就将许可证信息读入
	GetLicenseInfo(ser);
	cout << endl << "许可证当前使用情况如下：";
	ShowLicenseInfo();

	//开启服务器时，如果服务器上次没有崩溃，所有序列号的当前使用人数应该全为0
	//出现不为0的情况，则表示服务器崩溃过
	//崩溃过就设置计时器，如果上次登录过的用户超过一定时间没有申请重新登录就踢下线
	for (int i = 0;i < CurrentNum.size();i++)
	{
		if (CurrentNum[i] > 0)
		{
			Start = clock();
			TimeFlag = 1;//设置为需要计时
			for (int a = 0;a < SerialNumber.size();a++)
				for (int b = 0;b < UserID[a].size();b++)
					LastLogedID.push_back(UserID[a][b]);
			break;
		}
	}

	ser.process();

	return 0;
}

//构造函数，设置服务端信息
server::server()
{
	string SERVER_IP;//服务端IP
	int SERVER_PORT;//服务端端口
	cout << "请输入如服务端IP地址和服务端端口号，中间用空格隔开" << endl;
	cin >> SERVER_IP >> SERVER_PORT;
	listener = 0;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = SERVER_PORT;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());//将字符串类型转换uint32_t
}

//初始化函数，功能创建监听套接字，绑定端口，并进行监听
void server::init()
{
	int   Ret;
	WSADATA   wsaData;// 用于初始化套接字环境

	//初始化WinSock环境
	//失败时
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("WSAStartup()   failed   with   error   %d\n", Ret);
		WSACleanup();
	}

	//初始化WinSock环境成功
	listener = socket(AF_INET, SOCK_STREAM, 0);//采用ipv4,TCP传输
	if (listener == -1) 
	{ 
		printf("Error at socket(): %ld\n", WSAGetLastError()); 
		perror("创建失败"); 
		exit(1); 
	}
	printf("创建成功\n");

	unsigned long ul = 1;
	if (ioctlsocket(listener, FIONBIO, (unsigned long*)&ul) == -1) 
	{ 
		perror("ioctl failed"); 
		exit(1); 
	}
	
	if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind error");
		exit(1);
	}
	if (listen(listener, 6) < 0) 
	{ 
		perror("listen failed"); 
		exit(1); 
	}
	socnum.push_back(listener);//将监听套接字加入
}

void server::GetLicense(string path, vector<string>& files, vector<int>& is_file_packet, int hierarchy)
{
	//文件句柄
	long hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;  //很少用的文件信息读取结构
	string p;  //string类很有意思的一个赋值函数:assign()，有很多重载版本
	if (hierarchy == 0)
	{
		if (_findfirst(p.assign(path).c_str(), &fileinfo) != -1)
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				files.push_back(p.assign(path));
				is_file_packet.push_back(1);
			}
			else
			{
				files.push_back(p.assign(path));
				is_file_packet.push_back(0);
			}
		}
	}

	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1) {
		do {
			//cout << fileinfo.name << endl;
			if ((fileinfo.attrib & _A_SUBDIR)) {  //比较文件类型是否是文件夹
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
					files.push_back(p.assign(path).append("/").append(fileinfo.name));
					is_file_packet.push_back(1);
					GetLicense(p.assign(path).append("/").append(fileinfo.name), files, is_file_packet, hierarchy + 1);

				}

			}
			else {
				files.push_back(p.assign(path).append("/").append(fileinfo.name));
				is_file_packet.push_back(0);
			}

		} while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
		_findclose(hFile);
	}
}

void server::process()
{
	int mount = 0;
	fd_set fds;
	FD_ZERO(&fds);//将fds清零

	//初始化，与服务端建立连接
	init();

	printf("正在等待客户端信息......\n");
	while (1)
	{
		mount = socnum.size();
		//将fds每次都重新赋值
		for (int i = 0; i < mount; i++)
		{
			FD_SET(socnum[i], &fds);
		}

		struct timeval timeout = { 1,0 };//设置每隔1秒select一次
		//服务端只设置读，不考虑写
		switch (select(0, &fds, NULL, NULL, &timeout))
		{
		case -1:
		{
			perror("select\n");
			printf("Error at socket(): %ld\n", WSAGetLastError());
			printf("%d\n", mount);
			
			Sleep(1000);
			break;
		}
		case 0:
		{
			break;
		}
		default:
		{
			//将数组中的每一个套接字都和剩余的套接字进行比较得到当前的任务
			for (int i = 0; i < mount; i++)
			{
				//更新许可证信息
				GetLicenseInfo(ser);
				//如果监听套接字可读的消息，就建立连接
				if (i == 0 && FD_ISSET(socnum[i], &fds))
				{
					struct sockaddr_in client_address;
					socklen_t client_addrLength = sizeof(struct sockaddr_in);
					//返回一个用户的套接字
					int clientfd = accept(listener, (struct sockaddr*)&client_address, &client_addrLength);
					//添加用户，服务器上显示消息，并通知用户连接成功
					socnum.push_back(clientfd);
					cout << "客户端 " << clientfd << "成功连接本服务器" << endl;
					char ID[1024];
					sprintf(ID, "你好 你的ID为 %d\n请输入登录信息 序列号(第一次使用才需要) 用户名 密码 ：", clientfd);
					
					//服务器产生ID并发送给客户端让客户端知道自己的ID
					send(clientfd, ID, sizeof(ID) - 1, 0);//减去最后一个'/0'
				}
				if (i != 0 && FD_ISSET(socnum[i], &fds))
				{
					char buf[1024];
					memset(buf, '\0', sizeof(buf));
					int size = recv(socnum[i], buf, sizeof(buf) - 1, 0);
					//检测客户端是否掉线
					if (size == 0 || size == -1)
					{
						cout << "客户端 " << socnum[i] << " 已掉线" << endl;

						//判断掉线的客户端是否有使用序列号
						int ser_count = -1;
						int ID_count = -1;//两者均为-1表示掉线的客户端没有使用任何许可证
						for (int a=0 ;a<UserID.size();a++)
							for (int b=0 ;b < UserID[a].size();b++)
							{
								//掉线的客户端正在使用序列号
								if (socnum[i] == UserID[a][b])
								{
									ser_count = a;
									ID_count = b;
									break;
								}
							}
						if (ser_count == -1 && ID_count == -1)
							cout << "该客户端没有使用任何许可证，许可证使用情况不变" << endl;
						else
						{
							CurrentNum[ser_count]--;

							UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);
							cout << "掉线的客户端使用了许可证" << SerialNumber[ser_count];
							cout << endl << "许可证的使用情况更新，当前使用情况如下：";
							ShowLicenseInfo();

							//对文件进行更新
							UpdateLicenseFile(ser_count);
						}

						closesocket(socnum[i]);//关闭这个套接字
						FD_CLR(socnum[i], &fds);//在列表中删除
						socnum.erase(socnum.begin() + i);//在vector数组中删除
						mount--;
					}
					//若没有掉线
					else
					{
						vector<string> recev_info = split(buf);
						string str;//发送给客户端的信息
						//服务器没有崩溃
						if (recev_info[0] == "UNLOGED")
						{
							//服务端发送退出登录指令
							if (recev_info.size() == 2)
							{
								printf("客户端 %d 发送退出登录请求\n", socnum[i]);

								//检查该客户端是否登录，如果没有登录就拒绝该请求，否则允许退出
								int logor = 0;//为0表示没有登录，为1表示登录
								int ser_count = -1;
								int ID_count = -1;
								for (int a = 0;a < SerialNumber.size();a++)
								{
									for (int b = 0;b < UserID[a].size();b++)
									{
										if (UserID[a][b] == socnum[i])
										{
											logor = 1;
											ser_count = a;
											ID_count = b;
											break;
										}
									}
									if (logor == 1)break;
								}
								if (logor == 0)str = " 您没有登录";
								else
								{
									str = " 您已成功退出登录";
									cout << "客户端 " << socnum[i] << "退出登录" << endl;
									//退出登录
									CurrentNum[ser_count]--;
									UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);
									cout << endl << "许可证的使用情况更新，当前使用情况如下：";
									ShowLicenseInfo();
									//对文件进行更新
									UpdateLicenseFile(ser_count);
								}
							}
							//对用户名，密码进行校验
							else if (recev_info.size() == 3)
							{
								printf("客户端 %d 发送登录请求\n", socnum[i]);

								//为0表示允许客户端登录，为1表示使用人数达到上限，
								//为2表示不存在该序列号或用户名
								//为3表示重复登录
								//为4表示存在改序列号或用户，但用户名，序列号，密码不匹配
								//为5表示该用户第一次登录软件A，需要输入序列号进行验证
								int state = 2;
								int count = 0;//如果允许登录，记录是哪一个序列号被使用

								for (int a = 0;a < Usernames.size();a++)
								{
									//检查该客户端是否已经登录
									for (int b = 0;b < UserID[a].size();b++)
									{
										if (UserID[a][b] == socnum[i])
										{
											state = 3;
											break;
										}
									}
									if (state == 3)break;
									if (Usernames[a] == recev_info[1] && Password[a] != recev_info[2])
									{
										state = 4;
										break;
									}
									if (Usernames[a] == recev_info[1] && Password[a] == recev_info[2])
									{
										//如果用户名存在且与密码匹配，则检查是否重复登录
										for (int b = 0;b < UserID[a].size();b++)
										{
											if (UserID[a][b] == socnum[i])
											{
												state = 3;
												break;
											}
										}
										if (state == 3)break;
										//如果没有重复登录，就检查该许可证使用人数是否达到上限
										if (CurrentNum[a] < Maximum[a])
										{
											//检查是否为第一次使用
											if (Is_Fisrt_Use[a] == true)state = 5;
											else
											{
												state = 0;
												count = a;
												break;
											}
										}
										else state = 1;
									}
								}
								
								if (state == 0)
								{
									str = " 允许登录 若要退出登录，请输入quit：";
									//更新许可证使用情况
									CurrentNum[count]++;
									UserID[count].push_back(socnum[i]);

									//对文件进行更新
									UpdateLicenseFile(count);

									cout << "用户名，密码校验成功，允许登录" << endl;
									cout << endl << "许可证使用情况更新，当前使用情况如下：";
									ShowLicenseInfo();
								}
								else if (state == 1)str = " 该用户所拥有的所有许可证的使用人数达到上限，请稍后再登录或再去购买一个许可证";
								else if (state == 3)str = " 您已经登录,不可重复登录";
								else if (state == 2)str = " 不存在该用户名，请重新输入：";
								else if (state == 5)str = " 该用户目前有空余的许可证为首次使用，需要验证序列号，请重新输入：";
								else str = " 用户名和密码不匹配，请重新输入：";
							}
							//对序列号，用户名和密码进行校验
							else if (recev_info.size() == 4)
							{
								printf("客户端 %d 发送登录请求\n", socnum[i]);

								//为0表示允许客户端登录，为1表示使用人数达到上限，
								//为2表示不存在该序列号或用户名
								//为3表示重复登录
								//为4表示存在该序列号或用户，但用户名，序列号，密码不匹配
								int state = 2;
								int count = 0;//如果允许登录，记录是哪一个序列号被使用
								for (int a = 0;a < Usernames.size();a++)
								{
									//检查该客户端是否已经登录
									for (int b = 0;b < UserID[a].size();b++)
									{
										if (UserID[a][b] == socnum[i])
										{
											state = 3;
											break;
										}
									}
									if (state == 3)break;

									//序列号，用户名，密码不匹配
									if (SerialNumber[a] == recev_info[1] && Usernames[a] != recev_info[2])
										state = 4;
									if (Password[a] != recev_info[3] && Usernames[a] == recev_info[2])
										state = 4;
									if (SerialNumber[a] == recev_info[1] && Password[a] != recev_info[3])
										state = 4;

									//用户名存在且与序列号，密码匹配
									if (Usernames[a] == recev_info[2] && Password[a] == recev_info[3]
										&& SerialNumber[a] == recev_info[1])
									{
										//检查是否重复登录
										for (int b = 0;b < UserID[a].size();b++)
										{
											if (UserID[a][b] == socnum[i])
											{
												state = 3;
												break;
											}
										}
										if (state == 3)break;
										//如果没有重复登录，就检查该许可证使用人数是否达到上限
										if (CurrentNum[a] < Maximum[a])
										{
											state = 0;
											count = a;
											//将该序列号设置为非第一次使用
											Is_Fisrt_Use[a] = false;
											break;
										}
										else state = 1;
									}
								}

								if (state == 0)
								{
									str = " 允许登录 若要退出登录，请输入quit：";
									//更新许可证使用情况
									CurrentNum[count]++;
									UserID[count].push_back(socnum[i]);

									//对文件进行更新
									UpdateLicenseFile(count);

									cout << "序列号，用户名，密码校验成功，允许登录" << endl;
									cout << endl << "许可证使用情况更新，当前使用情况如下：";
									ShowLicenseInfo();
								}
								else if (state == 1)str = " 该用户所拥有的所有许可证的使用人数达到上限，请稍后再登录或再去购买一个许可证";
								else if (state == 3)str = " 您已经登录,不可重复登录";
								else if (state == 2)str = " 不存在该用户名或序列号，请重新输入：";
								else str = " 用户名和密码不匹配，请重新输入：";
							}
							else
							{
								str = " 出现未知错误，请重试";
							}
						}
						//服务器之前崩溃过
						else
						{
							//退出之前的登录
							if (recev_info.size() == 3)
							{
								//获取该客户端崩溃前的ID
								int lastID = StrToNum(recev_info[2]);

								printf("客户端 %d 发送退出登录请求\n", socnum[i]);

								//先检查发送过来的ID是否已经因长时间
								//没有发送重新登录请求而被服务器踢下去
								int find_flag = 0;//没有找到为0，找到为1
								for (int j = 0;j < LastLogedID.size();j++)
									if (LastLogedID[j] == lastID)
									{
										find_flag = 1;
										break;
									}
								if (find_flag == 0)
								{
									str = " 退出登录失败 请在收到这条信息后重试，";
									str = str + "因为服务器重启后，您长时间没有发送请求，已被服务器踢下线，";
								}
								else
								{
									//检查该客户端是否登录，如果没有登录就拒绝该请求，否则允许退出
									int logor = 0;//为0表示没有登录，为1表示登录
									int ser_count = -1;
									int ID_count = -1;
									for (int a = 0;a < SerialNumber.size();a++)
									{
										for (int b = 0;b < UserID[a].size();b++)
										{
											if (UserID[a][b] == lastID)
											{
												logor = 1;
												ser_count = a;
												ID_count = b;
											}
										}
										if (logor == 1)break;
									}
									if (logor == 0)str = " 服务器崩溃前您没有登录或已退出登录";
									else
									{
										//先将LastLogedID里的该ID清除掉
										for (int a = 0;a < LastLogedID.size();a++)
										{
											if (LastLogedID[a] == lastID)
												LastLogedID.erase(LastLogedID.begin() + a);
										}

										str = " 您已成功退出登录";
										cout << "客户端 " << socnum[i] << "退出登录" << endl;
										//退出登录
										CurrentNum[ser_count]--;
										UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);
										cout << endl << "许可证的使用情况更新，当前使用情况如下：";
										ShowLicenseInfo();

										//对文件进行更新
										UpdateLicenseFile(ser_count);
									}
								}
							}
							//重新登录，校验用户名和密码
							else if (recev_info.size() == 4)
							{
								//获取该客户端崩溃前的ID
								int lastID = StrToNum(recev_info[3]);

								printf("客户端 %d 发送重新登录请求\n", socnum[i]);
								//对用户发来的崩溃前的ID进行校验

								//state为0表示允许客户端重新登录上次登录过的序列号，
								//为1表示用户名，密码与之前使用的不匹配
								//为2表示客户端已经被服务器踢下去
								int state = 1;
								int count = -1;//如果允许登录，记录是哪一个序列号被使用

								//先检查发送过来的ID是否已经因长时间
								//没有发送重新登录请求而被服务器踢下去
								int find_flag = 0;//没有找到为0，找到为1
								for (int j = 0;j < LastLogedID.size();j++)
									if (LastLogedID[j] == lastID)
									{
										find_flag = 1;
										break;
									}
								if (find_flag == 0)state = 2;
								else
								{
									//检查该客户端重新登录发送的ID和用户名，密码是否匹配
									for (int j = 0;j < SerialNumber.size();j++)
									{
										for (int a = 0;a < UserID[j].size();a++)
										{
											if (UserID[j][a] == lastID && Usernames[j] == recev_info[1]
												&& Password[j] == recev_info[2])
											{
												count = j;
												state = 0;
												UserID[j][a] = socnum[i];//更改登录ID
												//将LastLogedID里的该ID清除掉
												for (int b = 0;b < LastLogedID.size();b++)
												{
													if (LastLogedID[b] == lastID)
														LastLogedID.erase(LastLogedID.begin() + b);
												}
												break;
											}
										}
										if (count != -1)break;
									}
								}

								if (state == 0)
								{
									str = " 已重新登录";

									//对文件进行更新
									UpdateLicenseFile(count);

									cout << endl << "许可证使用情况更新，当前使用情况如下：";
									ShowLicenseInfo();
								}
								else if (state == 2)
									str = " 重新登录失败 因为您因长时间没有发起重新登录请求，请在收到该消息后重试";
								else 
									str = " 您当前使用的用户名与之前使用的不匹配，请先退出上次的登录";
							}
							//重新输入，校验序列号，用户名和密码
							else if (recev_info.size() == 5)
							{
								//获取该客户端崩溃前的ID
								int lastID = StrToNum(recev_info[4]);

								printf("客户端 %d 发送重新登录请求\n", socnum[i]);

								//对用户发来的崩溃前的ID进行校验

								//state为0表示允许客户端重新登录上次登录过的序列号，
								//为1表示使用的序列号，用户名，密码和之前的不匹配
								//为2表示客户端已经被服务器踢下去
								int state = 1;
								int count = -1;//如果允许登录，记录是哪一个序列号被使用

								//先检查发送过来的ID是否已经因长时间
								//没有发送重新登录请求而被服务器踢下去
								int find_flag = 0;//没有找到为0，找到为1
								for (int j = 0;j < LastLogedID.size();j++)
									if (LastLogedID[j] == lastID)
									{
										find_flag = 1;
										break;
									}
								if (find_flag == 0)state = 2;
								else
								{
									//检查该客户端重新登录发送的ID和序列号，用户名，密码是否匹配
									for (int j = 0;j < SerialNumber.size();j++)
									{
										for (int a = 0;a < UserID[j].size();a++)
										{
											if (UserID[j][a] == lastID && Usernames[j] == recev_info[2]
												&& Password[j] == recev_info[3]
												&& SerialNumber[j]==recev_info[1])
											{
												count = j;
												state = 0;
												UserID[j][a] = socnum[i];//更改登录ID
												//将LastLogedID里的该ID清除掉
												for (int b = 0;b < LastLogedID.size();b++)
												{
													if (LastLogedID[b] == lastID)
														LastLogedID.erase(LastLogedID.begin() + b);
												}
												break;
											}
										}
										if (count != -1)break;
									}
								}

								if (state == 0)
								{
									str = " 已重新登录";

									//对文件进行更新
									UpdateLicenseFile(count);

									cout << endl << "许可证使用情况更新，当前使用情况如下：";
									ShowLicenseInfo();
								}
								else if (state == 2)
									str = " 重新登录失败 因为您因长时间没有发起重新登录请求，请在收到该消息后重试";
								else
									str = " 您当前使用的序列号或用户名与之前使用的不匹配，请先退出上次的登录";
							}
							else str = " 出现未知错误，请重试";
						}
						
						//将消息发送给发来消息的客户端
						char client[1024];
						sprintf(client, "客户端 %d", socnum[i]);
						strcat(client, str.c_str());
						
						send(socnum[i], client, sizeof(client) - 1, 0);//如果
					}
				}
			}
			break;
		}
		}

		if (LastLogedID.size() == 0)TimeFlag = 0;
		if (TimeFlag == 1)
		{
			End = clock();
			double endtime = (double)(End - Start) / CLOCKS_PER_SEC;
			//超过了时间限定，把上次登录过但没有申请重新登录的用户踢下线
			if (endtime > MaxTime)
			{
				for (int p = 0;p < LastLogedID.size();)
				{
					int ser_count = -1;
					int ID_count = -1;
					for (int a = 0;a < SerialNumber.size();a++)
					{
						for (int b = 0;b < UserID[a].size();b++)
						{
							if (UserID[a][b] == LastLogedID[p])
							{
								ser_count = a;
								ID_count = b;
								break;
							}
						}
						if (ser_count == a)break;
					}

					//先将LastLogedID里的该ID清除掉
					LastLogedID.erase(LastLogedID.begin() + p);

					//退出登录
					CurrentNum[ser_count]--;
					UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);

					//对文件进行更新
					UpdateLicenseFile(ser_count);
				}

				cout << endl << "已将服务器崩溃前登录过并且重启后长时间没有申请重新登录的用户踢下线" << endl;

				cout << endl << "许可证的使用情况更新，当前使用情况如下：";
				ShowLicenseInfo();
			}
		}
	}
}

//将输入的信息根据空格进行分割
vector<string> server::split(char *str)
{
	vector<string> elems;
	const char *delim = " ";
	char *s = strtok(str, delim);
	while (s != NULL) {
		elems.push_back(s);
		s = strtok(NULL, delim);
	}
	return elems;
}