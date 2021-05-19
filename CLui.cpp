#include "CLui.h"
#include "CLEngineBase.h"
#include "CLRaytracer.h"
#include "CLCamera.h"
#include "stdafx.h"
#include <glm/gtc/type_ptr.hpp>

namespace Glaze3D
{
    std::unique_ptr<CLui::RendererData> CLui::renderer = nullptr;

    void CLui::terminate()
    {
        if (fontTexture)
        {
            glDeleteTextures(1, &fontTexture);
            ImGui::GetIO().Fonts->TexID = 0;
            fontTexture = 0;
        }
        delete renderer.release();
    }

	void CLui::init()
	{
        if (isInitialized)
            return;

        if (!renderer) { renderer = std::make_unique<RendererData>(); }

        // Setup imgui window.    
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Initialize ImGui context and set values.
        ImGui_ImplGlfw_InitForOpenGL(eng->window, true);
        ImGui_ImplOpenGL3_Init();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
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

        // Setup fonts and build texture atlas.
        unsigned char* pixels;
        int width, height;
        // Load as RGBA 32-bits for OpenGL3 demo because it is more
        // likely to be compatible with user's existing shader.
        io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 14.0f);
        //Glaze3D::CustomFileDialog::Instance()->runAfterLoadingAFont();
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        // Create OpenGL texture
        glGenTextures(1, &fontTexture);
        glBindTexture(GL_TEXTURE_2D, fontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, pixels);
        // Store our identifier
        io.Fonts->TexID = (void*)(intptr_t)fontTexture;

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
        io.RenderDrawListsFn = RenderDrawList;
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

        // Backup GL state
        GLint last_texture, last_array_buffer, last_vertex_array;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
        auto vertex_shader =
            "#version 330\n"
            "uniform mat4 ProjMtx;\n"
            "in vec2 Position;\n"
            "in vec2 UV;\n"
            "in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "	Frag_UV = UV;\n"
            "	Frag_Color = Color;\n"
            "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";
        auto fragment_shader =
            "#version 330\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
            "}\n";
        renderer->shaderHandle = glCreateProgram();
        renderer->vertHandle = glCreateShader(GL_VERTEX_SHADER);
        renderer->fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(renderer->vertHandle, 1, &vertex_shader, 0);
        glShaderSource(renderer->fragHandle, 1, &fragment_shader, 0);
        glCompileShader(renderer->vertHandle);
        glCompileShader(renderer->fragHandle);
        glAttachShader(renderer->shaderHandle, renderer->vertHandle);
        glAttachShader(renderer->shaderHandle, renderer->fragHandle);
        glLinkProgram(renderer->shaderHandle);
        renderer->attribLocationTex = glGetUniformLocation
        (renderer->shaderHandle, "Texture");
        renderer->attribLocationProjMtx = glGetUniformLocation
        (renderer->shaderHandle, "ProjMtx");
        renderer->attribLocationPosition = glGetAttribLocation
        (renderer->shaderHandle, "Position");
        renderer->attribLocationUV = glGetAttribLocation
        (renderer->shaderHandle, "UV");
        renderer->attribLocationColor = glGetAttribLocation
        (renderer->shaderHandle, "Color");
        glGenBuffers(1, &renderer->vboHandle);
        glGenBuffers(1, &renderer->elementsHandle);
        glGenVertexArrays(1, &renderer->vaoHandle);
        glBindVertexArray(renderer->vaoHandle);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vboHandle);
        glEnableVertexAttribArray(renderer->attribLocationPosition);
        glEnableVertexAttribArray(renderer->attribLocationUV);
        glEnableVertexAttribArray(renderer->attribLocationColor);
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
        glVertexAttribPointer(renderer->attribLocationPosition, 2, GL_FLOAT,
            GL_FALSE, sizeof(ImDrawVert), (GLvoid*)
            OFFSETOF(ImDrawVert, pos));
        glVertexAttribPointer(renderer->attribLocationUV, 2, GL_FLOAT,
            GL_FALSE, sizeof(ImDrawVert), (GLvoid*)
            OFFSETOF(ImDrawVert, uv));
        glVertexAttribPointer(renderer->attribLocationColor, 4, GL_UNSIGNED_BYTE,
            GL_TRUE, sizeof(ImDrawVert), (GLvoid*)
            OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

        // Restore modified GL state
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBindVertexArray(last_vertex_array);

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
        io.DeltaTime = renderer->time > 0.0
            ? static_cast<float>(current_time - renderer->time)
            : static_cast<float>(1.0f / 60.0f);
        renderer->time = current_time;

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
                    isPaused = !isPaused;
                }
            if (!isPaused)
                if (ImGui::Button("Pause##pause main menu"))
                {
                    isPaused = !isPaused;
                }

            if (ImGui::Button("Update"))
            {
                // ADD LATER
            }

            if (ImGui::Button("Next Frame"))
            {
                framestepOn = true;
                isPaused = false;
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
                //eng->m_Camera->m_Changed = true;
                eng->render->m_FrameCount = 0;
            }

            static glm::vec2 r = glm::vec2(eng->m_Camera.pitch, eng->m_Camera.yaw);
            if (ImGui::DragFloat2("Camera Rotation", glm::value_ptr(r), 0.001, -3.1415926, 3.1415926))
            {
                eng->m_Camera.pitch = r.x;
                eng->m_Camera.yaw = r.y;
                //eng->m_Camera.m_Changed = true;
                eng->m_Camera.Update();
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat3("Light Position", glm::value_ptr(eng->m_Light.position), 0.01, -9999, 9999))
            {
                //m_Camera->m_Changed = true;
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat("Light Intensity", &eng->m_Light.intensity, 0.01, 0.0, 9999))
            {
                //m_Camera->m_Changed = true;
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat("Skybox Intensity", &eng->render->skyboxIntensity, 0.01, 0.0, 9999))
            {
                //m_Camera->m_Changed = true;
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragFloat("Light Attenuation", &eng->m_Light.attenuation, 0.01, 0.001, 9999))
            {
                //m_Camera->m_Changed = true;
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::DragInt("Light Bounces", &eng->render->lightBounces, 1, 1, 20))
            {
                //m_Camera->m_Changed = true;
                eng->render->m_FrameCount = 0;
            }

            if (ImGui::SliderInt("DOF On", &eng->render->dofOn, 0, 1))
            {
                //m_Camera->m_Changed = true;
                eng->render->m_FrameCount = 0;
            }

            ImGui::End();
        }
    }

    void CLui::RenderDrawList(ImDrawData* drawData)
    {
        // Backup GL state
        GLint last_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        GLint last_array_buffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        GLint last_element_array_buffer;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
        GLint last_vertex_array;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
        GLint last_blend_src;
        glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
        GLint last_blend_dst;
        glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
        GLint last_blend_equation_rgb;
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
        GLint last_blend_equation_alpha;
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
        GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
        GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
        GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
        GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
        // Setup render state: alpha-blending enabled,
        // no face culling, no depth testing, scissor enabled
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);
        glActiveTexture(GL_TEXTURE0);
        // Handle cases of screen coordinates != from
        // framebuffer coordinates (e.g. retina displays)
        ImGuiIO& io = ImGui::GetIO();
        float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
        drawData->ScaleClipRects(io.DisplayFramebufferScale);
        // Setup orthographic projection matrix
        static glm::mat4x4 ortho_projection =
        {
            { 7.7f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 7.7f, 0.0f, 0.0f },
            { 0.0f, 0.0f, -1.0f, 0.0f },
            { -1.0f, 1.0f, 0.0f, 1.0f },
        };
        ortho_projection[0][0] = 2.0f / io.DisplaySize.x;
        ortho_projection[1][1] = 2.0f / -io.DisplaySize.y;
        glUseProgram(renderer->shaderHandle);
        glUniform1i(renderer->attribLocationTex, 0);
        glUniformMatrix4fv(renderer->attribLocationProjMtx, 1, GL_FALSE,
            glm::value_ptr(ortho_projection));
        glBindVertexArray(renderer->vaoHandle);

        for (int index = 0; index < drawData->CmdListsCount; index++)
        {
            const ImDrawList* cmd_list = drawData->CmdLists[index];
            const ImDrawIdx* idx_buffer_offset = 0;
            glBindBuffer(GL_ARRAY_BUFFER, renderer->vboHandle);
            glBufferData(GL_ARRAY_BUFFER,
                (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert),
                (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->elementsHandle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx),
                (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

            for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin();
                pcmd != cmd_list->CmdBuffer.end(); pcmd++)
            {
                if (pcmd->UserCallback)
                {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                    glScissor((int)pcmd->ClipRect.x,
                        (int)(fb_height - pcmd->ClipRect.w),
                        (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                        (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                        GL_UNSIGNED_SHORT, idx_buffer_offset);
                }

                idx_buffer_offset += pcmd->ElemCount;
            }
        }

        // Restore modified GL state
        glUseProgram(last_program);
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
        glBindVertexArray(last_vertex_array);
        glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
        glBlendFunc(last_blend_src, last_blend_dst);

        if (last_enable_blend) { glEnable(GL_BLEND); }
        else { glDisable(GL_BLEND); }

        if (last_enable_cull_face) { glEnable(GL_CULL_FACE); }
        else { glDisable(GL_CULL_FACE); }

        if (last_enable_depth_test) { glEnable(GL_DEPTH_TEST); }
        else { glDisable(GL_DEPTH_TEST); }

        if (last_enable_scissor_test) { glEnable(GL_SCISSOR_TEST); }
        else { glDisable(GL_SCISSOR_TEST); }
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
        style->TabMinWidthForUnselectedCloseButton = 0.000000;
        style->ColorButtonPosition = 1;
        style->ButtonTextAlign = ImVec2(0.5, 0.5);
        style->SelectableTextAlign = ImVec2(0, 0);
        style->DisplayWindowPadding = ImVec2(19, 19);
        style->DisplaySafeAreaPadding = ImVec2(15, 0);
        style->MouseCursorScale = 1.000000;
        style->AntiAliasedLines = 1;
        style->AntiAliasedFill = 1;
        style->CurveTessellationTol = 1.250000;
        style->CircleSegmentMaxError = 1.600000;
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