#include "Client.h"
#include <sys/types.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
//#include "aes.hpp"
//#pragma comment(lib,"deps/tiny-AES/tiny-aes.lib")
#ifdef _DEBUG
#pragma comment(lib,"deps/cryptopp/cryptlib.lib")
#else
#pragma comment(lib,"deps/cryptopp/cryptlibRel.lib")
#endif // _DEBUG

#include "modes.h"
#include "aes.h"
#include "filters.h"
#include <iomanip>
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

std::string Client::sendrecieve(std::string text) {

	if (m_sock != INVALID_SOCKET) {
		char buf[4024];
		const char* encryptedString = encrypy(text);

		int sendresult = send(m_sock, encryptedString, strlen(encryptedString) + 1, 0);
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

const char * Client::encrypy(const std::string& input) {

	/* tiny-aes example [MESSY AF] */

	//struct AES_ctx ctx;
	//uint8_t testkey[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
	//				  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
	//uint8_t testiv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	//uint8_t in[]  = { "TEST           "};//has to be a multiple of 16 lol

	//AES_init_ctx_iv(&ctx, testkey, testiv);
	//AES_CBC_encrypt_buffer(&ctx, in, 16);

	//DEBUGLOG(in);

	/* cryptopp example [Bit Better] */
	using namespace CryptoPP;
	
	byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];
	memset(key, 0x01, CryptoPP::AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x01, CryptoPP::AES::BLOCKSIZE);
	
	std::string result;
	
	CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(result));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
	stfEncryptor.MessageEnd();
	
	DEBUGLOG(result);

	return result.c_str();
}


const char* Client::decrypt(const std::string& input) {

	using namespace CryptoPP;

	byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];
	memset(key, 0x01, CryptoPP::AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x01, CryptoPP::AES::BLOCKSIZE);

	AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
	std::string newResult;
	StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(newResult));
	stfDecryptor.Put(reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
	stfDecryptor.MessageEnd();

	DEBUGLOG(newResult);

	return newResult.c_str();
}

std::unique_ptr<Client> m_Client;