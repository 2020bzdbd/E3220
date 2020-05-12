#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "public.h"

vector<string> SerialNumber;//�洢���к�
vector<string> Usernames;//�洢�û���
vector<string> Password;//�洢����
vector<int> Maximum;//ÿ�����кŵ����ʹ������
vector<int> CurrentNum;//�����кŵ�ǰʹ������
vector<vector<int>> UserID;//�洢ʹ�ø����кŵĿͻ��˵�ID
vector<bool> Is_Fisrt_Use;//��¼�����к��Ƿ��һ�α��û�ʹ��
vector<int> LastLogedID;//���ڴ洢����������ǰ��¼����ID
server ser;

clock_t Start, End;//���ڼ�ʱ
int TimeFlag = 0;//Ϊ0�ǲ���Ҫ��ʱ��Ϊ1ʱ��Ҫ��ʱ

#define MaxTime 100//����ϴε�¼�����û��ڱ�����100����û���������µ�¼����������
#define FILEPATH "license"

string sourcePath = FILEPATH;

//�ַ����Ƿ�ȫ����������ɣ�����ǣ�����true�����򷵻�false
bool isNum(string str)
{
	stringstream sin(str);
	double d;
	char c;
	if (!(sin >> d))return false;
	if (sin >> c)return false;
	return true;
}

//���ַ���ת��Ϊ����
int StrToNum(string str)
{
	int num;
	stringstream ss(str);
	ss >> num;
	return num;
}

//�����֤��Ϣ����
void GetLicenseInfo(server ser)
{
	//��ȡ��Ϣǰ�Ƚ����е���������Ϣ���
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
	
	if (files.size() == 1)cout << "û���κ����֤" << endl;
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
				last_username = str;//��ȡ�û���
				//��ȡ���û���������
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
				SerialNumber.push_back(str);//�������к�
				Usernames.push_back(last_username);//�����û���
				Password.push_back(last_user_password);//��������
				i++;
				ifstream fin;
				fin.open(files[i], ios::in);
				string s;
				int num;
				//������ʹ������
				getline(fin, s);
				num = StrToNum(s);
				Maximum.push_back(num);
				string current;
				getline(fin, current);//�ڶ��м�¼�Ƿ��ǵ�һ��ʹ��
				if (current == "TRUE")Is_Fisrt_Use.push_back(true);
				else Is_Fisrt_Use.push_back(false);
				getline(fin, current);//�����м�¼��ǰʹ������
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

//������֤��������Ϣ
void ShowLicenseInfo()
{	
	if (SerialNumber.size() > 0)
	{	
		cout << endl << "{";
		for (int i = 0;i < SerialNumber.size();i++)
		{
			cout << endl << "���к�" << SerialNumber[i] << "�����Ϣ���£�" << endl;
			cout << "�����֤�û���" << Usernames[i];
			cout << "   �û����룺" << Password[i] << endl;
			cout << "�Ƿ�ʹ�ù���";
			if (Is_Fisrt_Use[i] == true)cout << "��" << endl;
			else cout << "��" << endl;
			cout << "�������ޣ�" << Maximum[i] << " ��ǰʹ��������" << CurrentNum[i] << endl;
			if (CurrentNum[i] > 0)
			{
				cout << "ʹ�ø����֤�Ŀͻ���IDΪ��";
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

//�������֤�ļ�
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
	//����������ʱ�ͽ����֤��Ϣ����
	GetLicenseInfo(ser);
	cout << endl << "���֤��ǰʹ��������£�";
	ShowLicenseInfo();

	//����������ʱ������������ϴ�û�б������������кŵĵ�ǰʹ������Ӧ��ȫΪ0
	//���ֲ�Ϊ0����������ʾ������������
	//�����������ü�ʱ��������ϴε�¼�����û�����һ��ʱ��û���������µ�¼��������
	for (int i = 0;i < CurrentNum.size();i++)
	{
		if (CurrentNum[i] > 0)
		{
			Start = clock();
			TimeFlag = 1;//����Ϊ��Ҫ��ʱ
			for (int a = 0;a < SerialNumber.size();a++)
				for (int b = 0;b < UserID[a].size();b++)
					LastLogedID.push_back(UserID[a][b]);
			break;
		}
	}

	ser.process();

	return 0;
}

//���캯�������÷������Ϣ
server::server()
{
	string SERVER_IP;//�����IP
	int SERVER_PORT;//����˶˿�
	cout << "������������IP��ַ�ͷ���˶˿ںţ��м��ÿո����" << endl;
	cin >> SERVER_IP >> SERVER_PORT;
	listener = 0;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = SERVER_PORT;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());//���ַ�������ת��uint32_t
}

//��ʼ�����������ܴ��������׽��֣��󶨶˿ڣ������м���
void server::init()
{
	int   Ret;
	WSADATA   wsaData;// ���ڳ�ʼ���׽��ֻ���

	//��ʼ��WinSock����
	//ʧ��ʱ
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("WSAStartup()   failed   with   error   %d\n", Ret);
		WSACleanup();
	}

	//��ʼ��WinSock�����ɹ�
	listener = socket(AF_INET, SOCK_STREAM, 0);//����ipv4,TCP����
	if (listener == -1) 
	{ 
		printf("Error at socket(): %ld\n", WSAGetLastError()); 
		perror("����ʧ��"); 
		exit(1); 
	}
	printf("�����ɹ�\n");

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
	socnum.push_back(listener);//�������׽��ּ���
}

void server::GetLicense(string path, vector<string>& files, vector<int>& is_file_packet, int hierarchy)
{
	//�ļ����
	long hFile = 0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;  //�����õ��ļ���Ϣ��ȡ�ṹ
	string p;  //string�������˼��һ����ֵ����:assign()���кܶ����ذ汾
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
			if ((fileinfo.attrib & _A_SUBDIR)) {  //�Ƚ��ļ������Ƿ����ļ���
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

		} while (_findnext(hFile, &fileinfo) == 0);  //Ѱ����һ�����ɹ�����0������-1
		_findclose(hFile);
	}
}

void server::process()
{
	int mount = 0;
	fd_set fds;
	FD_ZERO(&fds);//��fds����

	//��ʼ���������˽�������
	init();

	printf("���ڵȴ��ͻ�����Ϣ......\n");
	while (1)
	{
		mount = socnum.size();
		//��fdsÿ�ζ����¸�ֵ
		for (int i = 0; i < mount; i++)
		{
			FD_SET(socnum[i], &fds);
		}

		struct timeval timeout = { 1,0 };//����ÿ��1��selectһ��
		//�����ֻ���ö���������д
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
			//�������е�ÿһ���׽��ֶ���ʣ����׽��ֽ��бȽϵõ���ǰ������
			for (int i = 0; i < mount; i++)
			{
				//�������֤��Ϣ
				GetLicenseInfo(ser);
				//��������׽��ֿɶ�����Ϣ���ͽ�������
				if (i == 0 && FD_ISSET(socnum[i], &fds))
				{
					struct sockaddr_in client_address;
					socklen_t client_addrLength = sizeof(struct sockaddr_in);
					//����һ���û����׽���
					int clientfd = accept(listener, (struct sockaddr*)&client_address, &client_addrLength);
					//����û�������������ʾ��Ϣ����֪ͨ�û����ӳɹ�
					socnum.push_back(clientfd);
					cout << "�ͻ��� " << clientfd << "�ɹ����ӱ�������" << endl;
					char ID[1024];
					sprintf(ID, "��� ���IDΪ %d\n�������¼��Ϣ ���к�(��һ��ʹ�ò���Ҫ) �û��� ���� ��", clientfd);
					
					//����������ID�����͸��ͻ����ÿͻ���֪���Լ���ID
					send(clientfd, ID, sizeof(ID) - 1, 0);//��ȥ���һ��'/0'
				}
				if (i != 0 && FD_ISSET(socnum[i], &fds))
				{
					char buf[1024];
					memset(buf, '\0', sizeof(buf));
					int size = recv(socnum[i], buf, sizeof(buf) - 1, 0);
					//���ͻ����Ƿ����
					if (size == 0 || size == -1)
					{
						cout << "�ͻ��� " << socnum[i] << " �ѵ���" << endl;

						//�жϵ��ߵĿͻ����Ƿ���ʹ�����к�
						int ser_count = -1;
						int ID_count = -1;//���߾�Ϊ-1��ʾ���ߵĿͻ���û��ʹ���κ����֤
						for (int a=0 ;a<UserID.size();a++)
							for (int b=0 ;b < UserID[a].size();b++)
							{
								//���ߵĿͻ�������ʹ�����к�
								if (socnum[i] == UserID[a][b])
								{
									ser_count = a;
									ID_count = b;
									break;
								}
							}
						if (ser_count == -1 && ID_count == -1)
							cout << "�ÿͻ���û��ʹ���κ����֤�����֤ʹ���������" << endl;
						else
						{
							CurrentNum[ser_count]--;

							UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);
							cout << "���ߵĿͻ���ʹ�������֤" << SerialNumber[ser_count];
							cout << endl << "���֤��ʹ��������£���ǰʹ��������£�";
							ShowLicenseInfo();

							//���ļ����и���
							UpdateLicenseFile(ser_count);
						}

						closesocket(socnum[i]);//�ر�����׽���
						FD_CLR(socnum[i], &fds);//���б���ɾ��
						socnum.erase(socnum.begin() + i);//��vector������ɾ��
						mount--;
					}
					//��û�е���
					else
					{
						vector<string> recev_info = split(buf);
						string str;//���͸��ͻ��˵���Ϣ
						//������û�б���
						if (recev_info[0] == "UNLOGED")
						{
							//����˷����˳���¼ָ��
							if (recev_info.size() == 2)
							{
								printf("�ͻ��� %d �����˳���¼����\n", socnum[i]);

								//���ÿͻ����Ƿ��¼�����û�е�¼�;ܾ������󣬷��������˳�
								int logor = 0;//Ϊ0��ʾû�е�¼��Ϊ1��ʾ��¼
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
								if (logor == 0)str = " ��û�е�¼";
								else
								{
									str = " ���ѳɹ��˳���¼";
									cout << "�ͻ��� " << socnum[i] << "�˳���¼" << endl;
									//�˳���¼
									CurrentNum[ser_count]--;
									UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);
									cout << endl << "���֤��ʹ��������£���ǰʹ��������£�";
									ShowLicenseInfo();
									//���ļ����и���
									UpdateLicenseFile(ser_count);
								}
							}
							//���û������������У��
							else if (recev_info.size() == 3)
							{
								printf("�ͻ��� %d ���͵�¼����\n", socnum[i]);

								//Ϊ0��ʾ����ͻ��˵�¼��Ϊ1��ʾʹ�������ﵽ���ޣ�
								//Ϊ2��ʾ�����ڸ����кŻ��û���
								//Ϊ3��ʾ�ظ���¼
								//Ϊ4��ʾ���ڸ����кŻ��û������û��������кţ����벻ƥ��
								//Ϊ5��ʾ���û���һ�ε�¼���A����Ҫ�������кŽ�����֤
								int state = 2;
								int count = 0;//��������¼����¼����һ�����кű�ʹ��

								for (int a = 0;a < Usernames.size();a++)
								{
									//���ÿͻ����Ƿ��Ѿ���¼
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
										//����û���������������ƥ�䣬�����Ƿ��ظ���¼
										for (int b = 0;b < UserID[a].size();b++)
										{
											if (UserID[a][b] == socnum[i])
											{
												state = 3;
												break;
											}
										}
										if (state == 3)break;
										//���û���ظ���¼���ͼ������֤ʹ�������Ƿ�ﵽ����
										if (CurrentNum[a] < Maximum[a])
										{
											//����Ƿ�Ϊ��һ��ʹ��
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
									str = " �����¼ ��Ҫ�˳���¼��������quit��";
									//�������֤ʹ�����
									CurrentNum[count]++;
									UserID[count].push_back(socnum[i]);

									//���ļ����и���
									UpdateLicenseFile(count);

									cout << "�û���������У��ɹ��������¼" << endl;
									cout << endl << "���֤ʹ��������£���ǰʹ��������£�";
									ShowLicenseInfo();
								}
								else if (state == 1)str = " ���û���ӵ�е��������֤��ʹ�������ﵽ���ޣ����Ժ��ٵ�¼����ȥ����һ�����֤";
								else if (state == 3)str = " ���Ѿ���¼,�����ظ���¼";
								else if (state == 2)str = " �����ڸ��û��������������룺";
								else if (state == 5)str = " ���û�Ŀǰ�п�������֤Ϊ�״�ʹ�ã���Ҫ��֤���кţ����������룺";
								else str = " �û��������벻ƥ�䣬���������룺";
							}
							//�����кţ��û������������У��
							else if (recev_info.size() == 4)
							{
								printf("�ͻ��� %d ���͵�¼����\n", socnum[i]);

								//Ϊ0��ʾ����ͻ��˵�¼��Ϊ1��ʾʹ�������ﵽ���ޣ�
								//Ϊ2��ʾ�����ڸ����кŻ��û���
								//Ϊ3��ʾ�ظ���¼
								//Ϊ4��ʾ���ڸ����кŻ��û������û��������кţ����벻ƥ��
								int state = 2;
								int count = 0;//��������¼����¼����һ�����кű�ʹ��
								for (int a = 0;a < Usernames.size();a++)
								{
									//���ÿͻ����Ƿ��Ѿ���¼
									for (int b = 0;b < UserID[a].size();b++)
									{
										if (UserID[a][b] == socnum[i])
										{
											state = 3;
											break;
										}
									}
									if (state == 3)break;

									//���кţ��û��������벻ƥ��
									if (SerialNumber[a] == recev_info[1] && Usernames[a] != recev_info[2])
										state = 4;
									if (Password[a] != recev_info[3] && Usernames[a] == recev_info[2])
										state = 4;
									if (SerialNumber[a] == recev_info[1] && Password[a] != recev_info[3])
										state = 4;

									//�û��������������кţ�����ƥ��
									if (Usernames[a] == recev_info[2] && Password[a] == recev_info[3]
										&& SerialNumber[a] == recev_info[1])
									{
										//����Ƿ��ظ���¼
										for (int b = 0;b < UserID[a].size();b++)
										{
											if (UserID[a][b] == socnum[i])
											{
												state = 3;
												break;
											}
										}
										if (state == 3)break;
										//���û���ظ���¼���ͼ������֤ʹ�������Ƿ�ﵽ����
										if (CurrentNum[a] < Maximum[a])
										{
											state = 0;
											count = a;
											//�������к�����Ϊ�ǵ�һ��ʹ��
											Is_Fisrt_Use[a] = false;
											break;
										}
										else state = 1;
									}
								}

								if (state == 0)
								{
									str = " �����¼ ��Ҫ�˳���¼��������quit��";
									//�������֤ʹ�����
									CurrentNum[count]++;
									UserID[count].push_back(socnum[i]);

									//���ļ����и���
									UpdateLicenseFile(count);

									cout << "���кţ��û���������У��ɹ��������¼" << endl;
									cout << endl << "���֤ʹ��������£���ǰʹ��������£�";
									ShowLicenseInfo();
								}
								else if (state == 1)str = " ���û���ӵ�е��������֤��ʹ�������ﵽ���ޣ����Ժ��ٵ�¼����ȥ����һ�����֤";
								else if (state == 3)str = " ���Ѿ���¼,�����ظ���¼";
								else if (state == 2)str = " �����ڸ��û��������кţ����������룺";
								else str = " �û��������벻ƥ�䣬���������룺";
							}
							else
							{
								str = " ����δ֪����������";
							}
						}
						//������֮ǰ������
						else
						{
							//�˳�֮ǰ�ĵ�¼
							if (recev_info.size() == 3)
							{
								//��ȡ�ÿͻ��˱���ǰ��ID
								int lastID = StrToNum(recev_info[2]);

								printf("�ͻ��� %d �����˳���¼����\n", socnum[i]);

								//�ȼ�鷢�͹�����ID�Ƿ��Ѿ���ʱ��
								//û�з������µ�¼�����������������ȥ
								int find_flag = 0;//û���ҵ�Ϊ0���ҵ�Ϊ1
								for (int j = 0;j < LastLogedID.size();j++)
									if (LastLogedID[j] == lastID)
									{
										find_flag = 1;
										break;
									}
								if (find_flag == 0)
								{
									str = " �˳���¼ʧ�� �����յ�������Ϣ�����ԣ�";
									str = str + "��Ϊ����������������ʱ��û�з��������ѱ������������ߣ�";
								}
								else
								{
									//���ÿͻ����Ƿ��¼�����û�е�¼�;ܾ������󣬷��������˳�
									int logor = 0;//Ϊ0��ʾû�е�¼��Ϊ1��ʾ��¼
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
									if (logor == 0)str = " ����������ǰ��û�е�¼�����˳���¼";
									else
									{
										//�Ƚ�LastLogedID��ĸ�ID�����
										for (int a = 0;a < LastLogedID.size();a++)
										{
											if (LastLogedID[a] == lastID)
												LastLogedID.erase(LastLogedID.begin() + a);
										}

										str = " ���ѳɹ��˳���¼";
										cout << "�ͻ��� " << socnum[i] << "�˳���¼" << endl;
										//�˳���¼
										CurrentNum[ser_count]--;
										UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);
										cout << endl << "���֤��ʹ��������£���ǰʹ��������£�";
										ShowLicenseInfo();

										//���ļ����и���
										UpdateLicenseFile(ser_count);
									}
								}
							}
							//���µ�¼��У���û���������
							else if (recev_info.size() == 4)
							{
								//��ȡ�ÿͻ��˱���ǰ��ID
								int lastID = StrToNum(recev_info[3]);

								printf("�ͻ��� %d �������µ�¼����\n", socnum[i]);
								//���û������ı���ǰ��ID����У��

								//stateΪ0��ʾ����ͻ������µ�¼�ϴε�¼�������кţ�
								//Ϊ1��ʾ�û�����������֮ǰʹ�õĲ�ƥ��
								//Ϊ2��ʾ�ͻ����Ѿ�������������ȥ
								int state = 1;
								int count = -1;//��������¼����¼����һ�����кű�ʹ��

								//�ȼ�鷢�͹�����ID�Ƿ��Ѿ���ʱ��
								//û�з������µ�¼�����������������ȥ
								int find_flag = 0;//û���ҵ�Ϊ0���ҵ�Ϊ1
								for (int j = 0;j < LastLogedID.size();j++)
									if (LastLogedID[j] == lastID)
									{
										find_flag = 1;
										break;
									}
								if (find_flag == 0)state = 2;
								else
								{
									//���ÿͻ������µ�¼���͵�ID���û����������Ƿ�ƥ��
									for (int j = 0;j < SerialNumber.size();j++)
									{
										for (int a = 0;a < UserID[j].size();a++)
										{
											if (UserID[j][a] == lastID && Usernames[j] == recev_info[1]
												&& Password[j] == recev_info[2])
											{
												count = j;
												state = 0;
												UserID[j][a] = socnum[i];//���ĵ�¼ID
												//��LastLogedID��ĸ�ID�����
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
									str = " �����µ�¼";

									//���ļ����и���
									UpdateLicenseFile(count);

									cout << endl << "���֤ʹ��������£���ǰʹ��������£�";
									ShowLicenseInfo();
								}
								else if (state == 2)
									str = " ���µ�¼ʧ�� ��Ϊ����ʱ��û�з������µ�¼���������յ�����Ϣ������";
								else 
									str = " ����ǰʹ�õ��û�����֮ǰʹ�õĲ�ƥ�䣬�����˳��ϴεĵ�¼";
							}
							//�������룬У�����кţ��û���������
							else if (recev_info.size() == 5)
							{
								//��ȡ�ÿͻ��˱���ǰ��ID
								int lastID = StrToNum(recev_info[4]);

								printf("�ͻ��� %d �������µ�¼����\n", socnum[i]);

								//���û������ı���ǰ��ID����У��

								//stateΪ0��ʾ����ͻ������µ�¼�ϴε�¼�������кţ�
								//Ϊ1��ʾʹ�õ����кţ��û����������֮ǰ�Ĳ�ƥ��
								//Ϊ2��ʾ�ͻ����Ѿ�������������ȥ
								int state = 1;
								int count = -1;//��������¼����¼����һ�����кű�ʹ��

								//�ȼ�鷢�͹�����ID�Ƿ��Ѿ���ʱ��
								//û�з������µ�¼�����������������ȥ
								int find_flag = 0;//û���ҵ�Ϊ0���ҵ�Ϊ1
								for (int j = 0;j < LastLogedID.size();j++)
									if (LastLogedID[j] == lastID)
									{
										find_flag = 1;
										break;
									}
								if (find_flag == 0)state = 2;
								else
								{
									//���ÿͻ������µ�¼���͵�ID�����кţ��û����������Ƿ�ƥ��
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
												UserID[j][a] = socnum[i];//���ĵ�¼ID
												//��LastLogedID��ĸ�ID�����
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
									str = " �����µ�¼";

									//���ļ����и���
									UpdateLicenseFile(count);

									cout << endl << "���֤ʹ��������£���ǰʹ��������£�";
									ShowLicenseInfo();
								}
								else if (state == 2)
									str = " ���µ�¼ʧ�� ��Ϊ����ʱ��û�з������µ�¼���������յ�����Ϣ������";
								else
									str = " ����ǰʹ�õ����кŻ��û�����֮ǰʹ�õĲ�ƥ�䣬�����˳��ϴεĵ�¼";
							}
							else str = " ����δ֪����������";
						}
						
						//����Ϣ���͸�������Ϣ�Ŀͻ���
						char client[1024];
						sprintf(client, "�ͻ��� %d", socnum[i]);
						strcat(client, str.c_str());
						
						send(socnum[i], client, sizeof(client) - 1, 0);//���
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
			//������ʱ���޶������ϴε�¼����û���������µ�¼���û�������
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

					//�Ƚ�LastLogedID��ĸ�ID�����
					LastLogedID.erase(LastLogedID.begin() + p);

					//�˳���¼
					CurrentNum[ser_count]--;
					UserID[ser_count].erase(UserID[ser_count].begin() + ID_count);

					//���ļ����и���
					UpdateLicenseFile(ser_count);
				}

				cout << endl << "�ѽ�����������ǰ��¼������������ʱ��û���������µ�¼���û�������" << endl;

				cout << endl << "���֤��ʹ��������£���ǰʹ��������£�";
				ShowLicenseInfo();
			}
		}
	}
}

//���������Ϣ���ݿո���зָ�
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