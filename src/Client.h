#pragma once
#include <iostream>
class Client
{
public:
	Client();
	~Client();

	std::string sendrecieve(const std::string& s);
	bool recieveDLL(char* buffer);
private:
	int m_sock = 0;
	bool reconnect();
	std::string encrypy(const std::string& input);
	std::string decrypt(const std::string& input);
	void setupEncryption();
	bool connected = false;
};

extern std::unique_ptr<Client> m_Client;