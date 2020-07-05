#include "Menu.h"
#include <iostream>
#include <algorithm> 
#define COLOUR(x) x/255 
#define CENTER(width) ((ImGui::GetWindowSize().x - width) * 0.5f)
#ifdef _DEBUG
#define DEBUGLOG(msg) std::cout << msg << std::endl;   
#else
#define DEBUGLOG(msg)
#endif 

/*
	Constructor for inital menu setup
*/
Menu::Menu() {
	//setup colours
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
		if (state == 0) {
			loading();
		}
		else if (state == 1) {
			login(state);
		}
		else if (state == 2) {
			cheats();
		}
	}
	ImGui::End();
}

void Menu::login(int& loggedIn)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	{
		ImGui::SetCursorPosX(CENTER(image_size.x));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 35));
		{
			if (my_texture) {
				ImGui::Image((void*)my_texture, image_size);
			}
		}
		ImGui::PopStyleVar();

		ImGui::SetCursorPosX(70);
		ImGui::Text(_xor_("Username: ").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 5);
		ImGui::InputText("", usernameBuf, sizeof(usernameBuf));

		ImGui::SetCursorPosX(70);
		ImGui::Text(_xor_("Password: ").c_str());

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
				ImGui::Checkbox(_xor_(" Remember Me").c_str(), &rem);
				ImGui::PopFont();
			}
			ImGui::PopStyleVar();

			static bool loginError = false;
			ImGui::SetCursorPosX(CENTER(120));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15);
			{
				if (ImGui::Button(_xor_("Login").c_str(), ImVec2(120, 40))) {
					loggedIn = state::cheats;
					DEBUGLOG("User logged in");
				}
			}
			ImGui::PopStyleVar();

			if (loginError) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(COLOUR(255.f), COLOUR(5.f), COLOUR(5.f), 1.f));
				ImGui::PushFont(smallFont);
				ImGui::SetCursorPosX(CENTER(ImGui::CalcTextSize(_xor_("Invalid credentials").c_str()).x));
				ImGui::Text(_xor_("Invalid credentials").c_str());
				ImGui::PopFont();
				ImGui::PopStyleColor();
			}
		}
		ImGui::PopStyleVar();
	}
	ImGui::PopStyleVar();
}

void Menu::cheats() {
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	{
		ImGui::SetCursorPosX(CENTER(image_size.x));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 35));
		{
			if (my_texture) {
				ImGui::Image((void*)my_texture, image_size);
			}
		}
		ImGui::PopStyleVar();

		static int currentCheat = 0;
		const char* items[] = { _xor_("Grand Theft Auto 5").c_str() , _xor_("CSGO").c_str() };
		ImGui::SetCursorPosX(68);
		ImGui::ListBox("", &currentCheat, items, IM_ARRAYSIZE(items),5);

		ImGui::SetCursorPosX(CENTER(120));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15);
		{
			if (ImGui::Button(_xor_("Launch").c_str(), ImVec2(120, 40))) {
				//inject
				DEBUGLOG("Inject..");
			}
		}
		ImGui::PopStyleVar();

		ImGui::PushFont(smallFont);
		ImGui::SetCursorPosX(CENTER(ImGui::CalcTextSize(_xor_("Expires in 10 days").c_str()).x));
		ImGui::Text(_xor_("Expires in 10 days").c_str());
		ImGui::PopFont();
	}
	ImGui::PopStyleVar();
}

void Menu::loading() {
	ImGui::SetCursorPosX(CENTER(image_size.x));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 35));
	{
		if (my_texture) {
			ImGui::Image((void*)my_texture, image_size);
		}

		ImGui::SetCursorPosX(CENTER(45)-22.5f);
		ImGui::LoadingIndicatorCircle("t", 45, ImVec4(COLOUR(5.0f), COLOUR(116.0f), COLOUR(203.0f), 1.f), ImVec4(COLOUR(30.0f), COLOUR(30.0f), COLOUR(30.0f), 1.f), 14, 7.0f);

		ImGui::SetCursorPosX(CENTER(ImGui::CalcTextSize(loadingType.c_str()).x));
		ImGui::Text(loadingType.c_str());
	}
	ImGui::PopStyleVar();
}

void ImGui::LoadingIndicatorCircle(const char* label, const float indicator_radius,
	const ImVec4& main_color, const ImVec4& backdrop_color,
	const int circle_count, const float speed) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems) {
		return;
	}

	ImGuiContext& g = *GImGui;
	const ImGuiID id = window->GetID(label);

	const ImGuiStyle& style = g.Style;
	const ImVec2 pos = window->DC.CursorPos;
	const float circle_radius = indicator_radius / 10.0f;
	const ImRect bb(pos, ImVec2(pos.x + indicator_radius * 2.0f,
		pos.y + indicator_radius * 2.0f));
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id)) {
		return;
	}
	const float t = g.Time;
	const auto degree_offset = 2.0f * IM_PI / circle_count;
	for (int i = 0; i < circle_count; ++i) {
		const auto x = indicator_radius * std::sin(degree_offset * i);
		const auto y = indicator_radius * std::cos(degree_offset * i);
		const auto growth = max(0.0f, std::sin(t * speed - i * degree_offset));
		ImVec4 color;
		color.x = main_color.x * growth + backdrop_color.x * (1.0f - growth);
		color.y = main_color.y * growth + backdrop_color.y * (1.0f - growth);
		color.z = main_color.z * growth + backdrop_color.z * (1.0f - growth);
		color.w = 1.0f;
		window->DrawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x,
			pos.y + indicator_radius - y),
			circle_radius + growth * circle_radius,
			GetColorU32(color));
	}
}

std::unique_ptr<Menu> m_Menu;