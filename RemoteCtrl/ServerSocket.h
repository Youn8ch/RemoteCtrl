#pragma once
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"

class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pdata, size_t& nsize) {
		size_t i = 0;
		for (; i < nsize; i++)
		{
			if (*(WORD*)(pdata+i)==0xFEFF)
			{
				sHead = *(WORD*)(pdata + i);
				i += 2;
				break;
			}
		}
		if (i + 8 > nsize)
		{	
			nsize = 0;
			return;
			// �����ݿ��ܲ�ȫ�����ͷδ���ܵ�
		}
		nLength = *(DWORD*)(pdata + i); i += 4;
		if (nLength +i>nsize)
		{
			nsize = 0; return;
			// �����ݲ�ȫ
		}
		sCmd = *(WORD*)(pdata + i); i += 2;
		if (nLength >4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pdata + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pdata + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum = sSum)
		{
			nsize = i; // head 2 length 4 data
			return;
		}
		nsize = 0;

	}
	CPacket operator=(const CPacket& pack) {
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
	~CPacket(){}

public:
	WORD sHead;	// �̶�λ FE FF
	DWORD nLength; // ������ �ӿ������ʼ������У�����
	WORD sCmd; // ��������
	std::string strData;
	WORD sSum; // ��У��
};


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
		m_sock = socket(PF_INET, SOCK_STREAM, 0); // TODO : У��
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

#define BUFFER_SIZE 4096

	int DealCommand() {
		if (m_client==-1) return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer+index, BUFFER_SIZE -index, 0);
			if (len <= 0)
			{
				return -1;
			}
			m_packet = CPacket((const BYTE * )buffer, index);
			if (index >0)
			{
				memmove(buffer, buffer + index, BUFFER_SIZE - index);
				index = 0;
				return m_packet.sCmd;
			}
		}
	}

	bool Send(const char* pdata, int size) {
		if (m_client == -1) return false;
		return send(m_client, pdata, size, 0) > 0;
	}

private:
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;
private:
	CServerSocket operator = (const CServerSocket&) {}
	CServerSocket(const CServerSocket&) {}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		m_sock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			LOGE("��ʼ�����绷��ʧ��");
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