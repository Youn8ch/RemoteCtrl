// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"
// #pragma comment(lib, "Ws2_32.lib")

int main()
{

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

