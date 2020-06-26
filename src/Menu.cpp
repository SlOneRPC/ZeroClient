#include "Menu.h"

#define COLOUR(x) x/255 

Menu::Menu() {
	//setup colours setup
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(COLOUR(4.0f), COLOUR(0.0f), COLOUR(91.0f), 1.f);

	//setup window
	style.WindowRounding = 0.0f;
}

void Menu::mainMenu() {
	static bool _visible = true;

	ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2{ ZER0_WIDTH,ZER0_HEIGHT }, ImGuiCond_Once);

	if (ImGui::Begin("Zer0Skill.net Client", &_visible,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollbar))
	{
		//render menu here
	}
	ImGui::End();
}
