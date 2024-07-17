// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <list>
// #pragma comment(lib, "Ws2_32.lib")

void Dump(BYTE* pdata, size_t length) {
	std::string strout;
	for (size_t i = 0; i < length; i++)
	{
		char buf[8] = "";
		if (i>0)
		{
			strout += " ";
		}
		snprintf(buf, sizeof(buf), "%02X", pdata[i] & 0xFF);
		// printf("pData[%d] in hex: %s\n", i, buf);
		strout += buf;
	}
	strout += "\n";
	std::cout << "Dump:\n" << strout << std::endl;
}

int MakeDriverInfo() {
	std::string result;
	for (int i = 1; i <= 26; i++) {
		if (_chdrive(i) == 0) {
			if (result.size()>0)
			{
				result += ',';
			}
			result += ('A' + i - 1);
		}
	}
	// CServerSocket::getInstance()->Send(CPacket(1,(const BYTE*)result.c_str(),result.size()));
	CPacket pack(1, (const BYTE*)result.c_str(), result.size());
	// Dump((BYTE*)pack.getData(), pack.getSize());
	return 0;
}

typedef struct file_info{
	file_info() {
		IsInvalid = 0;
		IsDirectory = -1;
		HasNext = 0;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid; // 是否有效
	BOOL IsDirectory; // 是否是目录 0否 1是
	BOOL HasNext; // 是否还有后续 0否 1是
	char szFileName[256]; // 文件名
}FILEINFO,*PFILEINFO;

int MakeDirectoryInfo() {
	std::string path;
	// std::list<FILEINFO> ListFileInfo;
	if (CServerSocket::getInstance()->GetFilePath(path) == false)
	{
		LOGE("> Get file path failed <");
		return -1;
	}
	if (_chdir(path.c_str())!=0)
	{
		FILEINFO finfo;
		finfo.IsInvalid = TRUE;
		finfo.IsDirectory = TRUE;
		finfo.HasNext = FALSE;
		memcpy(finfo.szFileName, path.c_str(), path.size());
		// ListFileInfo.push_back(finfo);
		CPacket pack(2, (BYTE*) & finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		LOGE("> No permission to access the directory <");
		return -2;
	}
	_finddata_t fdata;
	int hfind = 0;
	if ((hfind = _findfirst("*", &fdata)) == -1) {
		LOGE("> No file being the path <");
		return -3;
	}

	do
	{
		FILEINFO finfo;
		finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		// ListFileInfo.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
	} while (!_findnext(hfind,&fdata));
	FILEINFO finfo;
	finfo.HasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	CServerSocket::getInstance()->Send(pack);
	return 0;
}

int RunFile() {
	std::string path;
	CServerSocket::getInstance()->GetFilePath(path);
	ShellExecuteA(NULL,NULL,path.c_str(),NULL,NULL,SW_SHOWNORMAL);
	return 0;
}

int DownloadFile() {
	std::string path;
	CServerSocket::getInstance()->GetFilePath(path);
	long long data = 0;
	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, path.c_str(), "rb");
	if (pFile==NULL || err!=0 )
	{
		CPacket pack(4, (BYTE*)&data, 8);
		CServerSocket::getInstance()->Send(pack);
		return -1;
	}
	fseek(pFile, 0, SEEK_END);
	data = _ftelli64(pFile);
	CPacket head(4, (BYTE*)&data, 8);
	fseek(pFile, 0, SEEK_SET);
	char buffer[1024] = "";
	size_t rlen = 0;
	do {
		rlen = fread(buffer, 1, 1024, pFile);
		CPacket pack(4, (BYTE*)buffer, rlen);
		CServerSocket::getInstance()->Send(pack);
	} while (rlen >= 1024);
	CPacket pack(4, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	fclose(pFile);
}

int main()
{

	//CServerSocket* pServer = CServerSocket::getInstance();
	//int count = 0;
	//if (pServer->Initsocket() == false) {
	//	LOGE("> Net init failed! <");
	//	exit(0);
	//}
	//LOGI("> Net Created! <");
	//while (pServer!=NULL)
	//{
	//	if (pServer->AcceptClient() == false)
	//	{
	//		if (count++ >= 3)
	//		{
	//			LOGE("> Retry num = %d ,exit! <",count);
	//			exit(0);
	//		}
	//		LOGE("> Accept Client failed! Retry <");
	//	}
	//	int ret = pServer->DealCommand();
	//	// TODO
	//}
	
	int nCmd = 1;
	switch (nCmd)
	{
	case 1:
		MakeDriverInfo(); // 查看磁盘分区
		break;
	case 2:
		MakeDirectoryInfo(); // 查看指定目录下的文件
		break;
	case 3:
		RunFile(); // 打开文件
		break;
	case 4:
		DownloadFile();
		break;
	default:
		break;
	}

}

