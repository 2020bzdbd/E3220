#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<fstream>

using namespace std;

//LZW编码
void encode(string Str, vector<int>& result)
{
	//创建默认字典，即与256个ASCII字符对应
	map<string, int> dictionary;
	int dic_size = 256;
	for (int i = 0;i < 256;i++)dictionary[string(1, i)] = i;

	char C;
	string P;
	//开始编码压缩
	for (int i = 0;i < Str.size();i++)
	{
		C = Str[i];
		if (dictionary.count(P + C))P += C;
		else
		{
			result.push_back(dictionary[P]);
			dictionary[P + C] = dic_size++;
			P = C;
		}
	}
	if(!P.empty())result.push_back(dictionary[P]);
}

//LZW解码
string decode(vector<int>& result)
{
	//创建默认字典
	map<int, string> dictionary;
	int dic_size = 256;
	for (int i = 0;i < 256;i++)dictionary[i] = string(1, i);
	string S = "";
	string entry = "";
	string sourse = "";

	//利用编码字符，还原出压缩字典，并进行解压缩
	for (int i = 0;i < result.size();i++)
	{
		int k = result[i];
		if (dictionary.count(k))
			entry = dictionary[k];
		else if (k == dic_size)
			entry = S + S[0];

		sourse += entry;

		if(S!="")dictionary[dic_size++] = S + entry[0];
		S = entry;
	}
	return sourse;
}

int main()
{
	string FilePath;
	vector<int>Text;
	cout << "请输入要压缩的文件地址：";
	cin >> FilePath;

	const char* file = FilePath.c_str();
	FILE * fr;
	char path[100];
	sprintf_s(path, 100, file, (1));
	fopen_s(&fr, path, "rb");
	char  temp[1];
	int byte = 0;
	vector<char>Byte;
	while (!feof(fr)) {
		temp[0] = fgetc(fr);

		Byte.push_back(temp[0]);

		unsigned char Temp = (unsigned char)temp[0];
		Text.push_back((int)Temp);
		byte++;
	}
	Text.pop_back();

	Byte.pop_back();

	string Str;
	Str.resize(Text.size());
	for (int i = 0;i < Byte.size();i++)
		Str[i] = Byte[i];

	//将压缩后的数据串存储到int型的容器中
	vector<int> result;
	encode(Str, result);
	cout << "压缩前字节数：" <<Text.size() << " " << "压缩后字节数：" << result.size();

	ofstream fout1("3.txt", ios::out);
	for (int i = 0;i < result.size();i++)fout1 << result[i];
	fout1.close();


	string sourse;
	sourse = decode(result);

	cout << endl << "解压完成" << endl;

	int count = 0;
	for (int i = 0;i < sourse.size();i++)
		if (Str[i] != sourse[i])count++;

	cout << endl << "共有" << count << "个字符出错" << endl;

	ofstream fout("2.pptx", ios::out | ios::binary);
	for (int i = 0;i < sourse.size();i++)
	{
		char Temp = (char)sourse[i];
		fout << Temp;
	}
	fout.close();
	system("pause");
	return 0;
}