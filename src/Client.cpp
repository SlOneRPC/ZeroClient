#include "Client.h"
#include "Common.h"

//sock includes
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#if 1
	//encryption includes
	#include "modes.h"
	#include "aes.h"
	#include "filters.h"
	#include "hex.h"
	#include "osrng.h"
	#include "files.h"
	#include <iomanip>
	#ifdef _DEBUG
	#pragma comment(lib,"deps/cryptopp/cryptlib.lib")
	#else
	#pragma comment(lib,"deps/cryptopp/cryptlibRel.lib")
	#endif // _DEBUG
	using namespace CryptoPP;
#endif // 0
#include "HWInfo.h"

//server
#define PORT 8001
#define SERVERIP "127.0.0.1"
sockaddr_in serv_addr;
byte key[AES::MAX_KEYLENGTH], iv[AES::BLOCKSIZE];
/* 
	Client constructor for setting up winsock and socket connection
*/
Client::Client() {
	WSADATA data; WORD ver = MAKEWORD(2, 2); 
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

	inet_pton(AF_INET, _xor_(SERVERIP).c_str(), &serv_addr.sin_addr);
	connected = reconnect();//open inital socket connection to the server
	setupEncryption();
	std::string auth = _xor_("SUCCESS_AUTHENTICATION");
	setup = connected && sendrecieve(std::to_string(getHWinfo64())) == auth;
}

/*
	Socket destructor for cleanup
*/
Client::~Client() {
	if (m_sock != INVALID_SOCKET) {
		closesocket(m_sock);
		WSACleanup();
	}
}

/*
	Create a new socket connection
*/
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

/*
	Send a encrypted message and recieve a response through the socket
*/
std::string Client::sendrecieve(const std::string& text) {
	//we need to check if we got an inital connection to the server
	if (m_sock != INVALID_SOCKET && connected) {
		char buf[4024];
		std::string encryptedString = encrypt(text);
		int sendresult = send(m_sock, encryptedString.c_str(), encryptedString.size() + 1, 0);
		if (sendresult != SOCKET_ERROR) {
			ZeroMemory(buf, 4024);
			int bytesRec = recv(m_sock, buf, 4024, 0);
			if (bytesRec > 0) {
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
int f = 0;
/*
	Setup encryption system, CBC 256 key and iv
*/
void Client::setupEncryption() {
	//setup public key and iv for initial message
	std::string ENC_KEY = _xor_("XSEZ1ZiXpwonxSLIbwyoOwBnJOX9mM1n"); // public key
	std::string IV = _xor_("byOPz5oNOIGvk1bC"); // public iv
	memcpy(key, ENC_KEY.data(), CryptoPP::AES::MAX_KEYLENGTH);
	memcpy(iv, IV.data(), CryptoPP::AES::BLOCKSIZE);


	std::string aesString = sendrecieve(_xor_("Test"));
	memcpy(key, aesString.substr(0, aesString.find(";")).data(), CryptoPP::AES::MAX_KEYLENGTH);
	memcpy(iv, aesString.substr(aesString.find(";") + 1).data(), CryptoPP::AES::BLOCKSIZE);
}

/*
	Encrypt socket message (hex format)
*/
std::string Client::encrypt(const std::string& input) {

	std::string result;

	AES::Encryption aesEncryption(key, AES::MAX_KEYLENGTH);
	CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);


	//encrypt the string using PKCS padding
	StringSource(input, true,
		new StreamTransformationFilter(cbcEncryption, new StringSink(result),
			StreamTransformationFilter::PKCS_PADDING));


	std::string encoded;

	StringSource(result, true,
		new HexEncoder(
			new StringSink(encoded)
		)
	);

	DEBUGLOG("Encrypted Hex: " + encoded);
	
	return encoded;
}

/*
	Decrypt socket message (hex format)
*/
std::string Client::decrypt(const std::string& input) {

	AES::Decryption aesDecryption(key, AES::MAX_KEYLENGTH);
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

	if(result.size() < 40)
		DEBUGLOG("Decryped returned string: " + result);

	return result;
}

/*
	Decrypt a char array buffer (hex format)
*/
char* Client::decyptBuffer(char* buffer, int length)
{
	std::string ss(buffer, length);
	std::string decryptedString = decrypt(ss);
	ss.clear();

	char* finalBuffer = new char[decryptedString.size()];
	std::copy(decryptedString.begin(), decryptedString.end(), finalBuffer);
	
	decryptedString.clear();
	delete[] buffer;
	return finalBuffer;
}

/*
	Recieve an encrypted DLL through the socket 
	and decrypt with AES CBC
*/
char* Client::recieveDLL() {
	if (m_sock != INVALID_SOCKET && connected) {
		std::string fileSize = sendrecieve(_xor_("REQUEST_DLL"));
		if (fileSize != "") {
			char* buf= new char[BUFSIZ];//buffer for recieving packets
			char* tempbuff = new char[std::stoi(fileSize)];

			int filebytesrec = 0;
			int currentrec = 1;

			while (currentrec > 0) {
				ZeroMemory(buf, BUFSIZ);
				currentrec = recv(m_sock, buf, BUFSIZ, 0);
				
				if (currentrec == 0)
					break;
				
				memcpy(tempbuff + filebytesrec, buf, currentrec);
				filebytesrec += currentrec;
			}
			
			if (filebytesrec > 0) {
				char* buffer = new char[filebytesrec];
				ZeroMemory(buffer, filebytesrec);
				memcpy(buffer, tempbuff, filebytesrec);
				delete[] tempbuff;
				delete[] buf;

				return decyptBuffer(buffer, filebytesrec);
			}
			else
			{
				delete[] tempbuff;
				delete[] buf;
			}
		}

	}
	return nullptr;
}

std::unique_ptr<Client> m_Client;