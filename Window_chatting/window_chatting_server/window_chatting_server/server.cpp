#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32")
#include <windows.h>
#include <list>
#include <thread>
#include <iterator>
#include <mutex>
#include <vector>

/*
������ �ϴ� ���
1. ���� �⺻ ���� �� 
2. Ŭ���̾�Ʈ ���� �� ���� list�� �ֱ� �� 
3. Ŭ���̾�Ʈ �޽��� �޴� �� thread�� �и� �� 
4. Ŭ���̾�Ʈ���Լ� ���� �޽��� �ٸ� Ŭ���̾�Ʈ���� ���� ��
5. CTRL_C ������ ��� ���� ��
6. HB ���� 
*/


SOCKET g_listenSocket = 0;
std::list<SOCKET> g_clntList;
std::mutex g_m;  // mutex ��ü


BOOL AddClnt(SOCKET clntSock) {
	g_m.lock();
	g_clntList.push_back(clntSock);
	std::cout << "ok" << std::endl;
	g_m.unlock();
	return TRUE;
}

void SendMessageToClient(char* szBuffer) {
	std::list<SOCKET>::iterator it;
	g_m.lock();
	for (it = g_clntList.begin(); it != g_clntList.end(); ++it) {
		::send(*it, szBuffer, strlen(szBuffer) + 1, 0);
	}
	g_m.unlock();
}

void ReceiveMessageFromClient(SOCKET clntSock) {
	char szBuffer[128];
	std::cout << "Ŭ���̾�Ʈ�� �����߽��ϴ�." << std::endl;
	while (::recv(clntSock, szBuffer, sizeof(szBuffer), 0) > 0) {
		std::cout << szBuffer << std::endl;
		SendMessageToClient(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
	}
	
	puts("Client ������ �������ϴ�.");
	g_m.lock();
	g_clntList.remove(clntSock);
	g_m.unlock();


	::closesocket(clntSock);

}

BOOL WINAPI IsItControlC(DWORD fdwCtrlType) {
	if (fdwCtrlType == CTRL_C_EVENT) {
		std::list<SOCKET>::iterator it;
		// ����� ��� Ŭ���̾�Ʈ �� ���� ���� �ݰ� ����
		::shutdown(g_listenSocket, SD_BOTH);
		g_m.lock();
		for (it = g_clntList.begin(); it != g_clntList.end(); ++it) {
			closesocket(*it);
		}
		g_clntList.clear();
		g_m.unlock();;
		puts("��� Ŭ���̾�Ʈ ������ �����߽��ϴ�.");
		::Sleep(100);
		::closesocket(g_listenSocket);

		// ���� ����
		::WSACleanup();
		exit(0);
	}
	return TRUE;
};

int main(int argc, char* argv[]) {
	WSADATA wsa = { 0 };
	::WSAStartup(MAKEWORD(2, 2), &wsa);

	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)IsItControlC, TRUE) == FALSE) {
		puts("ERROR: Ctrl+C ó���� ��� ����");
	}

	g_listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (g_listenSocket == INVALID_SOCKET) {
		puts("ERROR: ���� ���� ����");
		return 0;
	}

	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9999);
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	
	
	if (::bind(g_listenSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		puts("ERROR : ���� ���Ͽ� IP�� PORT ��ȣ ���ε� ����");
		return 0;
	}

	if (::listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		puts("ERROR : ���� ���� ���� ����");
		return 0;
	}
	
	puts("*** ä�ü����� �����մϴ�. ***");

	std::vector<std::thread> threads;
	// Ŭ���̾�Ʈ ���� ó��
	SOCKET clntSock = 0;
	char szBuffer[128];
	SOCKADDR_IN clntAddr;
	int clntAddrLen = sizeof(clntAddr);
	
	while ((clntSock = ::accept(g_listenSocket, (SOCKADDR*)&clntAddr, &clntAddrLen)) != INVALID_SOCKET) {

		if (AddClnt(clntSock) == FALSE) {
			puts("���� ���ῡ �����߽��ϴ�.	");
			break;
		}


		std::thread th(ReceiveMessageFromClient, clntSock);
		threads.push_back(std::move(th));
	}
	for (auto& thread : threads) {
		thread.join();
	}

	puts("*** ä�ü����� �����մϴ�. ***");
	return 0;

}