#include <Core/thirdParty/ImGUI/imguiIncludes.h>

#include "ImguiD12.h"

namespace CPR::GFX::D12
{
	ImguiD12::ImguiD12()
	{
		// Setup ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.FontGlobalScale = 1.f;
		//io.ConfigDockingWithShift = true;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		//ImGui_ImplWin32_Init(sc->GetHWND());
	}
	ImguiD12::~ImguiD12()
	{
	}
	void ImguiD12::BeginFrame()
	{
	}
	void ImguiD12::EndFrame()
	{
	}
	void ImguiD12::Render()
	{
	}
}
