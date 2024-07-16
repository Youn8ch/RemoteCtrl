#pragma once
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"
class CServerSocket
{

public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool Initsocket() {
		m_sock = socket(PF_INET, SOCK_STREAM, 0); // TODO : 校验
		if (m_sock ==-1)
		{
			return false;
		}
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(8554);

		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			return false;
		}// TODO

		if (listen(m_sock, 1) == -1) {
			return false;
		}
		return true;
	}
	
	bool AcceptClient() {
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client==-1)
		{
			return false;
		}
		return true;
	}

	int DealCommand() {
		if (m_client==-1) return false;
		char buffer[1024] = "";
		while (true)
		{
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0)
			{
				return -1;
			}
			// TODO deal
		}
	}

	bool Send(const char* pdata, int size) {
		if (m_client == -1) return false;
		return send(m_client, pdata, size, 0) > 0;
	}

private:
	SOCKET m_sock;
	SOCKET m_client;

private:
	CServerSocket operator = (const CServerSocket&) {}
	CServerSocket(const CServerSocket&) {}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		m_sock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			LOGE("初始化网络环境失败");
		}
	}
	~CServerSocket() {
		closesocket(m_sock);
		WSACleanup();
	}

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	static CServerSocket* m_instance;

	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	class CHelper
	{
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

extern CServerSocket server;