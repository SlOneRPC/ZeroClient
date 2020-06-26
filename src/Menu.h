#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

#define ZER0_WIDTH 350
#define ZER0_HEIGHT 420

class Menu
{
public:
	Menu();

	void mainMenu();
private:

};
