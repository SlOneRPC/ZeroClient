#include "Menu.h"
#include <iostream>
#define COLOUR(x) x/255 
#define CENTER(width) ((ImGui::GetWindowSize().x - width) * 0.5f)
#define DEBUGLOG(msg) std::cout << msg << std::endl;

Menu::Menu() {
	//setup colours setup
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] =  ImVec4(COLOUR(30.0f), COLOUR(30.0f), COLOUR(30.0f), 1.f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(COLOUR(50.0f), COLOUR(50.0f), COLOUR(50.0f), 1.f);
	style.Colors[ImGuiCol_Border] = ImVec4(COLOUR(0.0f), COLOUR(0.0f), COLOUR(0.0f), 1.f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(COLOUR(50.0f), COLOUR(50.0f), COLOUR(50.0f), 1.f);
	style.Colors[ImGuiCol_Button] = ImVec4(COLOUR(5.0f), COLOUR(116.0f), COLOUR(203.0f), 1.f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(COLOUR(5.0f), COLOUR(116.0f), COLOUR(203.0f), 1.f);
	style.Colors[ImGuiCol_Header] = ImVec4(COLOUR(5.0f), COLOUR(116.0f), COLOUR(203.0f), 1.f);
	//setup window
	style.WindowRounding = 0.0f;
	style.FrameRounding = 2.f;
}

void Menu::mainMenu() {
	ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2{ ZER0_WIDTH,ZER0_HEIGHT }, ImGuiCond_Once);

	if (ImGui::Begin("\n", &AppOpen,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollbar|
		ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (!loggedIn)
			login(loggedIn);
		else
			cheats();
	}
	ImGui::End();
}

void Menu::login(bool& loggedIn)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	{
		ImGui::SetCursorPosX(CENTER(image_size.x));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 60));
		{
			if (my_texture) {
				ImGui::Image((void*)my_texture, image_size);
			}
		}
		ImGui::PopStyleVar();

		ImGui::SetCursorPosX(70);
		ImGui::Text("Username: ");

		ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 5);
		ImGui::InputText("", usernameBuf, sizeof(usernameBuf));

		ImGui::SetCursorPosX(70);
		ImGui::Text("Password: ");

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 20));
		{
			ImGui::PushID(999);
			{
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 5);
				ImGui::InputText("", passwordBuf, sizeof(passwordBuf), ImGuiInputTextFlags_Password);
			}
			ImGui::PopID();


			static bool rem = false;
			ImGui::SetCursorPosX(70);

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1);
			{
				ImGui::PushFont(smallFont);
				ImGui::Checkbox(" Remember Me", &rem);
				ImGui::PopFont();
			}
			ImGui::PopStyleVar();

			ImGui::SetCursorPosX(CENTER(120));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15);
			{
				if (ImGui::Button("Login", ImVec2(120, 40))) {
					loggedIn = true;
					DEBUGLOG("User logged in");
				}
			}
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
	}
	ImGui::PopStyleVar();
}

void Menu::cheats() {
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	{
		ImGui::SetCursorPosX(CENTER(image_size.x));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 60));
		{
			if (my_texture) {
				ImGui::Image((void*)my_texture, image_size);
			}
		}
		ImGui::PopStyleVar();

		static int currentCheat = 0;
		const char* items[] = { "Grand Theft Auto 5" , "CSGO" };
		ImGui::SetCursorPosX(68);
		ImGui::ListBox("", &currentCheat, items, IM_ARRAYSIZE(items),5);

		ImGui::SetCursorPosX(CENTER(120));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15);
		{
			if (ImGui::Button("Launch", ImVec2(120, 40))) {
				//inject
				DEBUGLOG("Inject..");
			}
		}
		ImGui::PopStyleVar();

		ImGui::PushFont(smallFont);
		ImGui::SetCursorPosX(CENTER(ImGui::CalcTextSize("Expires in 10 days").x));
		ImGui::Text("Expires in 10 days");
		ImGui::PopFont();
	}
	ImGui::PopStyleVar();
}