#pragma once
#include<iostream>
#include<winsock.h>
#include<fstream>
#include<sstream>
#include<direct.h>
#include<io.h>
#include<string>
#include<vector>
#include<stdio.h>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define PORT 8307
#define SERVER_IP "192.168.31.212"
#define BUFFER_SIZE 1024
#define HEAD_SIZE 7

#define PROGRAM_END 0
#define NORMAL_FILE_TRANS 1
#define NORMAL_FILE_TRANS_END 2
#define CREATE_FILE_PACKET 3
#define CREATE_FILE 4
