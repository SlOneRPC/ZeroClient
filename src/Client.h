#pragma once
#include <iostream>
class Client
{
public:
	Client();
	~Client();

	std::string sendrecieve(const std::string& s);
	bool recieveDLL(char* buffer);
	bool isSetup() { return setup; };
private:
	int m_sock = 0;
	bool reconnect();
	std::string encrypt(const std::string& input);
	std::string decrypt(const std::string& input);
	void setupEncryption();
	bool connected = false;
	bool setup = false;
};

extern std::unique_ptr<Client> m_Client;