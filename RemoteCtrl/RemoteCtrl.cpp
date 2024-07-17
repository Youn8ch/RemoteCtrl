// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"
#include <direct.h>

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
	Dump((BYTE*)pack.getData(), pack.getSize());
	return 0;
}

int main()
{
	MakeDriverInfo();

	CServerSocket* pServer = CServerSocket::getInstance();
	int count = 0;
	if (pServer->Initsocket() == false) {
		LOGE("> Net init failed! <");
		exit(0);
	}
	LOGI("> Net Created! <");
	while (pServer!=NULL)
	{
		if (pServer->AcceptClient() == false)
		{
			if (count++ >= 3)
			{
				LOGE("> Retry num = %d ,exit! <",count);
				exit(0);
			}
			LOGE("> Accept Client failed! Retry <");
		}
		int ret = pServer->DealCommand();
		// TODO
	}


}

