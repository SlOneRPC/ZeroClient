#pragma once
#include <iostream>
class Client
{
public:
	Client();
	~Client();

	std::string sendrecieve(std::string s);
private:
	int m_sock = 0;
	bool reconnect();
	const char* encrypy(const std::string& input);
	const char* decrypt(const std::string& input);
};

extern std::unique_ptr<Client> m_Client;