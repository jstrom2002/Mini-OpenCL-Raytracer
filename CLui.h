#pragma once
#include "stdafx.h"
#include <memory>
#include "imgui.h"

namespace Glaze3D
{
    struct RendererData;

    struct CLui
    {
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

		void init();
        void renderUI();
        void SetDefaultStyle();
	};
}