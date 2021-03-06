#pragma once
#include <iostream>
class Client
{
public:
	Client();
	~Client();

	std::string sendrecieve(const std::string& s);
	char* recieveDLL();
	bool isSetup() { return setup; };
private:
	int m_sock = 0;
	bool reconnect();
	std::string encrypt(const std::string& input);
	std::string decrypt(const std::string& input);
	char* decyptBuffer(char* buffer, int length);
	void setupEncryption();
	bool connected = false;
	bool setup = false;
};

extern std::unique_ptr<Client> m_Client;