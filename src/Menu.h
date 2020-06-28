#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include "Client.h"
#include <thread>

#define ZER0_WIDTH 350
#define ZER0_HEIGHT 460

enum state {
	loading,
	login,
	cheats
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
private:
	char usernameBuf[120] = {};
	char passwordBuf[120] = {};

	void login(int& login);
	void cheats();
	void loading();
};