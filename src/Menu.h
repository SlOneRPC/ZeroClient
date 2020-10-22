#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include "XorStr.h"
#include <dinput.h>
#include <tchar.h>
#include "Client.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#define ZER0_WIDTH 350
#define ZER0_HEIGHT 430

enum state {
	loading,
	login,
	cheats
};

struct subscription {
	std::string name;
	std::string lenght;
};

class Menu
{
public:
	Menu();

	void mainMenu();
	bool AppOpen = true;
	ID3D11ShaderResourceView* my_texture = NULL;
	ImVec2 image_size;
	ImFont* smallFont;
	int state = state::loading;
	bool loginError = false;
	std::string loadingType = _xor_("Loading..");
	std::string errormsg = _xor_("Invalid credentials");
	std::vector<subscription> subscriptions = {};
	void doLogin();
private:
	int currentCheat = 0;
	char usernameBuf[120] = {};
	char passwordBuf[120] = {};
	void login(int& login);
	void cheats();
	void loading();
};
extern std::unique_ptr<Menu> m_Menu;