#include "GUI.h"

#include <glm/trigonometric.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_sdlrenderer.h>
#include <tracy/tracy/Tracy.hpp>

#include "../Logger/Logger.h"
#include "../Window/Window.h"
#include "../World/World.h"

#ifdef _MSC_VER // Windows
#include <SDL.h>
#else // Linux
#include <SDL2/SDL.h>
#endif

void GUI::initialize(Window* window_, World* world_)
{
    window = window_;
    world = world_;

    ImGui::CreateContext();

    if (!ImGui_ImplSDL2_InitForSDLRenderer(window->window, window->renderer))
	{
		SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR, 
            window->name,
			"Error initializing ImGui for SDL platform layer.", 
            nullptr
        );

		ImGui::DestroyContext();
		window->destroy();
		exit(1);
	}

    if (!ImGui_ImplSDLRenderer_Init(window->renderer))
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            window->name,
            "Error initializing ImGui for SDL renderer.",
            nullptr
        );

        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        window->destroy();
        exit(1);
    }
}

void GUI::process_input(SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
    ImGuiIO& io = ImGui::GetIO();
    int mouse_x, mouse_y;
    const uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
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
    const ImGuiWindowFlags log_window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
	if (ImGui::Begin("Camera Log", nullptr, log_window_flags))
    {
        std::vector<LogEntry>& camera_log = Logger::messages[LOG_CATEGORY_CAMERA];
        print_log_messages(camera_log);
    }
    ImGui::End();

    // Clip coordinates log window
    if (ImGui::Begin("Clip info", nullptr, log_window_flags))
    {
        std::vector<LogEntry>& clip_log = Logger::messages[LOG_CATEGORY_CLIPPING];
        print_log_messages(clip_log);
    }
    ImGui::End();

    // Light log window
    if (ImGui::Begin("Light", nullptr, log_window_flags))
    {
        std::vector<LogEntry>& light_log = Logger::messages[LOG_CATEGORY_LIGHT];
        print_log_messages(light_log);
    }
    ImGui::End();

    // Performance counters log window
    if (ImGui::Begin("Performance Counters", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        std::vector<LogEntry>& perf_log = Logger::messages[LOG_CATEGORY_PERF_COUNTER];
        print_log_messages(perf_log);
    }
    ImGui::End();

    // Light movement controls
    if (ImGui::Begin("Edit Light", nullptr, log_window_flags))
    {
        // Position
        ImGui::Text("Position");
        ImGui::Spacing();

        ImGui::DragFloat("position x", &world->light.translation.x, 0.01f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("position y", &world->light.translation.y, 0.01f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("position z", &world->light.translation.z, 0.01f, 0.0f, 0.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Rotation
        ImGui::Text("Rotation");
        ImGui::Spacing();

        static float in_pitch = world->light.rotation.pitch;
        static float in_yaw = world->light.rotation.yaw;
        static float in_roll = world->light.rotation.roll;

        ImGui::DragFloat("rotation x", &in_pitch, 0.5f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("rotation y", &in_yaw, 0.5f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("rotation z", &in_roll, 0.5f, 0.0f, 0.0f, "%.2f");

        world->light.rotation.pitch = in_pitch;
        world->light.rotation.yaw = in_yaw;
        world->light.rotation.roll = in_roll;
    }
    ImGui::End();

    //ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    // Clear all of the logs in the logger
    Logger::reset();
}

void GUI::destroy()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GUI::print_log_messages(std::vector<LogEntry>& log)
{
    for (const LogEntry& entry : log)
    {
        ImGui::Text(entry.message.c_str());
    }
}
