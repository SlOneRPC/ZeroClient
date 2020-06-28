#include "Client.h"
#include <sys/types.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#define PORT 8001

#ifdef _DEBUG
#define DEBUGLOG(msg) std::cout << msg << std::endl;   
#else
#define DEBUGLOG(msg)
#endif 
sockaddr_in serv_addr;

Client::Client() {
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	INT WSResult = WSAStartup(ver, &data);
	if (WSResult != 0) {
		DEBUGLOG("Cannot create winsock!");
		return;
	}

	m_sock = socket(AF_INET, SOCK_STREAM,0);
	if (m_sock == INVALID_SOCKET) {
		DEBUGLOG("Cannot create socket!");
		WSACleanup();
		return;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

	reconnect();//open inital connection
}

Client::~Client() {
	if (m_sock != INVALID_SOCKET) {
		closesocket(m_sock);
		WSACleanup();
	}
}

bool Client::reconnect() {
	if (m_sock != INVALID_SOCKET) {
		int connResult = connect(m_sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
		if (connResult == SOCKET_ERROR) {
			DEBUGLOG("Cannot connect to the server!");
			closesocket(m_sock);
			WSACleanup();
		}
		else {
			return true;
		}
	}
	else {
		DEBUGLOG("Socket Not initialized!");
	}
	return false;
}

std::string Client::sendrecieve(const char* text) {

	if (m_sock != INVALID_SOCKET) {
		char buf[4024];
		int sendresult = send(m_sock, text, strlen(text) + 1, 0);
		if (sendresult != SOCKET_ERROR) {
			ZeroMemory(buf, 4024);
			int bytesRec = recv(m_sock, buf, 4024, 0);
			if (bytesRec > 0) {
				DEBUGLOG("SERVER : " << std::string(buf, 0, bytesRec));
				return std::string(buf, 0, bytesRec);
			}
		}
		else if (reconnect()) {//sucessfully reconnected
			return sendrecieve(text);//retry
		}
		else {
			DEBUGLOG("Server connection closed!");
		}
	}
	return "";
}

std::unique_ptr<Client> m_Client;