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
#include "hex.h"
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
			DEBUGLOG("Cannot connect to the server! Is the server open?");
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
	//we need to check if we got an inital connection to the server
	if (m_sock != INVALID_SOCKET && connected) {
		char buf[4024];
		std::string encryptedString = encrypy(text);
		int sendresult = send(m_sock, encryptedString.c_str(), encryptedString.size() + 1, 0);
		if (sendresult != SOCKET_ERROR) {
			ZeroMemory(buf, 4024);
			int bytesRec = recv(m_sock, buf, 4024, 0);
			if (bytesRec > 0) {
				DEBUGLOG("SERVER : " << std::string(buf, 0, bytesRec));
				return decrypt(std::string(buf, 0, bytesRec));
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

std::string Client::encrypy(const std::string& input) {
	/* Not using tiny-aes because cryptopp has auto padding */
	using namespace CryptoPP;
	
	byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
	memset(key, 0x01, AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x01, AES::BLOCKSIZE);
	
	std::string result;
	
	AES::Encryption aesEncryption(key, AES::DEFAULT_KEYLENGTH);
	CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	//encrypt the string using PKCS padding
	StringSource(input, true,
		new StreamTransformationFilter(cbcEncryption, new StringSink(result),
			StreamTransformationFilter::PKCS_PADDING));
	
	std::string encoded;
	//convert the encrypted string to hex
	StringSource(result, true,
		new HexEncoder(
			new StringSink(encoded)
		) 
	); 

	DEBUGLOG("Encrypted Hex: " + encoded);

	return encoded;
}


std::string Client::decrypt(const std::string& input) {

	using namespace CryptoPP;

	byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];
	memset(key, 0x01, CryptoPP::AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x01, CryptoPP::AES::BLOCKSIZE);

	AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
	
	std::string decoded;
	//we need to decode the hex before decrypting
	StringSource(input, true,
		new HexDecoder(
			new StringSink(decoded)
		) 
	); 

	std::string result;
	//decrypt the string with PKCS padding
	StringSource(decoded, true,
		new StreamTransformationFilter(cbcDecryption, new StringSink(result),
			StreamTransformationFilter::PKCS_PADDING));

	DEBUGLOG("Decryped String: " + result);

	return result;
}

std::unique_ptr<Client> m_Client;