#include "CLui.h"
#include "CLEngineBase.h"
#include "CLRaytracer.h"
#include "CLCamera.h"
#include "stdafx.h"
#include <glm/gtc/type_ptr.hpp>

namespace Glaze3D
{
	void CLui::init()
	{
        if (isInitialized)
            return;

        // Setup imgui window.    
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Initialize ImGui context and set values.
        ImGui_ImplGlfw_InitForOpenGL(eng->window, true);
        ImGui_ImplOpenGL3_Init();
        ImGuiIO& io = ImGui::GetIO(); 
        (void)io;
        io.Fonts->AddFontDefault();
        io.Fonts->Build();
        io.IniFilename = NULL;								   // Disable loading from .ini file.
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
        //io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar;

        // Enable GLFW's mouse cursor.
        glfwSetInputMode(eng->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // Set ImGui's cursor settings -- ImGui's software cursor is very slow and eats up roughly 3 frames per second.
        io.MouseDrawCursor = true;

        // Store our identifier
        io.FontDefault = 0;

        // Keyboard mapping. ImGui will use those indices to peek
        // into the io.KeyDown[] array.
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
        //io.RenderDrawListsFn = RenderDrawList;
        //io.SetClipboardTextFn = SetClipboardText;
        //io.GetClipboardTextFn = GetClipboardText;
#ifdef _WIN32
        io.ImeWindowHandle = eng->m_hWnd;
#endif

        /*if (instantCallbacks)
        {*/
        //glfwSetCursorPosCallback(renderer->window, (GLFWcursorposfun)cursorCallback);
        //glfwSetMouseButtonCallback(renderer->window, MouseButtonCallback);
        //glfwSetScrollCallback(renderer->window, ScrollCallback);
        //glfwSetKeyCallback(renderer->window, KeyCallback);
        //glfwSetCharCallback(renderer->window, CharCallback);
    //}

        int w, h, display_w, display_h;
        glfwGetWindowSize(eng->window, &w, &h);
        glfwGetFramebufferSize(eng->window, &display_w, &display_h);
        io.DisplaySize.x = static_cast<float>(w);
        io.DisplaySize.y = static_cast<float>(h);
        io.DisplayFramebufferScale.x = static_cast<float>(display_w) / w;
        io.DisplayFramebufferScale.y = static_cast<float>(display_h) / h;

        SetDefaultStyle();

        isInitialized = true;
	}

    void CLui::renderUI()
    {
        static auto& io = ImGui::GetIO();

        if (glfwGetWindowAttrib(eng->window, GLFW_RESIZABLE))
        {
            // setup display size (every frame to accommodate for window resizing)
            static int w, h, display_w, display_h;
            glfwGetWindowSize(eng->window, &w, &h);
            glfwGetFramebufferSize(eng->window, &display_w, &display_h);
            io.DisplaySize.x = static_cast<float>(w);
            io.DisplaySize.y = static_cast<float>(h);;
            io.DisplayFramebufferScale.x = static_cast<float>(display_w) / w;
            io.DisplayFramebufferScale.y = static_cast<float>(display_h) / h;
        }


        // setup time step
        auto current_time = glfwGetTime();

        // Get mouse position in screen coordinates
        glfwGetCursorPos(eng->window, &mousePixels_x, &mousePixels_y);
        io.MousePosPrev = io.MousePos;
        io.MousePos = ImVec2(static_cast<float>(mousePixels_x),
            static_cast<float>(this->mousePixels_y));

        //// NEW CODE -- Use ImGui window space coordinates.
        this->mouseNDC_x = ((this->mousePixels_x / window_width) - 0.5) * 2.0;
        this->mouseNDC_y = -((this->mousePixels_y / window_height) - 0.5) * 2.0;

        // Get mouse delta.
        this->mouseDeltaX = this->mouseNDC_x - this->mouseLastNDC_x;
        this->mouseDeltaY = this->mouseNDC_y - this->mouseLastNDC_y;

        for (auto i = 0; i < 3; i++)
        {
            io.MouseDown[i] = this->mousePressed[i] ||
                glfwGetMouseButton(eng->window, i) != 0;
            // If a mouse press event came, always pass it as
            // "this frame", so we don't miss click-release events
            // that are shorter than 1 frame.
            this->mousePressed[i] = false;
        }

        //io.MouseWheel = this->mouseWheel;
        //this->mouseWheel = 0.0f;


        if (ImGui::BeginMainMenuBar())
        {
            // Make menu background have alpha=1 so it's readable.
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.94, 0.94, 0.94, 1));
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Project", nullptr))
                {

                }
                if (ImGui::MenuItem("Load Project", nullptr))
                {

                }
                ImGui::Text("Recent Projects: ");
                if (ImGui::MenuItem("Save Project", nullptr))
                {

                }
                if (ImGui::MenuItem("Exit##main_menu", nullptr))
                {
                    windowClose = true;
                    glfwWindowShouldClose(eng->window);
                    glfwDestroyWindow(eng->window);
                }
                ImGui::EndMenu();
            }

            if (isPaused)
                if (ImGui::Button("Unpause##unpause main menu"))
                {
                    isPaused = false;
                    framestepOn = false;
                }
            if (!isPaused)
                if (ImGui::Button("Pause##pause main menu"))
                {
                    isPaused = true;
                }

            if (ImGui::Button("Update"))
            {
                // ADD LATER
            }

            if (ImGui::Button("Next Frame"))
            {
                framestepOn = true;
                isPaused = true;
            }

            if (isPaused)
            {
                ImGui::TextColored(Glaze3DWhiteColor, "-- RENDERING PAUSED -- ");
            }

            std::string fps_str = "FPS: " + std::to_string(eng->FPS);
            ImGui::Text(fps_str.c_str());

            ImGui::PopStyleColor();
            ImGui::EndMainMenuBar();
        }

        // Render UI.
        if (firstRun)
        {
            ImVec2 win_sz = ImVec2(window_width * 0.8, window_height * 0.4);
            ImGui::SetNextWindowSize(win_sz);
            ImGui::SetNextWindowPos(ImVec2(0, window_height - win_sz.y * 1.5));
        }
        if (ImGui::Begin("Settings", &renderPopup, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Dummy(ImVec2(0, 5));

            static glm::vec3 o = glm::vec3(eng->m_Camera.position);
            if (ImGui::DragFloat3("Camera Position", glm::value_ptr(o), 0.01, 9999, 9999))
            {
                eng->m_Camera.position = o;
                eng->render->m_FrameCount = 0;
            }

            static glm::vec2 r = glm::vec2(eng->m_Camera.pitch, eng->m_Camera.yaw);
            if (ImGui::DragFloat2("Camera Rotation", glm::value_ptr(r), 0.001, -3.1415926, 3.1415926))
            {
                eng->m_Camera.pitch = r.x;
                eng->m_Camera.yaw = r.y;
                eng->m_Camera.Update();
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat3("Light Position", glm::value_ptr(eng->m_Light.position), 0.01, -9999, 9999))
            {
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat("Light Intensity", &eng->m_Light.intensity, 0.01, 0.0, 9999))
            {
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat("Skybox Intensity", &eng->render->skyboxIntensity, 0.01, 0.0, 9999))
            {
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat("Light Attenuation", &eng->m_Light.attenuation, 0.01, 0.001, 9999))
            {
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::SliderInt("Light Bounces", &eng->render->lightBounces, 1, 20))
            {
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::SliderInt("Light Type", &eng->render->lightType, 0, 2))
            {
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::SliderInt("DOF On", &eng->render->dofOn, 0, 1))
            {
                eng->render->m_FrameCount = 0;
            }

            ImGui::End();
        }
    }

    void CLui::SetDefaultStyle() 
    {
        ImGuiStyle* style = &ImGui::GetStyle();

        style->Alpha = 1.000000;
        style->ChildRounding = 5.000000;
        style->ChildBorderSize = 2.000000;
        style->PopupRounding = 5.000000;
        style->PopupBorderSize = 2.000000;
        style->FramePadding = ImVec2(10, 10);
        style->FrameRounding = 5.000000;
        style->FrameBorderSize = 1.000000;
        style->ItemSpacing = ImVec2(10, 14);
        style->ItemInnerSpacing = ImVec2(10, 3);
        style->CellPadding = ImVec2(4, 4);
        style->TouchExtraPadding = ImVec2(0, 0);
        style->IndentSpacing = 26.000000;
        style->ColumnsMinSpacing = 6.000000;
        style->ScrollbarSize = 14.000000;
        style->ScrollbarRounding = 5.000000;
        style->GrabMinSize = 9.000000;
        style->GrabRounding = 2.000000;
        style->TabRounding = 5.000000;
        style->TabBorderSize = 1.000000;
        //style->TabMinWidthForUnselectedCloseButton = 0.000000;
        style->ColorButtonPosition = 1;
        style->ButtonTextAlign = ImVec2(0.5, 0.5);
        style->SelectableTextAlign = ImVec2(0, 0);
        style->DisplayWindowPadding = ImVec2(19, 19);
        style->DisplaySafeAreaPadding = ImVec2(15, 0);
        style->MouseCursorScale = 1.000000;
        style->AntiAliasedLines = 1;
        style->AntiAliasedFill = 1;
        style->CurveTessellationTol = 1.250000;
        //style->CircleSegmentMaxError = 1.600000;
        style->WindowPadding = ImVec2(5, 15);
        style->WindowRounding = 5.000000;
        style->WindowBorderSize = 2.000000;
        style->WindowMinSize = ImVec2(32, 32);
        style->WindowTitleAlign = ImVec2(0, 0.5);
        style->WindowMenuButtonPosition = 0;
        style->Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
        style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.6, 0.6, 0.6, 1);
        style->Colors[ImGuiCol_WindowBg] = ImVec4(0.94, 0.94, 0.94, 0.86);
        style->Colors[ImGuiCol_ChildBg] = ImVec4(0.8, 0.796078, 0.807843, 0.501961);
        style->Colors[ImGuiCol_PopupBg] = ImVec4(1, 1, 1, 0.6);
        style->Colors[ImGuiCol_Border] = ImVec4(0.925, 0.3725, 0.6078, 0.35);
        style->Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
        style->Colors[ImGuiCol_FrameBg] = ImVec4(0.925, 0.3725, 0.6078, 0.35);
        style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.258824, 0.588235, 0.588235, 0.4);
        style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.211765, 0.65098, 0.678431, 0.670588);
        style->Colors[ImGuiCol_TitleBg] = ImVec4(0.678431, 0.796078, 0.835294, 0.929412);
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.678431, 0.796078, 0.835294, 0.929412);
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.678431, 0.796078, 0.835294, 0.929412);
        style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.678431, 0.796078, 0.835294, 0.929412);
        style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98, 0.98, 0.98, 0.53);
        style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69, 0.69, 0.69, 0.8);
        style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49, 0.49, 0.49, 0.8);
        style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49, 0.49, 0.49, 1);
        style->Colors[ImGuiCol_CheckMark] = ImVec4(0.203922, 0.780392, 0.780392, 1);
        style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.203922, 0.745098, 0.776471, 0.780392);
        style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.231373, 0.541176, 0.694118, 0.6);
        style->Colors[ImGuiCol_Button] = ImVec4(0.207843, 0.737255, 0.756863, 0.4);
        style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.258824, 0.588235, 0.588235, 1);
        style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.0980392, 0.529412, 0.541176, 1);
        style->Colors[ImGuiCol_Header] = ImVec4(0.14902, 0.588235, 0.596078, 0.309804);
        style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26, 0.80, 0.75, 0.6);
        style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.26, 0.89, 0.82, 1);
        style->Colors[ImGuiCol_Separator] = ImVec4(0.0705882, 0.380392, 0.34902, 0.619608);
        style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14, 0.44, 0.8, 0.78);
        style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.14, 0.44, 0.8, 1);
        style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.8, 0.8, 0.8, 0.56);
        style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26, 0.59, 0.98, 0.67);
        style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26, 0.59, 0.98, 0.95);
        style->Colors[ImGuiCol_Tab] = ImVec4(0.678431, 0.796078, 0.835294, 0.929412);
        style->Colors[ImGuiCol_TabHovered] = ImVec4(0.0980392, 0.541176, 0.552941, 0.8);
        style->Colors[ImGuiCol_TabActive] = ImVec4(0.431373, 0.694118, 0.698039, 1);
        style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.486275, 0.92549, 0.933333, 0.984314);
        style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.466667, 0.919608, 0.913725, 1);
        style->Colors[ImGuiCol_TableBorderLight] = ImVec4(0.678431, 0.796078, 0.835294, 0.929412);
        style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.678431, 0.796078, 0.835294, 0.929412);
        style->Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0980392, 0.541176, 0.552941, 0.8);
        style->Colors[ImGuiCol_TableRowBg] = ImVec4(0.431373, 0.694118, 0.698039, 1);
        style->Colors[ImGuiCol_PlotLines] = ImVec4(0.39, 0.39, 0.39, 1);
        style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1, 0.43, 0.35, 1);
        style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.9, 0.7, 0, 1);
        style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1, 0.45, 0, 1);
        style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.258824, 0.823529, 0.796078, 0.34902);
        style->Colors[ImGuiCol_DragDropTarget] = ImVec4(0.26, 0.59, 0.98, 0.95);
        style->Colors[ImGuiCol_NavHighlight] = ImVec4(0.26, 0.59, 0.98, 0.8);
        style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.7, 0.7, 0.7, 0.7);
        style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.2, 0.2, 0.2, 0.2);
        style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.2, 0.2, 0.2, 0.35);
    }
}