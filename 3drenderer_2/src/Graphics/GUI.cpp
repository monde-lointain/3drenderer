#include "GUI.h"

#include "../Logger/Logger.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_sdlrenderer.h>
#include <SDL.h>
#include <tracy/tracy/Tracy.hpp>

bool GUI::init(SDL_Window* window, SDL_Renderer* renderer)
{
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    if (!ImGui_ImplSDLRenderer_Init(renderer))
    {
        return false;
    }
    return true;
}

void GUI::process_input(SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
    ImGuiIO& io = ImGui::GetIO();
    int mouse_x, mouse_y;
    const int buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
    io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
    io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
}

void GUI::render()
{
    ZoneScoped; // for tracy

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Handling logging
    
    // Camera log window
    ImGuiWindowFlags log_window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
	if (ImGui::Begin("Camera Log", nullptr, log_window_flags))
    {
        std::vector<LogEntry>& camera_log = Logger::messages[LOG_CATEGORY_CAMERA];
        print_log_messages(camera_log);
    }
    ImGui::End();

    // Clip coordinates log window
    if (ImGui::Begin("Clip coordinates", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        std::vector<LogEntry>& clip_log = Logger::messages[LOG_CATEGORY_CLIPPING];
        print_log_messages(clip_log);
    }
    ImGui::End();

    //// Performance counters log window
    //if (ImGui::Begin("Performance Counters", nullptr, ImGuiWindowFlags_NoCollapse))
    //{
    //    std::vector<LogEntry>& perf_log = Logger::messages[LOG_CATEGORY_PERF_COUNTER];
    //    print_log_messages(perf_log);
    //}
    //ImGui::End();

    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    // Clear all of the logs in the logger
    Logger::reset();
}

void GUI::print_log_messages(std::vector<LogEntry>& log)
{
    for (const LogEntry& entry : log)
    {
        ImGui::Text(entry.message.c_str());
    }
}

void GUI::destroy()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
