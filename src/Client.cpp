#include "Client.h"
#include <sys/types.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#include "aes.hpp"
#pragma comment(lib,"deps/tiny-AES/tiny-aes.lib")
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
		encrypy();
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

void Client::encrypy() {
	struct AES_ctx ctx;
	uint8_t testkey[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
					  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
	uint8_t testiv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	uint8_t in[]  = { "TEST           "};//has to be a multiple of 16 lol

	AES_init_ctx_iv(&ctx, testkey, testiv);
	AES_CBC_encrypt_buffer(&ctx, in, 16);

	//DEBUGLOG(in);
}

void Client::decrypt() {

}

std::unique_ptr<Client> m_Client;