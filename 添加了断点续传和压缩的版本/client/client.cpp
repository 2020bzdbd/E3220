#define _CRT_SECURE_NO_WARNINGS

#include"Client.h"
#include"unzip.h"
#include"zip.h"

#define ZIPSIZE 100000

SOCKET Client::s_server;
//SOCKET Client::s_accept;
//服务端地址客户端地址
SOCKADDR_IN Client::server_addr;
//SOCKADDR_IN Client::accept_addr;
string Client::file_path;
int Client::mother_file_index_from_start = 0;
int Client::mother_file_length = 0;
int Client::p_counter = 0;

string server_ip;
int port;

ofstream fout;
ofstream fout1;
ofstream fout2;
ofstream fout3;
ifstream fin;
ifstream fin1;

void Client::initialization()
{
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号	
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0)
	{
		cout << "初始化套接字库失败！" << endl;
	}
	else
	{
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2)
	{
		cout << "套接字库版本号不符！" << endl;		WSACleanup();
	}
	else
	{
		cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息
}

bool Client::file_transmision(string file_name)
{
	int recv_len;
	char recv_buf[100];
	int readlen;
	//char buf[1025];
	char buf[HEAD_SIZE+BUFFER_SIZE] = { 0 };
	int send_len;
	ifstream in_file(file_name, ios::in | ios::binary);
	//in_file >> noskipws;
	if (!in_file.is_open())
	{
		cout << "找不到文件" << file_name << endl;
		in_file.close();
		return false;
	}
	while (!in_file.eof())
	{
		//cout << 1;
		buf[0] = '0'; buf[1] = '0'; buf[2] = '1';
		char* infor_p = buf + HEAD_SIZE;
		in_file.read(infor_p, BUFFER_SIZE);
		//cout << buf << endl;
		readlen = in_file.gcount();
		if (readlen == 1024)
		{
			buf[3] = '1'; buf[4] = '0'; buf[5] = '2'; buf[6] = '4';
		}
		else
		{
			set_file_length(buf, readlen);
		}
		
		
		send_len = send(s_server, buf, readlen + HEAD_SIZE, 0);
		if (send_len < 0)
		{
			cout << "传输失败" << endl;
			in_file.close();
			return false;
		}
		/*p_counter++;
		if (p_counter >= 10240)
		{
			p_counter = 0;
			Sleep(200);
		}*/
	}
	//cout << "文件 " << file_name << " 传输完成"<<endl;
	in_file.close();
	//buf[0] = '0'; buf[1] = '0'; buf[2] = '2';
	//Sleep(1);
	//Sleep(0.5);
	send_len = send(s_server, "0020021file_transmission_end", 21+HEAD_SIZE, 0);//25
	if (send_len < 0)
	{
		cout << "发送文件传输结束指令失败" << endl;
		return false;
	}
	cout << endl;
	return true;
}

bool Client::sent_create_file_messege(string file_path)
{
	file_path = file_path.substr(mother_file_index_from_start);

	int send_len;
	
	string file_len = to_string(file_path.length());
	while (file_len.length() < 4)
	{
		file_len = "0" + file_len;
	}
	string sent_str = "003" + file_len + file_path;
	if (send_len = send(s_server, sent_str.c_str(), sent_str.length(), 0))
	{
		cout << "发送创建文件夹成功" << file_path << endl;
	}
	else
	{
		cout << "发送创建文件夹指令失败" << file_path << endl;
		return false;
	}
	return true;
}

bool Client::sent_file_name(string file_name)
{
	file_name = file_name.substr(mother_file_index_from_start);

	int send_len;
	string file_len = to_string(file_name.length());
	while (file_len.length() < 4)
	{
		file_len = "0" + file_len;
	}
	string sent_str = "004" + file_len + file_name;
	if (send_len = send(s_server, sent_str.c_str(), sent_str.length(), 0))//200
	{
		cout << "发送创建文件成功" << file_name << endl;
	}
	else
	{
		cout << "发送创建文件夹指令失败" << file_name << endl;
		return false;
	}
	return true;
}

bool Client::sent_end_transmission()
{
	int send_len;
	string temp = "all_transmission_end";
	if (send_len = send(s_server, ("0000020" + temp).c_str(), ("0000020" + temp).length(), 0))
	{
		cout << "已经传输结束传输指令" << endl;
	}
	else
	{
		cout << "传输结束指令出错了" << endl;
		return false;
	}
	return true;
}

void Client::getAllFiles(string path, vector<string>& files, vector<int>& is_file_packet, int hierarchy) {
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
				cout << "源文件是文件夹" << endl;
				files.push_back(p.assign(path));
				is_file_packet.push_back(1);
			}
			else
			{
				cout << "源文件就是一个文件" << endl;
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
					getAllFiles(p.assign(path).append("/").append(fileinfo.name), files, is_file_packet, hierarchy + 1);

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

void Client::__connect()
{
	int recv_len;
	char recv_buf[100];
	initialization();
	//填充服务端信息

	fin.open("record.txt");
	int branch = 0;//用于判断程序上次传输是否中断,若为0，则没中断，为1中断
	if (!fin.is_open())branch = 0;//若该文件打开失败，则说明为第一次使用该程序
	else
	{
		string str;
		getline(fin, str);
		if (str == "Finished")branch = 0;
		else branch = 1;
		fin.close();
	}

	//如果程序没有中断，新建或者重写record文件
	if (branch == 0)
	{
		fout.open("record.txt");
		fout << "Finished";
		fout.close();
	}

	cout << "请输入【文件地址】 【ip】 【端口号】中间以一个空格隔开" << endl;
	cin >> file_path >> server_ip >> port;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip.c_str()); //192.168.20.1     127.0.0.1
	server_addr.sin_port = htons(port);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	while (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		cout << "服务器连接失败！正在尝试重连！" << endl;
		WSACleanup();
		Sleep(2000);//等待2秒后重新连接
		initialization();
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip.c_str()); //192.168.20.1     127.0.0.1
		server_addr.sin_port = htons(port);
		//创建套接字
		s_server = socket(AF_INET, SOCK_STREAM, 0);
	}
	
	cout << "服务器连接成功！" << endl;
	
	if (recv_len = recv(s_server, recv_buf, 100, 0) < 0)
	{
		cout << "出错了,没有正常收到回复" << endl;
	}

	//发送,接收数据


	/*if (file_path[file_path.length() - 1] != '/' && file_path[file_path.length() - 1] != '\\')
	{
		file_path += '/';
	}*/
	if (_access(file_path.c_str(), 0) == -1)	//如果文件夹不存在
	{
		cout << "不存在该文件夹或者文件" << endl;
		exit(-1);
	}
}

//传输过程中服务器断开进行重连
void Client::re__connect()
{
	int recv_len;
	char recv_buf[100];

	cout << "服务端和网络可能断开，正在尝试重连" << endl;
	WSACleanup();
	Sleep(2000);
	initialization();
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip.c_str()); //192.168.20.1     127.0.0.1
	server_addr.sin_port = htons(port);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	while (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "服务器连接失败！正在尝试重连！" << endl;
		WSACleanup();
		Sleep(2000);
		initialization();
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip.c_str()); //192.168.20.1     127.0.0.1
		server_addr.sin_port = htons(port);
		//创建套接字
		s_server = socket(AF_INET, SOCK_STREAM, 0);
	}

	cout << "服务器重新连接成功！" << endl;

	if (recv_len = recv(s_server, recv_buf, 100, 0) < 0)
	{
		cout << "出错了,没有正常收到回复" << endl;
	}
}

//获得文件最后一次的修改时间
string GetFileTime(string filepath)
{
	struct stat buf;
	stat(filepath.c_str(), &buf);
	time_t time1 = buf.st_mtime;
	tm *ptm = localtime(&time1);

	char str[100];
	sprintf(str, "%d/%02d/%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1,
		ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	string modify_time = str;
	return modify_time;
}


void Client::__trans(int &flag)
{
	/*
	该文件记录传输情况，若上次传输正常结束，内容为Finished，仅一行
	若没有正常传输，第一行为Unfinished，第二行为上次传输结束时正在传输的文件的序号，有两行
	*/
	string FilePath = "record.txt";
	const char* file = FilePath.c_str();
	FILE * fr;
	char path[100];
	sprintf_s(path, 100, file, (1));
	fopen_s(&fr, path, "rb");
	char  temp[1];
	vector<char>file_content;
	int count = 1;
	if (fr != NULL)
	{
		while (!feof(fr)) {
			temp[0] = fgetc(fr);
			file_content.push_back(temp[0]);
		}
		fclose(fr);
		file_content.pop_back();

		for (int i = 0;i < file_content.size();i++)
		{
			if (file_content[i] == '\n')count++;//统计换行符，计算有多少行
		}
	}

	clock_t start_t, ends_t;
	vector<string> files;
	vector<int> is_file_packet;
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	char recv_buf[100];
	int file_name_head = 0, file_name_end = file_path.length() - 1;
	string mother_file;
	int mother_file_length;

	start_t = clock();
	for (int j = file_path.length() - 1; j >= 0; j--)
	{
		if (file_path[j] != '\\' && file_path[j] != '/')
		{
			file_name_end = j;
			break;
		}
	}
	for (int j = file_name_end; j >= 0; j--)
	{
		if (file_path[j] == '\\' || file_path[j] == '/')
		{
			file_name_head = j + 1;
			break;
		}
	}
	mother_file = file_path.substr(file_name_head, file_name_end - file_name_head + 1);//获取mother_file,哪怕你填的是绝对路径,也可以正确发送相对路径
	cout << "mother file is " << mother_file << endl;
	mother_file_index_from_start = file_name_head;
	mother_file_length = file_name_end - file_name_head + 1;


	getAllFiles(file_path, files, is_file_packet, 0);

	int diff = 0;//记录文件是否出现差异，若为0，则表明没有差异，否则有差异，需要全部重传
	fin.open("sended.txt");//该文件记录已经上次传输时已经传输好的文件的文件名
	fin1.open("time.txt");//该文件记录已传输文件的最后修改时间
	vector<string>sendedPath;
	vector<string>times;
	if (count != 1)
	{
		if (!fin.is_open() || !fin1.is_open())diff = 0;//若该文件打开失败，则说明为第一次使用该程序
		else
		{
			while (!fin.eof())
			{
				string s;
				getline(fin, s);
				sendedPath.push_back(s);
			}
			sendedPath.pop_back();
			while (!fin1.eof())
			{
				string s;
				getline(fin1, s);
				times.push_back(s);
			}
			times.pop_back();

			int struct_diff = 0;//如果文件结构相同，则为0，不相同为-1
			//开始比较已传输的文件目录结构
			for (int i = 0;i < sendedPath.size() && i < files.size();i++)
			{
				if (sendedPath[i] != files[i])
				{
					diff = -1;
					struct_diff = -1;
				}
				//如果重连后，要传输的文件结构或者文件内容发生了变化，则需要重传
			}

			//若文件目录结构没变，则比较文件的最后修改时间来判断文件的内容是否发生了变化
			if (struct_diff == 0)
			{
				//在已传输的文件夹和文件中选出文件
				vector<string>sendedfiles;
				for (int i = 0;i < sendedPath.size();i++)
					if (!is_file_packet[i])sendedfiles.push_back(sendedPath[i]);
				for (int i = 0;i < sendedfiles.size();i++)
				{
					string modify_time = GetFileTime(sendedfiles[i]);
					if (modify_time != times[i])diff = -1;
				}
			}
		}
	}
	fin.close();
	fin1.close();
	//上次传输正常结束或需要重传，清空sended文件和time文件
	if (count == 1 || diff == -1)
	{
		fout1.open("sended.txt");
		fout3.open("time.txt");
	}
	else
	{
		fout1.open("sended.txt", ios::app);
		fout3.open("time.txt", ios::app);
	}
	fout1.close();
	fout3.close();

	cout << files.size() << " " << is_file_packet.size() << endl;

	int num = 0;//传输时从哪一个文件开始传

	//上次传输被中断的情况
	if (count != 1)
	{
		//如果检测到文件结构发生变化，向服务端发送删除已经传输好的文件的指令，再进行重传
		if (diff == -1)
		{
			cout << endl << "检测到上次传输中断，且已传输部分发生变动，将删除已经传输的文件，并重传所有文件" << endl;

			//向服务端发送删除已经删除的文件指令
			int send_len;
			string file_len = "0000";
			if (send_len = send(s_server, ("111"+file_len).c_str(), ("111" + file_len).length(), 0))
			{
				cout << "发送删除文件夹指令成功" << endl;
			}
			else
			{
				cout << "发送删除文件夹指令失败" << endl;
				re__connect();
				return;
			}

			cout << endl << "已删除上次已经传输的文件" << endl;
			cout << endl << "开始重传" << endl << endl;
		}
		else
		{
			//文件结构没有发生变化，则从上次中断处开始传输
			ifstream fin("record.txt");
			string instruct;
			getline(fin, instruct);
			getline(fin, instruct);
			stringstream ss(instruct);
			ss >> num;
			fin.close();
			cout << "检测到上次传输被中断，且已传输部分没有变动，将从" << files[num + 1] << "开始传输" << endl << endl;
			num++;
		}
	}

	for (int i = 0; i < files.size(); i++)
	{
		if (is_file_packet[i])
		{
			if (!sent_create_file_messege(files[i]))
			{
				re__connect();
				return;
			}
			//记录传输情况
			fout.open("record.txt");
			fout << "Unfinished" << endl;
			fout << i;
			fout.close();
			
			//记录已传输的文件名
			fout1.open("sended.txt", ios::app);
			fout1 << files[i] << endl;
			fout1.close();
		}
		else
		{
			//先判断文件大小，如果文件大小小于100KB，则将该文件压缩传输，否则不压缩
			ifstream finsize(files[i]);
			finsize.seekg(0, ios::end);
			int size = finsize.tellg();
			finsize.close();

			
			//进行压缩
			if (size <= ZIPSIZE && size > 0)
			{
				cout << "该文件小于100KB，进行压缩" << endl;
				string str1 = files[i];
				string str2 = files[i];

				int pos = 0;
				pos = files[i].rfind("/");

				int pos1 = 0;
				pos1 = files[i].rfind('.');

				str1 = str1.erase(pos + 1, files[i].length());
				str2 = str2.substr(pos + 1, pos1);

				pos1 = str2.find(".");

				string str3 = str2;

				str2 = str2.substr(0, pos1);

				string zippath = str1 + str2 + ".zip";

				string despath = files[i];
				pos = files[i].rfind("/");
				despath = despath.substr(pos + 1, files[i].length());

				HZIP hz = CreateZip(zippath.c_str(), 0);
				ZipAdd(hz, despath.c_str(), files[i].c_str());
				CloseZip(hz);
				sent_file_name(zippath);
				
				file_transmision(zippath);
				remove(zippath.c_str());
				//发送解压指令
				
				string file_len = to_string(str3.length());
				while (file_len.length() < 4)
				{
					file_len = "0" + file_len;
				}

				int send_len;
				if (send_len = send(s_server, ("005" + file_len + str3).c_str(), ("005" + file_len + str3).length(), 0))
				{
					cout << "发送解压指令成功" << endl;
				}
				else
				{
					cout << "发送解压指令失败" << endl;
				}
				

			}
			else
			{

				if (!sent_file_name(files[i]))
				{
					re__connect();
					return;
				}
				
				if (!file_transmision(files[i]))
				{
					re__connect();
					return;
				}
			}
			
			//记录传输情况
			fout.open("record.txt");
			fout << "Unfinished" << endl;
			fout << i;
			fout.close();

			//记录已经传输的文件名
			fout1.open("sended.txt", ios::app);
			fout1 << files[i] << endl;
			fout1.close();

			//记录该文件的最后修改时间
			fout3.open("time.txt", ios::app);
			string modify_time = GetFileTime(files[i]);
			fout3 << modify_time << endl;
			fout3.close();

		}
	}
	//Sleep(1);
	//Sleep(0.5);
	Sleep(20);
	if (!sent_end_transmission())
	{
		re__connect();
		return;
	}
	cout << "文件传输完成" << endl;

	//正常传输完毕，记录
	fout.open("record.txt");
	fout << "Finished";
	fout.close();

	ends_t = clock();
	int pass_t = ends_t - start_t;
	cout << "经过时间" << (double)pass_t / 1000.0 << endl;

	flag = 1;//传输结束
}

void Client::__exit()
{
	//关闭套接字
	closesocket(s_server);	//closesocket(s_server);
	//释放DLL资源
	WSACleanup();
}


void Client::set_file_length(char* ch, int len)
{
	/*string file_len = to_string(len);
	while (file_len.length() < 4)
	{
		file_len = "0" + file_len;
	}
	for (int i = 0; i < 4; i++)
	{
		ch[3 + i] = file_len[i];
	}*/
	int temp_len = len;
	for (int i = 6; i >= 3; i--)
	{
		ch[i] = (temp_len % 10)+48;
		temp_len /= 10;
	}
}
