#pragma once
#include <iostream>
class Client
{
public:
	Client();
	~Client();

	std::string sendrecieve(const char* text);
private:
	int m_sock = 0;
	bool reconnect();
	void encrypy();
	void decrypt();
};

extern std::unique_ptr<Client> m_Client;