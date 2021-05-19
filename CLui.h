#pragma once
#include "stdafx.h"
#include <memory>
#include "imgui.h"

namespace Glaze3D
{
    struct RendererData;

    struct CLui
    {
        GLuint fontTexture = 0;
        double mousePixels_x = 0;
        double mousePixels_y = 0;
        double mouseDeltaX = 0;
        double mouseDeltaY = 0;
        double mouseLastNDC_x;
        double mouseLastNDC_y;
        double mouseNDC_x;
        double mouseNDC_y;
        bool mousePressed[3] = { false,false,false };
        int window_width = 1280;
        int window_height = 720;

        bool firstRun = true;
        bool framestepOn = false;
        bool isInitialized = false;
        bool isPaused = false;
        bool renderPopup = true;
        bool windowClose = false;

        /*Glaze3D UI color palatte:*/
        const ImVec4 Glaze3DMainGreyColor = ImVec4(0.784, 0.784, 0.784, 1);	// Light grey for backgrounds.
        const ImVec4 Glaze3DTealColor = ImVec4(0.2, 0.78, 0.78, 1);			// Light green accent color.
        const ImVec4 Glaze3DPinkColor = ImVec4(0.925, 0.3725, 0.6078, 1);	// Pink accent color.
        const ImVec4 Glaze3DDarkTealColor = ImVec4(0.0, 0.45, 0.45, 1);		// Darker green accent color.
        const ImVec4 Glaze3DWhiteColor = ImVec4(1, 1, 1, 1);				// Solid white for some text, outlines.

        struct RendererData
        {
            double time = 0;
            int shaderHandle = 0;
            int vertHandle = 0;
            int fragHandle = 0;
            int attribLocationTex = 0;
            int attribLocationProjMtx = 0;
            int attribLocationPosition = 0;
            int attribLocationUV = 0;
            int attribLocationColor = 0;
            unsigned int vboHandle = 0;
            unsigned int vaoHandle = 0;
            unsigned int elementsHandle = 0;
            bool disabled = false;

            ~RendererData()
            {
                try
                {
                    if (renderer->vaoHandle) { glDeleteVertexArrays(1, &renderer->vaoHandle); }
                    if (renderer->vboHandle) { glDeleteBuffers(1, &renderer->vboHandle); }
                    if (renderer->elementsHandle) { glDeleteBuffers(1, &renderer->elementsHandle); }
                }
                catch (std::exception e1)
                {
                    MessageBox(0, e1.what(), 0, 0);
                }

                renderer->vaoHandle = renderer->vboHandle = renderer->elementsHandle = 0;
                glDetachShader(renderer->shaderHandle, renderer->vertHandle);
                glDeleteShader(renderer->vertHandle);
                renderer->vertHandle = 0;
                glDetachShader(renderer->shaderHandle, renderer->fragHandle);
                glDeleteShader(renderer->fragHandle);
                renderer->fragHandle = 0;
                glDeleteProgram(renderer->shaderHandle);
                renderer->shaderHandle = 0;
            }
        };

        CLui(){}

        static void RenderDrawList(ImDrawData* drawData);
        static std::unique_ptr<RendererData> renderer;

		void init();
        void renderUI();
        void terminate();
        void SetDefaultStyle();
	};
}