// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "lobster/bytecode_generated.h"

#include "lobster/lobsterreader.h"

#include "lobster/sdlincludes.h"
#include "lobster/sdlinterface.h"
#include "lobster/glinterface.h"

#include "lobster/graphics.h"

#undef new
#include "imgui_internal.h"
#if defined(_MSC_VER) && !defined(NDEBUG)
    #define new DEBUG_NEW
#endif


using namespace lobster;

extern SDL_Window *_sdl_window;
extern SDL_GLContext _sdl_context;
extern SDL_Window *_sdl_debugger_window;
extern SDL_GLContext _sdl_debugger_context;

bool imgui_init = false;

enum Nesting {
    N_NONE,
    N_FRAME,
    N_MAIN_MENU_BAR,
    N_WIN,
    N_MENU_BAR,
    N_MENU,
    N_TAB_BAR,
    N_TAB,
    N_ID,
    N_WIDTH,
    N_TREE,
    N_POPUP,
    N_CHILD,
    N_VGROUP,
    N_TOOLTIP,
    N_FONT,
    N_COLOR,
    N_DRAG_DROP_SOURCE,
    N_DRAG_DROP_TARGET,
    N_TABLE,
    N_DISABLED,
    N_INDENT,
};

vector<Nesting> nstack;
vector<float> indent_stack;
ImGuiID last_dock_id = 0;
struct ListSelect { int sel = -1; int64_t last_use = -1; };
map<ImGuiID, ListSelect> list_state;
int64_t imgui_frame = 0;
ImVec2 tabs_extra_padding(0.0f, 0.0f);

// Flags we use in our implementation, in addition to DearImGui.
enum {
    ImGuiTreeNodeFlags_IndentHalf = 1 << 30
};

void IMGUIFrameCleanup() {
    nstack.clear();
    indent_stack.clear();
}

void IMGUICleanup() {
    if (!imgui_init) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    imgui_init = false;
    IMGUIFrameCleanup();
}

bool IMGUIInit(iint flags, bool dark, float rounding, float border) {
    if (imgui_init) return true;
    if (!_sdl_window || !_sdl_context) return false;
    IMGUI_CHECKVERSION();
    auto context = ImGui::CreateContext();
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= (ImGuiConfigFlags)flags;
    io.ConfigDebugHighlightIdConflicts = true;
    io.ConfigErrorRecovery = true;
    io.ConfigErrorRecoveryEnableAssert = false;
    io.ConfigErrorRecoveryEnableTooltip = true;
    io.ConfigErrorRecoveryEnableDebugLog = true;
    if (dark)
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsClassic();
    auto r = rounding;
    // For some reason active tabs are by default almost the same color as inactive ones, fix that.
    // TODO: instead, expose color/style API thru bindings.
    auto &style = ImGui::GetStyle();
    style.WindowBorderSize = style.ChildBorderSize = style.PopupBorderSize = border;
    style.Colors[ImGuiCol_Tab].w = 0.37f;  // More transparent.
    style.FrameRounding = r;
    style.WindowRounding = r;
    style.GrabRounding = r;
    style.ChildRounding = r;
    style.PopupRounding = r;
    style.ScrollbarRounding = r;
    style.TabRounding = r;
    if (IsSRGBAwareMode()) {
        // Colors are specified as SRGB, so pre-convert them if we're using linear.
        auto cols = ImGui::GetStyle().Colors;
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            cols[i].x = powf(cols[i].x, 2.2f);
            cols[i].y = powf(cols[i].y, 2.2f);
            cols[i].z = powf(cols[i].z, 2.2f);
        }
    }
    ImGui_ImplSDL2_InitForOpenGL(_sdl_window, _sdl_context);
    ImGui_ImplOpenGL3_Init(
        #ifdef PLATFORM_ES3
            "#version 300 es"
        #else
            "#version 150"
        #endif
    );
    // This disables ctrl+tab being a window switcher.
    // TODO: may need to be exposed to be optional?
    // https://github.com/ocornut/imgui/issues/3255
    context->ConfigNavWindowingKeyNext = 0;
    context->ConfigNavWindowingKeyPrev = 0;
    imgui_init = true;
    return true;
}

void NPush(Nesting n) {
    nstack.push_back(n);
}

Nesting NTop() {
    return nstack.empty() ? N_NONE : nstack.back();
}

void NPop(VM &vm, Nesting n) {
    for (;;) {
        if (nstack.empty()) {
            // This should be a rare error given that they're all called from HOFs.
            vm.BuiltinError("imgui: nested end without a start");
        }
        // We pop things regardless if they're the thing we're wanting to pop.
        // This allows the Lobster code to return from a HOF doing start+end, and not
        // get asserts for missing ends.
        auto tn = nstack.back();
        nstack.pop_back();
        switch (tn) {
            case N_FRAME:
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                // FIXME: this seems buggy.
                /*
                // Update and Render additional Platform Windows
                if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                }
                */
                break;
            case N_WIN:
                ImGui::End();
                break;
            case N_TAB_BAR:
                ImGui::EndTabBar();
                break;
            case N_TAB:
                ImGui::EndTabItem();
                break;
            case N_ID:
                ImGui::PopID();
                break;
            case N_WIDTH:
                ImGui::PopItemWidth();
                break;
            case N_TREE:
                ImGui::TreePop();
                break;
            case N_POPUP:
                ImGui::EndPopup();
                break;
            case N_CHILD:
                ImGui::EndChild();
                break;
            case N_VGROUP:
                ImGui::EndGroup();
                break;
            case N_MENU_BAR:
                ImGui::EndMenuBar();
                break;
            case N_MAIN_MENU_BAR:
                ImGui::EndMainMenuBar();
                break;
            case N_MENU:
                ImGui::EndMenu();
                break;
            case N_TOOLTIP:
                ImGui::EndTooltip();
                break;
            case N_FONT:
                ImGui::PopFont();
                break;
            case N_COLOR:
                ImGui::PopStyleColor();
                break;
            case N_DRAG_DROP_SOURCE:
                ImGui::EndDragDropSource();
                break;
            case N_DRAG_DROP_TARGET:
                ImGui::EndDragDropTarget();
                break;
            case N_TABLE:
                ImGui::EndTable();
                break;
            case N_DISABLED:
                ImGui::EndDisabled();
                break;
            case N_INDENT:
                if (!indent_stack.empty()) {
                    ImGui::Unindent(indent_stack.back());
                    indent_stack.pop_back();
                }
                break;
        }
        // If this was indeed the item we're looking for, we can stop popping.
        if (tn == n) break;
    }
}

void IsInit(VM &vm, pair<Nesting, Nesting> require = { N_WIN, N_MENU }) {
    if (!imgui_init) vm.BuiltinError("imgui: not running: call im.init first");
    for (auto n : nstack) {
        if (n == require.first || n == require.second) return;
        // A popup is a window, so should N_POPUP should allow anything that requires N_WIN.
        if (n == N_POPUP) {
            n = N_WIN;
            if (n == require.first || n == require.second) return;
        }
    }
    if (require.first != N_NONE || require.second != N_NONE) {
        vm.BuiltinError("imgui: invalid nesting (not inside im.window?)");
    }
}

void RequireMenuNesting(VM &vm) {
    for (auto n : nstack) if (n == N_MENU_BAR || n == N_MAIN_MENU_BAR || n == N_POPUP) return;
    vm.BuiltinError("imgui: invalid menu nesting");
}

pair<bool, bool> IMGUIEvent(SDL_Event *event) {
    if (!imgui_init) return { false, false };
    switch (event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            // ImGui likes to use ESC for cancelling edit actions, but pretty much
            // every game uses ESC to switch menus, which would cause users wanting to
            // switch menus to lose their edits unintentionally.
            // TODO: make this behavior configurable from Lobster code?
            if (event->key.keysym.sym == SDLK_ESCAPE) return { false, false };
            break;
        }
    }
    ImGui_ImplSDL2_ProcessEvent(event);
    // Always pass through key up events after processing them by ImGui.
    if (event->type == SDL_KEYUP) return { false, false };
    return { ImGui::GetIO().WantCaptureMouse, ImGui::GetIO().WantCaptureKeyboard };
}

void ClearFonts() {
    auto atlas = ImGui::GetIO().Fonts;
    atlas->Clear();
}

bool LoadFont(string_view name, float size, string_view lang_name) {
    string buf;
    auto l = LoadFile(name, &buf);
    if (l < 0) return false;
    auto mb = malloc(buf.size());  // FIXME.
    assert(mb);
    std::memcpy(mb, buf.data(), buf.size());
    ImFontConfig imfc;
    imfc.FontDataOwnedByAtlas = true;
    auto atlas = ImGui::GetIO().Fonts;
    const ImWchar *glyph_ranges = atlas->GetGlyphRangesDefault();
    if (lang_name == "SimplifiedChinese") {
        // Copied and modified from ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon() in imgui_draw.cpp
        // Include the 2500 glyphs from GetGlyphRangesChineseSimplifiedCommon(),
        // as well as 1000 glyphs from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
        // and an additional 182 glyphs from https://character-table.netlify.app/simplified-chinese/ (which is collated from https://cldr.unicode.org/)
        static const short accumulative_offsets_from_0x4E00[] = {
            0,1,2,4,1,1,1,1,1,1,1,2,1,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,1,1,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,11,8,2,1,2,1,
            2,1,2,1,2,1,5,1,6,3,1,1,1,1,1,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,3,2,1,2,1,1,1,10,1,1,5,2,4,2,4,1,4,2,2,2,9,3,2,1,1,6,1,1,1,4,1,1,4,2,4,5,1,4,
            2,2,2,2,1,6,3,7,1,1,1,1,2,2,4,2,1,4,3,1,5,10,12,5,4,3,2,14,2,3,3,2,1,1,1,4,2,1,6,10,4,1,6,5,1,7,1,5,4,8,4,1,1,2,9,4,15,5,2,4,1,1,5,2,5,20,
            2,2,9,5,2,1,11,2,9,17,1,8,1,5,8,27,4,6,9,20,11,13,14,6,23,15,30,2,2,1,1,1,2,1,2,2,4,3,6,2,6,3,3,3,1,1,1,2,1,2,1,1,1,1,1,1,2,1,1,3,5,3,4,1,
            5,3,2,2,2,1,4,4,8,3,1,2,1,2,1,1,4,5,4,2,3,3,3,2,10,2,3,1,3,7,2,2,1,3,3,2,1,1,1,2,2,1,1,2,3,1,3,7,1,5,1,1,1,1,2,3,4,4,1,2,3,2,6,1,1,1,1,1,
            2,5,1,7,3,4,3,2,15,2,2,1,5,3,13,9,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,4,4,2,2,7,6,7,5,22,4,1,1,5,1,2,13,1,1,2,7,3,7,15,1,1,3,1,2,2,4,1,2,4,1,2,
            1,1,2,1,1,3,2,4,1,1,2,2,1,4,4,1,1,2,1,1,2,1,7,3,2,1,1,3,2,1,1,8,3,2,5,3,4,2,19,4,2,1,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,1,1,1,1,
            2,1,3,1,1,1,3,1,4,2,1,2,2,1,1,2,1,1,1,1,1,2,2,2,4,2,1,1,1,6,1,1,1,2,1,1,1,1,2,3,1,3,1,2,1,2,2,6,2,2,6,5,3,3,1,6,4,2,11,2,6,1,1,9,6,3,1,2,3,
            1,3,13,1,1,2,2,3,2,2,5,5,3,1,3,2,1,1,1,4,1,3,6,8,6,1,2,1,1,3,1,4,8,2,5,5,1,2,7,16,4,3,5,2,1,2,6,7,5,1,2,4,23,3,1,1,4,6,8,4,4,2,2,3,2,1,5,5,
            4,4,1,10,12,2,2,4,10,13,1,9,5,3,2,2,7,14,3,1,8,18,22,1,2,2,3,2,4,41,1,1,36,21,20,5,14,16,1,3,2,2,2,9,3,1,3,6,3,1,5,3,2,23,4,5,5,3,3,7,4,
            2,7,3,4,1,1,1,6,3,1,1,1,1,1,1,1,3,2,4,5,8,11,1,1,7,7,9,7,4,5,3,20,1,8,3,6,11,1,25,1,8,4,15,12,3,6,6,5,23,5,3,4,6,13,24,2,14,6,5,10,1,24,20,
            15,7,1,2,2,3,3,3,11,3,6,2,6,1,4,2,3,6,2,2,1,1,2,1,1,2,3,3,1,1,1,10,3,1,1,2,4,2,3,1,1,1,9,2,3,14,1,2,2,1,4,5,2,2,1,1,10,1,3,3,12,3,6,11,2,
            11,4,1,5,1,2,1,6,2,9,3,19,4,2,2,1,3,17,4,3,10,8,5,16,3,17,26,2,9,19,8,25,14,1,7,3,21,8,32,71,4,1,2,1,1,3,1,2,4,1,2,3,12,8,4,2,2,2,1,1,2,1,
            3,8,1,1,1,1,1,1,1,2,1,1,1,1,2,4,1,5,3,1,1,1,3,4,1,1,3,2,1,1,1,5,6,1,10,1,1,2,4,3,16,1,1,1,1,3,2,3,2,3,1,5,2,3,2,2,2,3,7,13,7,2,2,1,1,1,1,1,
            1,3,3,1,1,1,3,1,2,4,9,2,1,4,10,2,8,6,2,1,18,2,1,1,2,1,14,4,6,5,41,5,7,3,11,12,7,6,2,19,4,7,24,129,16,1,3,1,3,1,1,1,1,2,3,3,1,1,1,3,7,3,1,
            1,2,1,2,4,4,5,1,2,2,2,1,9,7,1,10,5,8,7,8,1,13,16,1,1,2,2,3,1,1,2,5,2,1,3,5,1,3,1,1,2,2,1,2,2,1,7,1,6,8,1,1,1,17,1,9,3,32,1,3,6,2,1,1,6,5,4,
            2,2,1,3,4,1,5,1,1,8,2,8,1,24,1,2,8,5,2,5,1,2,1,3,1,6,2,2,1,4,1,3,1,3,2,1,1,4,2,5,1,1,8,9,4,7,2,6,6,2,1,6,1,10,1,1,7,7,4,6,4,8,2,1,1,13,4,
            2,1,1,6,1,3,1,4,2,1,2,5,12,8,8,2,3,2,3,13,2,4,1,3,1,2,1,3,3,6,8,5,4,7,11,1,3,3,2,4,3,3,2,8,9,5,1,6,4,7,4,6,1,1,1,2,2,2,1,3,3,3,8,7,1,6,6,5,
            5,5,3,24,9,4,2,7,13,5,1,8,7,20,3,6,20,22,4,6,2,8,20,34,2,2,3,1,1,1,4,2,2,16,9,1,3,8,1,1,6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,3,3,3,3,2,1,
            1,4,6,7,1,1,2,1,1,1,2,1,5,1,1,2,1,6,1,5,4,4,3,1,5,2,1,1,1,2,3,1,3,2,1,1,2,1,1,1,2,1,3,3,1,2,1,1,1,1,3,1,2,2,2,1,3,5,2,1,2,1,5,2,5,3,5,4,3,
            2,1,1,2,1,1,3,2,1,4,11,3,5,3,1,3,3,1,1,1,1,5,9,1,2,1,1,4,7,8,1,3,1,5,2,6,1,3,3,1,2,4,2,8,2,3,2,1,1,1,6,7,1,2,15,4,2,1,2,4,11,2,6,1,3,7,9,
            3,1,1,3,10,4,1,8,2,12,2,1,13,10,2,1,3,10,4,15,2,15,1,14,10,1,3,9,6,5,3,1,1,2,5,7,6,3,8,1,4,20,26,18,6,23,7,3,2,3,1,6,3,4,3,2,8,2,2,1,4,1,3,
            6,4,2,2,3,16,4,5,1,6,2,3,3,5,1,2,2,4,2,1,9,3,1,4,4,6,4,8,9,2,3,1,1,1,1,3,1,4,5,1,3,8,4,6,2,1,4,1,5,6,1,5,2,1,5,2,6,7,2,5,8,1,6,1,2,5,10,2,
            2,6,1,1,4,2,4,4,4,5,10,5,1,23,6,37,23,2,2,5,3,2,1,1,8,1,2,2,10,4,2,2,7,2,2,1,1,1,2,2,3,1,5,3,3,2,1,3,2,1,5,1,1,1,5,6,3,1,1,4,3,5,2,1,14,1,
            2,3,5,7,5,2,3,2,1,5,1,7,1,4,7,1,13,11,1,1,1,1,1,8,4,5,7,3,2,2,1,11,6,2,1,3,4,2,2,3,1,10,9,13,1,1,3,1,5,1,2,1,2,4,4,1,18,2,1,2,1,13,11,4,1,
            17,11,4,1,1,5,2,1,3,2,11,9,2,2,5,3,3,2,6,14,3,4,5,11,8,1,4,27,3,15,21,6,4,5,20,5,6,2,2,14,1,6,1,12,12,28,45,13,21,2,9,7,19,20,1,8,16,15,16,
            25,3,116,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,1,3,15,2,1,7,6,1,1,11,30,1,2,8,2,4,8,2,3,2,1,4,2,6,10,4,32,2,2,1,7,7,5,1,6,1,5,4,9,1,5,2,14,4,2,1,
            1,1,1,3,6,6,6,3,4,6,5,1,7,9,2,4,2,4,1,1,3,1,3,5,5,1,2,1,1,1,1,5,5,1,2,9,6,3,3,1,1,2,3,2,6,3,2,6,1,1,4,10,7,5,4,3,7,5,8,9,1,1,1,3,4,1,1,3,1,
            3,1,2,2,6,13,3,1,4,6,3,1,10,3,3,1,3,2,7,6,2,4,2,1,2,1,1,1,5,1,3,3,11,6,5,1,5,7,9,3,7,3,3,2,4,2,2,10,5,6,4,3,9,1,2,1,5,6,5,4,2,9,19,2,38,1,
            4,2,4,7,12,6,8,5,7,4,17,6,2,1,6,4,3,3,1,3,1,11,14,4,9,4,1,12,9,2,6,13,26,4,10,7,1,22,4,6,14,5,18,13,18,63,59,31,2,2,1,5,1,2,4,2,1,10,1,4,4,
            3,22,1,1,1,10,1,3,5,1,6,16,1,2,4,5,2,1,4,2,12,17,11,4,1,12,10,6,22,2,16,6,3,7,22,6,5,5,5,6,13,23,11,7,16,33,36,2,5,4,1,1,1,1,4,10,1,4,1,12,
            2,6,1,5,2,9,3,4,1,6,1,43,3,7,3,9,6,8,7,7,2,1,11,1,1,2,1,7,4,18,8,5,1,13,1,1,1,2,6,10,1,69,3,2,2,11,5,14,2,4,1,2,5,4,5,10,3,19,13,22,2,1,3,
            1,6,18,8,9,1,8,29,3,2,1,17,6,13,36,53,6,1,1,2,8,8,1,33,2,2,3,6,3,1,2,5,1,1,1,2,2,1,3,10,7,3,5,5,3,9,1,4,10,4,14,9,2,6,2,1,5,5,7,3,1,3,7,3,
            2,7,2,3,8,3,3,3,7,8,6,4,5,38,5,2,3,1,1,13,6,14,18,5,21,3,2,1,4,2,2,1,39,3,14,6,1,2,2,5,1,1,1,2,2,1,1,3,4,15,1,3,2,4,1,3,2,3,8,2,20,1,8,7,
            7,1,5,4,1,26,6,2,3,6,13,11,10,4,21,3,2,1,6,8,28,4,7,1,2,4,2,2,1,5,3,8,1,2,1,10,1,7,2,4,22,4,4,6,2,5,16,8,14,1,2,14,13,3,1,1,3,6,1,7,8,9,1,
            2,1,10,3,4,16,19,15,3,7,57,2,2,10,14,4,3,1,1,1,5,3,5,2,8,1,8,1,4,10,44,2,1,2,1,2,3,3,2,2,4,1,3,3,7,5,2,1,2,2,4,1,8,3,2,3,11,2,1,12,6,3,16,
            8,1,1,2,7,17,29,2,1,2,1,5,2,2,1,9,4,1,4,1,1,4,1,2,6,26,12,11,3,5,1,1,3,2,8,2,10,6,7,5,6,3,5,2,9,2,2,4,16,13,2,4,1,1,1,2,2,5,2,26,2,5,2,13,
            8,2,10,8,2,2,4,22,12,6,8,13,3,6,16,49,7,14,38,8,2,12,9,5,1,7,5,1,5,4,3,8,5,12,11,1,3,3,3,1,15,12,15,22,2,5,4,4,63,211,95,2,2,2,1,3,1,1,3,
            2,1,1,2,2,1,1,1,3,2,4,1,1,1,1,1,2,3,1,1,2,1,1,2,3,1,3,1,1,1,3,1,4,2,1,3,3,3,1,1,2,1,4,1,2,1,5,1,8,5,1,1,1,2,2,3,3,4,4,1,4,3,4,2,2,2,22,1,4,
            2,3,8,7,1,4,4,24,4,6,10,3,3,11,10,4,4,4,9,6,4,8,9,7,11,1,4,1,2,2,7,1,3,5,2,1,1,26,3,2,3,2,2,3,8,1,1,8,4,2,16,25,1,2,3,2,1,10,2,2,1,2,3,1,1,
            2,1,4,1,4,1,3,2,6,4,1,1,1,2,3,6,2,8,4,2,2,3,6,8,1,3,3,2,5,5,4,3,1,5,1,1,2,3,4,21,2,7,6,12,1,1,4,3,1,1,16,9,2,9,1,1,3,1,1,10,5,9,3,1, 1,11,
            11,13,2,8,25,7,3,6,1,8,4,5,1,6,1,5,2,10,1,11,2,4,1,4,1,1,2,14,17,23,1,2,1,7,4,4,9,2,5,7,3,1,8,1,6,1,2,2,2,6,4,10,6,2,5,3,4,3,1,6,1,5,6,8,8,
            1,1,1,1,4,5,25,1,3,1,8,1,1,2,14,3,7,2,2,6,4,2,1,2,1,3,8,8,1,17,5,28,1,6,1,5,2,1,3,10,3,2,16,4,9,8,1,18,8,1,1,15,7,1,2,1,21,26,4,6,2,8,1,5,
            4,13,9,14,3,22,6,7,5,5,13,7,15,37,2,4,3,7,10,1,16,1,12,1,42,10,6,3,20,15,5,26,6,1,5,15,23,22,39,22,1,1,1,9,17,6,8,4,1,2,1,1,8,2,7,2,7,7,1,
            6,5,17,6,1,2,2,9,5,2,9,10,11,5,2,2,6,10,1,2,2,1,4,5,26,12,2,3,2,9,2,7,20,2,13,10,18,27,6,6,5,46,28,13,30,5,7,1,7,3,2,8,2,2,3,1,2,1,4,7,10,
            3,7,2,5,4,6,15,2,4,16,1,3,1,15,4,11,15,5,1,9,14,2,19,5,53,32,2,5,59,1,2,1,1,2,1,9,17,3,26,137,1,9,211,6,53,1,2,1,3,1,4,1,1,1,2,1,3,1,1,1,1,
            2,1,1,1,1,1,3,1,1,2,1,1,3,4,4,2,3,3,1,3,1,3,1,5,1,1,2,2,1,2,1,2,1,2,1,2,1,3,2,2,1,2,2,1,2,1,2,2,1,7,2,6,1,1,2,2,4,1,4,3,3,10,5,6,21,9,1,14,
            1,18,145,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,2,1,1,1,2,2,1,2,1,1,1,6,1,1,2,1,1,4,2,2,2,1,3,2,1,1,2,2,5,5,4,1,1,1,10,4,5,1,20,4,16,1,
            10,5,8,1,5,10,2,2,9,2,5,4,2,2,3,16,5,19,7,1,7,17,2,4,3,27,6,1,5,15,14,28,1,3,3,8,172,1,1,1,3,2,1,1,4,7,2,2,4,2,1,2,1,7,1,2,2,1,3,2,1,2,1,4,
            5,1,2,5,1,8,4,1,3,1,2,1,2,1,6,2,1,3,4,1,2,1,1,1,1,7,4,1,5,7,2,4,3,1,1,1,3,3,1,5,1,2,2,3,3,1,2,1,1,1,2,12,10,3,1,2,4,5,6,2,4,4,12,2,8,1,7,10,
            1,17,2,15,1,3,4,4,3,10,6,9,4,3,23,5,16,5,23,48,2,1,1,5,5,6,6,2,5,2,5,4,2,1,7,8,2,2,7,53,3,2,1,1,1,2,163,532,1,10,6,4,1,1,1,2,3,1,1,2,5,3,2,
            8,3,3,2,2,1,10,11,1,1,4,1,1,10,5,4,2,2,4,2,1,3,1,3,1,2,1,4,9,1,7,2,1,1,1,2,5,1,1,1,9,1,1,5,7,3,6,12,7,10,3,6,9,105,2,3,1,1,1,2,2,3,1,1,1,1,
            2,3,1,4,9,2,4,11,15,3,1,1,1,1,1,5,4,1,4,1,1,2,3,1,1,3,5,12,1,2,4,1,1,11,1,1,14,1,9,1,4,4,1,3,11,15,8,2,1,3,1,1,1,2,3,3,6,19,2,5,4,3,1,2,5,2,
            7,2,4,1,8,6,2,20,6,1,25,4,3,5,2,1,1,7,11,2,6,15,2,4,15,13,57,1,2,4,6,2,128,1,1,2,1,1,2,1,1,2,1,2,1,1,1,1,2,7,2,3,1,1,4,1,3,4,42,4,6,1,5,1,
            49,85,8,1,2,1,1,3,1,4,2,3,2,4,1,5,2,2,3,4,3,15,196,2,1,1,1,2,1,2,3,2,1,2,4,2,2,1,5,3,2,6,3,7,3,4,43,5,59,41,5,1,2,1,10,5,296,5,27,8,7,13,12,
            9,9,8,321,1,1,2,2,1,7,2,4,2,8,2,4,2,4,1,5,21,2,10,15,39,18,3,9,9,1,3,3,4,54,5,13,27,21,47,5,21,6,24209,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,5,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,2,5,2,1,150,1,1,2,1,1,1,1,1,2,1,1,1,11,1,4,1,27,1,1,2,28,2
        };
        static ImWchar base_ranges[] = // not zero-terminated
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x2000, 0x206F, // General Punctuation
            0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
            0x31F0, 0x31FF, // Katakana Phonetic Extensions
            0xFF00, 0xFFEF, // Half-width characters
            0xFFFD, 0xFFFD  // Invalid
        };
        static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
        if (!full_ranges[0])
        {
            memcpy(full_ranges, base_ranges, sizeof(base_ranges));
            // Copied from UnpackAccumulativeOffsetsIntoRanges() in imgui_draw.cpp
            int base_codepoint = 0x4E00;
            const short* accumulative_offsets = accumulative_offsets_from_0x4E00;
            const int accumulative_offsets_count = IM_ARRAYSIZE(accumulative_offsets_from_0x4E00);
            ImWchar* out_ranges = full_ranges + IM_ARRAYSIZE(base_ranges);
            for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
            {
                out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
                base_codepoint += accumulative_offsets[n];
            }
            out_ranges[0] = 0;
        }
        glyph_ranges = &full_ranges[0];
    } else if (lang_name == "Chinese") {
        glyph_ranges = atlas->GetGlyphRangesChineseFull();
    } else if (lang_name == "Korean") {
        glyph_ranges = atlas->GetGlyphRangesKorean();
    } else if (lang_name == "Japanese") {
        glyph_ranges = atlas->GetGlyphRangesJapanese();
    } else if (lang_name == "Cyrillic") {
        glyph_ranges = atlas->GetGlyphRangesCyrillic();
    } else if (lang_name == "Greek") {
        glyph_ranges = atlas->GetGlyphRangesGreek();
    } else if (lang_name == "Thai") {
        glyph_ranges = atlas->GetGlyphRangesThai();
    } else if (lang_name == "Vietnamese") {
        glyph_ranges = atlas->GetGlyphRangesVietnamese();
    } else if (lang_name == "Polish") {
        static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x0100, 0x017F, 0 };
        glyph_ranges = ranges;
    } else if (lang_name == "FontAwesome6") {
        static const ImWchar ranges[] = { 0xe005, 0xf8ff, 0 };
        glyph_ranges = ranges;
        imfc.MergeMode = true;
        imfc.PixelSnapH = true;
        imfc.GlyphMinAdvanceX = size;
    }
    auto font = atlas->AddFontFromMemoryTTF(mb, (int)buf.size(), size, &imfc, glyph_ranges);
    return font != nullptr;
}

pair<LString *, bool> LStringInputText(VM &vm, const char *label, LString *str, int num_lines = 1, int extra_flags = 0) {
    struct InputTextCallbackData {
        LString *str;
        VM &vm;
        static int InputTextCallback(ImGuiInputTextCallbackData *data) {
            auto cbd = (InputTextCallbackData *)data->UserData;
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                IM_ASSERT(data->Buf == cbd->str->data());
                auto str = cbd->vm.NewString(string_view{ data->Buf, (size_t)data->BufTextLen });
                cbd->str->Dec(cbd->vm);
                cbd->str = str;
                data->Buf = (char *)str->data();
            } else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
                // If we get here the widget changed the buffer, so important we
                // do nothing and do not overwrite.
            } else if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways) {
                if (strcmp(cbd->str->data(), data->Buf)) {
                    // If we get here, the program is trying to overwrite the widget text.
                    // https://github.com/ocornut/imgui/issues/5377
                    auto cursor = data->CursorPos;
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, cbd->str->data());
                    if (cursor <= data->BufTextLen) {
                        // For algorithmic changes to the input data, nice to not lose cursor pos,
                        // though for switching unrelated data would be better not?
                        data->CursorPos = cursor;
                    }
                }
            }
            return 0;
        }
    };
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize |
                                ImGuiInputTextFlags_CallbackEdit |
                                ImGuiInputTextFlags_CallbackAlways |
                                extra_flags;
    InputTextCallbackData cbd { str, vm };
    bool retval = false;
    if (num_lines > 1) {
        retval = ImGui::InputTextMultiline(label, (char *)str->data(), str->len + 1,
            ImVec2(ImGui::CalcItemWidth(), ImGui::GetTextLineHeight() * num_lines), flags,
            InputTextCallbackData::InputTextCallback, &cbd);
    } else {
        retval = ImGui::InputText(label, (char *)str->data(), str->len + 1, flags,
            InputTextCallbackData::InputTextCallback, &cbd);
    }
    return { cbd.str, retval };
}

double InputFloat(const char *label, double value, double step = 0, double step_fast = 0, ImGuiInputTextFlags flags = 0) {
    ImGui::InputDouble(label, &value, step, step_fast, "%.3f", flags);
    return value;
}

bool BeginTable(const char *id) {
    if (ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter)) {
        ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthStretch);
        return true;
    }
    return false;
}

void EndTable() {
    ImGui::EndTable();
}

void Text(string_view s) {
    ImGui::TextUnformatted(s.data(), s.data() + s.size());
}

void Nil(VM &vm, Value *v, const TypeInfo *ti) {
    Text("nil");
    ImGui::SameLine();
    if (v && ImGui::Button("+")) {
        switch (ti->t) {
            case RTT_STRING:
                *v = vm.NewString(0);
                break;
            case RTT_VECTOR:
                *v = vm.NewVec(0, 0, vm.TypeInfoToIdx(ti));
                break;
            case RTT_CLASS: {
                Deserializer des(vm);
                if (des.PushDefault(vm.TypeInfoToIdx(ti), (type_elem_t)0, nullptr)) {
                    *v = des.PopV();
                }
                break;
            }
        }
    }
}

void VectorOps(VM &vm, LVector *vec, const TypeInfo *ti) {
    ImGui::SameLine();
    if (ImGui::Button("+")) {
        Deserializer des(vm);
        if (des.PushDefault(ti->subt, (type_elem_t)0, nullptr)) {
            vec->Push(vm, des.PopV());
        }
    }
    if (vec->len) {
        ImGui::SameLine();
        if (ImGui::Button("-")) {
            auto e = vec->Pop();
            e.LTDECTYPE(vm, vm.GetTypeInfo(ti->subt).t);
        }
    }
}

void ValToGUI(VM &vm, Value *v, const TypeInfo *ti, string_view_nt label, bool expanded, bool in_table = true) {
    if (in_table) {
        // Early out for types that don't make sense to display.
        if (ti->t == RTT_FUNCTION) return;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        Text(label.sv);
        ImGui::TableSetColumnIndex(1);
        // This seems to be the only way to make the second column stretch to the available space without it
        // collapsing to nothing in some cases: https://github.com/ocornut/imgui/issues/5478
        ImGui::SetNextItemWidth(std::max(200.0f, ImGui::GetContentRegionAvail().x));
        ImGui::PushID(v);  // Name may occur multiple times.
        label = string_view_nt("");
    }
    auto l = label.c_str();
    auto flags = expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    switch (ti->t) {
        case RTT_INT: {
            if (ti->enumidx == 0) {
                assert(vm.EnumName(ti->enumidx) == "bool");
                bool b = v->True();
                if (ImGui::Checkbox(l, &b)) *v = b;
            } else if (ti->enumidx >= 0) {
                int val = v->intval();
                int sel = 0;
                auto &vals = vm.vma.meta->enums[ti->enumidx].vals;
                vector<const char *> items(vals.size());
                int i = 0;
                for (auto &ev : vals) {
                    items[i] = ev.name.data();
                    if (val == ev.val) sel = i;
                    i++;
                }
                ImGui::Combo(l, &sel, items.data(), (int)items.size());
                *v = vals[sel].val;
            } else {
                iint i = v->ival();
                if (ImGui::InputScalar(l, ImGuiDataType_S64, (void *)&i, nullptr, nullptr, "%" PRId64, 0)) *v = i;
            }
            break;
        }
        case RTT_FLOAT: {
            double f = v->fval();
            if (ImGui::InputDouble(l, &f)) *v = f;
            break;
        }
        case RTT_VECTOR:
            if (v->False()) {
                Nil(vm, v, ti);
                break;
            }
            if (!v->vval()->len) {
                Text("[]");
                VectorOps(vm, v->vval(), ti);
                break;
            }
            if (ImGui::TreeNodeEx(*l ? l : "[..]", flags)) {
                VectorOps(vm, v->vval(), ti);
                if (BeginTable("[]")) {
                    auto &sti = vm.GetTypeInfo(ti->subt);
                    auto vec = v->vval();
                    for (iint i = 0; i < vec->len; i++) {
                        ValToGUI(vm, vec->AtSt(i), &sti, to_string(i), false);
                    }
                    EndTable();
                }
                ImGui::TreePop();
            }
            break;
        case RTT_CLASS:
            if (v->False()) {
                Nil(vm, v, ti);
                break;
            }
            // Upgrade to dynamic type if maybe subclass.
            ti = &v->oval()->ti(vm);
            v = v->oval()->Elems();  // To iterate it like a struct.
        case RTT_STRUCT_R:
        case RTT_STRUCT_S: {
            auto &st = vm.vma.meta->udts[ti->structidx];
            // Special case for numeric structs & colors.
            if (ti->len >= 2 && ti->len <= 4) {
                for (int i = 1; i < ti->len; i++)
                    if (ti->elemtypes[i].type != ti->elemtypes[0].type) goto generic;
                if (ti->elemtypes[0].type == TYPE_ELEM_INT) {
                    auto nums = ValueToI<4>(v, ti->len);
                    if (ImGui::InputScalarN(
                            l, ImGuiDataType_S64,
                            (void *)nums.data(), ti->len, NULL, NULL, "%d", flags)) {
                        ToValue(v, ti->len, nums);
                    }
                    break;
                } else if (ti->elemtypes[0].type == TYPE_ELEM_FLOAT) {
                    if (st.name == "color") {
                        auto c = ValueToFLT<4>(v, ti->len);
                        if (ImGui::ColorEdit4(l, (float *)c.data())) {
                            ToValue(v, ti->len, c);
                        }
                    } else {
                        auto nums = ValueToF<4>(v, ti->len);
                        // FIXME: format configurable.
                        if (ImGui::InputScalarN(
                                l,
                                sizeof(double) == sizeof(float) ? ImGuiDataType_Float
                                                                : ImGuiDataType_Double,
                                (void *)nums.data(), ti->len, NULL, NULL, "%.3f", flags)) {
                            ToValue(v, ti->len, nums);
                        }
                    }
                    break;
                }
            }
            generic:
            if (ImGui::TreeNodeEx(*l ? l : st.name.data(), flags)) {
                if (BeginTable(st.name.data())) {
                    int fi = 0;
                    for (int i = 0; i < ti->len; i++) {
                        auto &sti = vm.GetTypeInfo(ti->GetElemOrParent(i));
                        ValToGUI(vm, v + i, &sti, string_view_nt(st.fields[fi++].name),
                                    false);
                        if (RTIsStruct(sti.t)) i += sti.len - 1;
                    }
                    EndTable();
                }
                ImGui::TreePop();
            }
            break;
        }
        case RTT_STRING: {
            if (v->False()) {
                Nil(vm, v, ti);
                break;
            }
            *v = LStringInputText(vm, l, v->sval()).first;
            break;
        }
        case RTT_RESOURCE: {
            if (v->False()) {
                Nil(vm, nullptr, ti);
                break;
            }
            auto r = v->xval();
            // This would be awesome, but can't work as-is because the texture is in a different
            // context, to fix this, we'd need a vendor specific call like
            // https://registry.khronos.org/OpenGL/extensions/NV/NV_copy_image.txt
            // or a simple thing like switching context, glReadPixels etc, which will be super
            // slow.
            /*
            if (r->type == &texture_type) {
                auto t = (OwnedTexture *)r->res;
                auto tex = (ImTextureID)(size_t)t->t.id;
                auto sz = float2(t->t.size.xy()) / 4.0f;
                ImGui::Image(tex, ImVec2(sz.x, sz.y));
                break;
            }
            */
            string sd;
            auto mu = r->res->MemoryUsage();
            append(sd, "resource<", r->type->name, "> (", mu.x + mu.y, " bytes) ");
            r->res->Dump(sd);
            Text(sd);
            break;
        }
        default:
            string sd;
            v->ToString(vm, sd, *ti, vm.debugpp);
            Text(sd);
            break;
    }
    if (in_table) {
        ImGui::PopID();
        //ImGui::PopItemWidth();
    }
}

void VarsToGUI(VM &vm) {
    auto DumpVars = [&](bool constants) {
        if (BeginTable("vars")) {
            for (uint32_t i = 0; i < vm.vma.meta->specidents.size(); i++) {
                auto &val = vm.fvars[i];
                auto &sid = vm.vma.meta->specidents[i];
                if (!sid.global || sid.readonly != constants) continue;
                auto name = string_view_nt(sid.name);
                auto &ti = vm.GetVarTypeInfo(i);
                #if RTT_ENABLED
                if (ti.t != val.type) continue;  // Likely uninitialized.
                #endif
                ValToGUI(vm, &val, &ti, name, false);
                if (RTIsStruct(ti.t)) i += ti.len - 1;
            }
            EndTable();
        }
    };
    if (ImGui::TreeNodeEx("Globals", 0)) {
        DumpVars(false);
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Constants", 0)) {
        DumpVars(true);
        ImGui::TreePop();
    }
}

void EngineStatsGUI() {
    auto &ft = SDLGetFrameTimeLog();
    ImGui::PlotLines("gl.deltatime", ft.data(), (int)ft.size());
}

void FlexBufferGUI(flexbuffers::Reference r, const char *label, bool in_table) {
    if (in_table) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        Text(label);
        ImGui::TableSetColumnIndex(1);
        // This seems to be the only way to make the second column stretch to the available space
        // without it collapsing to nothing in some cases:
        // https://github.com/ocornut/imgui/issues/5478
        ImGui::SetNextItemWidth(std::max(200.0f, ImGui::GetContentRegionAvail().x));
        ImGui::PushID(label);  // Name may occur multiple times.
    }
    switch (r.GetType()) {
        case flexbuffers::FBT_MAP: {
            if (ImGui::TreeNodeEx("{..}", 0)) {
                if (BeginTable(label)) {
                    auto m = r.AsMap();
                    auto keys = m.Keys();
                    auto vals = m.Values();
                    for (size_t i = 0; i < keys.size(); i++) {
                        string key;
                        keys[i].ToString(true, false, key);
                        FlexBufferGUI(vals[i], key.c_str(), true);
                    }
                    EndTable();
                }
                ImGui::TreePop();
            }
            break;
        }
        case flexbuffers::FBT_VECTOR: {
            if (ImGui::TreeNodeEx("[..]", 0)) {
                if (BeginTable(label)) {
                    auto vals = r.AsVector();
                    for (size_t i = 0; i < vals.size(); i++) {
                        auto labeli = cat(i);
                        FlexBufferGUI(vals[i], labeli.c_str(), true);
                    }
                    EndTable();
                }
                ImGui::TreePop();
            }
            break;
        }
        default: {
            auto s = r.ToString();
            ImGui::LabelText(label, "%s", s.c_str());
            break;
        }
    }
    if (in_table) {
        ImGui::PopID();
    }
}

// See also VM::DumpStackTrace and VM::DumpStackTraceMemory()
void DumpStackTrace(VM &vm) {
    if (vm.fun_id_stack.empty()) return;

    VM::DumperFun dumper = [](VM &vm, string_view_nt name, const TypeInfo &ti, Value *x) {
        #if RTT_ENABLED
            auto debug_type = x->type;
        #else
            auto debug_type = ti.t;
        #endif
        if (debug_type == RTT_NIL && !ti.is_nil) {
            // Uninitialized.
            auto sd = string(name.sv);
            append(sd, ":");
            ti.Print(vm, sd, nullptr);
            append(sd, " (uninitialized)");
            Text(sd);
        } else if (ti.t != debug_type && !RTIsStruct(ti.t)) {
            // Some runtime type corruption, show the problem rather than crashing.
            auto sd = string(name.sv);
            append(sd, ":");
            ti.Print(vm, sd, nullptr);
            append(sd, " (ERROR != ", BaseTypeName(debug_type), ")");
            Text(sd);
        } else {
            ValToGUI(vm, x, &ti, name, false);
        }
    };

    auto cur_fileidx = vm.last.fileidx;
    auto cur_line = vm.last.line;
    int i = 0;
    for (auto &funstackelem : reverse(vm.fun_id_stack)) {
        auto [name, fip] = vm.DumpStackFrameStart(funstackelem.funstartinfo, cur_fileidx, cur_line);
        ImGui::PushID(i++);
        if (ImGui::TreeNode(name.c_str())) {
            if (BeginTable(name.c_str())) {
                vm.DumpStackFrame(fip, funstackelem.locals, dumper);
                EndTable();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
        cur_fileidx = funstackelem.fileidx;
        cur_line = funstackelem.line;
    }
}

string BreakPoint(VM &vm, string_view reason) {
    // Init just in case it wasn't already.
    // NOTE: this inits on main window first.
    if (!IMGUIInit(0, false, 3.0f, 1.0f)) {
        // FIXME: we could make this work without a main window, but I guess if we're running
        // in console its good to stay there?
        return "breakpoint: no main window for imgui";
    }

    auto cursor_was_on = SDLCursor(true);

    auto err = SDLDebuggerWindow();
    if (!err.empty()) return "Couldn\'t create debugger: " + err;

    auto existing_context = ImGui::GetCurrentContext();
    // FIXME: this is supposed to be able to share the font atlas with the other context,
    // but that seems to destroy it when the debugger context gets destroyed.
    ImGuiContext *debugger_imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(debugger_imgui_context);

    ImGui_ImplSDL2_InitForOpenGL(_sdl_debugger_window, _sdl_debugger_context);
    ImGui_ImplOpenGL3_Init("#version 150");

    // Set our own font.. would be better to inherit the one from the game.
    LoadFont("data/fonts/Droid_Sans/DroidSans.ttf", 16.0, "Default");

    bool quit = false;
    int cont = 0;
    bool first_stack_trace = true;
    for (;;) {
        quit = SDLDebuggerFrame();
        if (quit) break;

        ClearFrameBuffer(float3(0.5f));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::GetStyle().WindowRounding = 0.0f;
        ImGui::Begin("Lobster Debugger", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

        if (cont) {
            ImGui::Text("Program Running, debugger inactive");
            first_stack_trace = true;
            // Ensure we've rendered a full frame with the above text before aborting,
            // since this window will get no rendering updates.
            if (++cont == 3) break;
        } else {
            Text(reason);
            if (ImGui::Button("Continue")) {
                cont = 1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Quit")) {
                quit = true;
                break;
            }

            if (first_stack_trace) {
                ImGui::SetNextItemOpen(true);
                first_stack_trace = false;
            }
            DumpStackTrace(vm);
            VarsToGUI(vm);

            if (ImGui::TreeNode("Memory Usage")) {
                // FIXME: imgui-ify? Table?
                auto mu = vm.MemoryUsage(25);
                Text(mu);
                ImGui::TreePop();
            }
        }

        ImGui::End();

        ImGui::Render();
        SetTexture(0, DummyTexture());
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::SetCurrentContext(existing_context);
    ImGui::DestroyContext(debugger_imgui_context);

    SDLDebuggerOff();

    if (quit) vm.NormalExit("Program terminated by debugger");

    SDLCursor(cursor_was_on);

    return "";
}

int &ListState(int &cur, const char* label, iint max, int def) {
    auto pcur = &cur;
    if (cur < -1) {
        auto id = ImGui::GetID(label);
        auto &ls = list_state[id];
        if (ls.sel < 0) ls.sel = def;
        ls.last_use = imgui_frame;
        pcur = &ls.sel;
    }
    // Convenient: user code doesn't need to keep track of changes in size.
    *pcur = min(*pcur, (int)max - 1);
    return *pcur;
}

const char *Label(VM &vm, Value val) {
    auto l = val.sval()->data();
    if (!*l)
        vm.BuiltinError("imgui: widget label must not be empty (use ##something for empty label)");
    return l;
}

#if LOBSTER_FRAME_PROFILER == 1
static float prof_histogram_values_getter(void *data, int idx) {
    return ((float *)data)[idx] * 1000.0f;
}
#endif

void AddIMGUI(NativeRegistry &nfr) {

nfr("init", "dark_style,flags,rounding,border", "B?I?F?F?", "",
    "",
    [](StackPtr &, VM &vm, Value darkstyle, Value flags, Value rounding, Value border) {
        if (!IMGUIInit(flags.ival(), darkstyle.True(), rounding.fltval(), border.fltval()))
            vm.BuiltinError("im.init: no window");
        return NilVal();
    });

nfr("cleanup", "", "", "",
    "not required to be called, this is done automatically, but useful if you want to call im.init again",
    [](StackPtr &, VM &vm) {
        IMGUICleanup();
        return NilVal();
    });

nfr("add_font", "font_path,size,glyph_ranges", "SFS?", "B",
    "glyph_ranges will activate additional unicode ranges to be rasterized, and can be"
    " Default (most European languages), SimplifiedChinese, Chinese, Japanese, Korean, Cyrillic, Thai, Vietnamese, Greek, ..",
    [](StackPtr &, VM &vm, Value fontname, Value size, Value glyph_ranges) {
        IsInit(vm, { N_NONE, N_NONE });
        return Value(LoadFont(fontname.sval()->strv(), size.fltval(),
                              glyph_ranges.True() ? glyph_ranges.sval()->strv() : "Default"));
    });

nfr("clear_fonts", "", "", "",
    "allows you to restart your add_font calls",
    [](StackPtr &, VM &vm) {
        IsInit(vm,  { N_NONE, N_NONE });
        ClearFonts();
        //ImGui_ImplOpenGL3_DestroyFontsTexture();
    });

nfr("set_style_color", "i,color", "IF}:4", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm,  { N_NONE, N_NONE });
        auto c = PopVec<float4>(sp);
        auto i = Pop(sp).intval();
        if (i < 0 || i >= ImGuiCol_COUNT) return;
        ImGui::GetStyle().Colors[i] = ImVec4(c.x, c.y, c.z, c.w);
    });

nfr("set_style_spacing", "spacing", "F}:2", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm,  { N_NONE, N_NONE });
        auto s = PopVec<float2>(sp);
        ImGui::GetStyle().ItemSpacing = ImVec2(s.x, s.y);
    });

nfr("set_style_inner_spacing", "spacing", "F}:2", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm,  { N_NONE, N_NONE });
        auto s = PopVec<float2>(sp);
        ImGui::GetStyle().ItemInnerSpacing = ImVec2(s.x, s.y);
    });

nfr("set_style_window_padding", "spacing", "F}:2", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm,  { N_NONE, N_NONE });
        auto s = PopVec<float2>(sp);
        ImGui::GetStyle().WindowPadding = ImVec2(s.x, s.y);
    });

nfr("set_style_frame_padding", "spacing", "F}:2", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm,  { N_NONE, N_NONE });
        auto s = PopVec<float2>(sp);
        ImGui::GetStyle().FramePadding = ImVec2(s.x, s.y);
    });

nfr("set_style_tabs_extra_padding", "spacing", "F}:2", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm,  { N_NONE, N_NONE });
        auto s = PopVec<float2>(sp);
        tabs_extra_padding = ImVec2(s.x, s.y);
    });

nfr("frame_start", "", "", "",
    "(use im.frame instead)",
    [](StackPtr &, VM &vm) {
        IsInit(vm, { N_NONE, N_NONE });
        IMGUIFrameCleanup();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        NPush(N_FRAME);
        imgui_frame++;
    });

nfr("frame_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm, { N_NONE, N_NONE });
        NPop(vm, N_FRAME);
        for (auto it = list_state.cbegin(); it != list_state.cend(); ) {
            if (it->second.last_use != imgui_frame) {
                it = list_state.erase(it);
            } else {
                ++it;
            }
        }
    });

nfr("dockspace_over_viewport", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
    ImGui::DockSpaceOverViewport(
        0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
    });

nfr("window_demo", "", "", "B",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        bool show = true;
        ImGui::ShowDemoWindow(&show);
        return Value(show);
    });

nfr("window_start", "title,flags,dock", "SII", "B",
    "(use im.window instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto dock = Pop(sp);
        auto flags = Pop(sp);
        auto title = Pop(sp);
        if (dock.True() && ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::SetNextWindowDockID(last_dock_id, ImGuiCond_FirstUseEver);
        }
        bool open = ImGui::Begin(Label(vm, title), nullptr, (ImGuiWindowFlags)flags.ival());
        last_dock_id = ImGui::GetWindowDockID();
        /*
        if (dock.False() && ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::DockSpace(last_dock_id);
        }
        */
        Push(sp, open);
        // Unlike other begin/end pairs, we must always call end unconditionally, so we always push this.
        NPush(N_WIN);
    });

nfr("window_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        NPop(vm, N_WIN);
    });

nfr("next_window_size", "size,cond", "F}:2I", "",
    "size in pixels",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto cond = Pop(sp).ival();
        auto size = PopVec<float2>(sp);
        ImGui::SetNextWindowSize(ImVec2(size.x, size.y), (ImGuiCond)cond);
    });

nfr("next_window_pos", "pos,pivot,cond", "F}:2F}:2I", "",
    "pos in pixels, pivot values 0..1 relative to pos",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto cond = Pop(sp).ival();
        auto pivot = PopVec<float2>(sp);
        auto pos = PopVec<float2>(sp);
        ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), (ImGuiCond)cond, ImVec2(pivot.x, pivot.y));
    });

nfr("button", "label,size", "SF}:2?", "B",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto size = PopVec<float2>(sp);
        auto title = Pop(sp);
        auto press = ImGui::Button(Label(vm, title), ImVec2(size.x, size.y));
        Push(sp, press);
    });

nfr("button_small", "label", "S", "B",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto title = Pop(sp);
        auto press = ImGui::SmallButton(Label(vm, title));
        Push(sp, press);
    });

nfr("selectable", "label,selected", "SB?", "B",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto selected = Pop(sp).True();
        auto title = Pop(sp);
        auto press = ImGui::Selectable(Label(vm, title), selected);
        Push(sp, press);
    });

nfr("same_line", "", "", "", "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::SameLine();
        return NilVal();
    });

nfr("new_line", "", "", "", "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::NewLine();
    });

nfr("separator", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::Separator();
        return NilVal();
    });

nfr("spacing", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::Spacing();
        return NilVal();
    });

nfr("is_item_deactivated_after_edit", "", "", "B",
    "returns true if the last item was made inactive and made a value change when it was active",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto result = ImGui::IsItemDeactivatedAfterEdit();
        Push(sp, result);
    });

nfr("is_item_focused", "", "", "B",
    "returns true if the last item is focused",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto result = ImGui::IsItemFocused();
        Push(sp, result);
    });

nfr("set_item_default_focus", "", "", "", "sets the last item created to have focus",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::SetItemDefaultFocus();
    });

nfr("set_keyboard_focus_here", "", "", "", "sets the last item to have keyboard focus",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        // TODO: allow offset parameter?
        ImGui::SetKeyboardFocusHere(-1);
    });

nfr("get_scroll_y", "", "", "F", "get the y scroll amount in the range [0, get_scroll_max_y()]",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        Push(sp, ImGui::GetScrollY());
    });

nfr("get_scroll_max_y", "", "", "F", "get the maximum y scroll amount",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        Push(sp, ImGui::GetScrollMaxY());
    });

nfr("set_scroll_here_x", "center_x_ratio", "F", "",
    "scroll the pane in the X dimension so the imgui cursor is visible. Use 0.5 to center the pane on the item",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto center_x_ratio = Pop(sp).fltval();
        ImGui::SetScrollHereX(center_x_ratio);
    });

nfr("set_scroll_here_y", "center_y_ratio", "F", "",
    "scroll the pane in the Y dimension so the imgui cursor is visible. Use 0.5 to center the pane on the item",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto center_y_ratio = Pop(sp).fltval();
        ImGui::SetScrollHereY(center_y_ratio);
    });

nfr("want_capture_mouse", "", "", "B",
    "returns true if imgui wants to capture the mouse",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_NONE, N_NONE });
        auto result = ImGui::GetIO().WantCaptureMouse;
        Push(sp, result);
    });

nfr("get_modifiers", "", "", "BBB",
    "returns the status of the shift, ctrl and alt keys",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_NONE, N_NONE });
        Push(sp, ImGui::GetIO().KeyShift);
        Push(sp, ImGui::GetIO().KeyCtrl);
        Push(sp, ImGui::GetIO().KeyAlt);
    });

nfr("get_layout_pos", "", "", "F}:2", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto pos = ImGui::GetCursorPos();
        PushVec(sp, float2(pos.x, pos.y));
    });

nfr("set_layout_pos", "pos", "F}:2", "", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto pos = PopVec<float2>(sp);
        ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
    });

nfr("get_layout_screen_pos", "", "", "F}:2", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto pos = ImGui::GetCursorScreenPos();
        PushVec(sp, float2(pos.x, pos.y));
    });

nfr("set_layout_screen_pos", "pos", "F}:2", "", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto pos = PopVec<float2>(sp);
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
    });

nfr("get_content_region_avail", "", "", "F}:2",
    "returns the amount of space left in the current region from the cursor pos",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto avail = ImGui::GetContentRegionAvail();
        PushVec(sp, float2(avail.x, avail.y));
    });

nfr("calc_text_size", "text", "S", "F}:2",
    "returns the amount of space used by the given text in the current font",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto &text = *Pop(sp).sval();
        auto size = ImGui::CalcTextSize(text.strvnt().c_str());
        PushVec(sp, float2(size.x, size.y));
    });

nfr("calc_word_wrap_position", "text,width", "SF", "I",
    "returns the wrap point of the given text in the current font. If width is 0 then use the current region's available width",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto width = Pop(sp).fltval();
        auto &text = *Pop(sp).sval();
        auto text_sv = text.strvnt();
        auto *ctext = text_sv.c_str();
        auto *ctext_end = ctext + text_sv.size();
        auto font_size = ImGui::GetFontSize();
        if (width == 0) {
            width = ImGui::GetContentRegionAvail().x;
        }
        auto *wrap_ctext = ImGui::GetFont()->CalcWordWrapPosition(font_size, ctext, ctext_end, width);
        Push(sp, wrap_ctext - ctext);
    });

nfr("calc_item_width", "", "", "F",
    "returns the width of an item given the current cursor position",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto width = ImGui::CalcItemWidth();
        Push(sp, width);
    });

nfr("mouse_clicked", "button", "I", "B",
    "returns whether the given mouse button was clicked anywhere",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto button = Pop(sp).ival();
        Push(sp, Value(ImGui::IsMouseClicked((ImGuiMouseButton)button)));
    });

nfr("text", "label", "S", "",
    "",
    [](StackPtr &, VM &vm, Value text) {
        IsInit(vm);
        auto &s = *text.sval();
        Text(s.strv());
        return NilVal();
    });

nfr("text_wrapped", "label", "S", "",
    "",
    [](StackPtr &, VM &vm, Value text) {
        IsInit(vm);
        auto &s = *text.sval();
        ImGui::TextWrapped("%s", s.strvnt().c_str());
        return NilVal();
    });

nfr("text_bullet", "label", "S", "",
    "",
    [](StackPtr &, VM &vm, Value text) {
        IsInit(vm);
        auto &s = *text.sval();
        ImGui::BulletText("%s", s.data());
        return NilVal();
    });

nfr("indent_start", "amount", "F", "",
    "(use im.indent instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto amount = Pop(sp).fltval();
        ImGui::Indent(amount);
        NPush(N_INDENT);
        indent_stack.push_back(amount);
    });

nfr("indent_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_INDENT);
    });

nfr("font_start", "font_idx", "I", "",
    "(use im.font instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        int i = std::max(0, std::min(ImGui::GetIO().Fonts->Fonts.size() - 1, Pop(sp).intval()));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[i]);
        NPush(N_FONT);
    });

nfr("font_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_FONT);
    });

nfr("color_start", "color", "F}:4", "",
    "(use im.color instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto c = PopVec<float4>(sp);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(c.x, c.y, c.z, c.w));
        NPush(N_COLOR);
    });

nfr("color_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_COLOR);
    });

nfr("tooltip", "label", "S", "",
    "",
    [](StackPtr &, VM &vm, Value text) {
        IsInit(vm);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", Label(vm, text));
        return NilVal();
    });

nfr("tooltip_multi_start", "", "", "B",
    "(use im.tooltip_multi instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::BeginTooltip();  // The bool it returns is currently always true?
            Push(sp, true);
            NPush(N_TOOLTIP);
        } else {
            Push(sp, false);
        }
    });

nfr("tooltip_multi_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_TOOLTIP);
    });

nfr("checkbox", "label,bool", "SI", "I2",
    "",
    [](StackPtr &, VM &vm, Value text, Value boolval) {
        IsInit(vm);
        bool b = boolval.True();
        ImGui::Checkbox(Label(vm, text), &b);
        return Value(b);
    });

nfr("input_text", "label,str", "SSk", "S",
    "",
    [](StackPtr &, VM &vm, Value text, Value str) {
        IsInit(vm);
        return Value(LStringInputText(vm, Label(vm, text), str.sval()).first);
    });

nfr("input_text", "label,str,flags", "SSkI", "SB",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto extra_flags = Pop(sp).ival();
        auto str = Pop(sp).sval();
        auto text = Pop(sp);
        // Don't allow setting any of the callback flags.
        extra_flags &= ~(
            ImGuiInputTextFlags_CallbackCompletion |
            ImGuiInputTextFlags_CallbackHistory |
            ImGuiInputTextFlags_CallbackAlways |
            ImGuiInputTextFlags_CallbackCharFilter |
            ImGuiInputTextFlags_CallbackResize |
            ImGuiInputTextFlags_CallbackEdit
        );
        auto [newstr, enter] = LStringInputText(vm, Label(vm, text), str, 1, (int)extra_flags);
        Push(sp, Value(newstr));
        Push(sp, enter);
    });

nfr("input_text_multi_line", "label,str,num_lines,flags", "SSkII?", "S",
    "",
    [](StackPtr &, VM &vm, Value text, Value str, Value num_lines, Value flags) {
        IsInit(vm);
        return Value(LStringInputText(vm, Label(vm, text), str.sval(), num_lines.intval(), flags.intval()).first);
    });

nfr("input_int", "label,val,min,max", "SIII", "I",
    "",
    [](StackPtr &, VM &vm, Value text, Value val, Value min, Value max) {
        IsInit(vm);
        ImGui::InputScalar(Label(vm, text), ImGuiDataType_S64, (void *)&val,
                           nullptr, nullptr, "%" PRId64, 0);
        if (val.ival() < min.ival()) val = min;
        if (val.ival() > max.ival()) val = max;
        return val;
    });

nfr("input_int", "label,val", "SI", "I",
    "",
    [](StackPtr &, VM &vm, Value text, Value val) {
        IsInit(vm);
        ImGui::InputScalar(Label(vm, text), ImGuiDataType_S64, (void *)&val,
                           nullptr, nullptr, "%" PRId64, 0);
        return val;
    });

nfr("input_float", "label,val", "SF", "F",
    "",
    [](StackPtr &, VM &vm, Value text, Value val) {
        IsInit(vm);
        return Value(InputFloat(Label(vm, text), val.fval()));
    });

nfr("radio", "labels,active,horiz", "S]II", "I",
    "active to select which one is activated, -2 for last frame\'s "
    "selection or 0",
    [](StackPtr &, VM &vm, Value strs, Value active, Value horiz) {
        IsInit(vm);
        auto v = strs.vval();
        auto act = active.intval();
        int &sel = ListState(act, v->len ? Label(vm, v->AtS(0)) : "empty?", v->len, 0);
        for (iint i = 0; i < v->len; i++) {
            if (i && horiz.True()) ImGui::SameLine();
            ImGui::RadioButton(Label(vm, v->AtS(i)), &sel, (int)i);
        }
        return Value(sel);
    });

nfr("progress_bar", "fraction,size,overlay", "FF}:2S", "",
    "display progress bar filled up to the given fraction. size.x < 0 to use all available space, "
    " size.x > 0 for a specific pixel width",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto &overlay = *Pop(sp).sval();
        auto size = PopVec<float2>(sp);
        auto fraction = Pop(sp).fltval();
        ImGui::ProgressBar(fraction, ImVec2(size.x, size.y), overlay.strvnt().c_str());
    });

nfr("combo", "label,labels,active", "SS]I", "I",
    "active to select which one is activated, -2 for last frame\'s "
    "selection or 0",
    [](StackPtr &, VM &vm, Value text, Value strs, Value active) {
        IsInit(vm);
        auto v = strs.vval();
        auto act = active.intval();
        int &sel = ListState(act, Label(vm, text), v->len, 0);
        vector<const char *> items(v->len);
        for (iint i = 0; i < v->len; i++) {
            items[i] = Label(vm, v->AtS(i));
        }
        ImGui::Combo(Label(vm, text), &sel, items.data(), (int)items.size());
        return Value(sel);
    });

nfr("listbox", "label,labels,active,height", "SS]II", "I",
    "active to select which one is activated, -1 for no initial selection, -2 for last frame\'s "
    "selection or none",
    [](StackPtr &, VM &vm, Value text, Value strs, Value active, Value height) {
        IsInit(vm);
        auto v = strs.vval();
        auto act = active.intval();
        int &sel = ListState(act, Label(vm, text), v->len, -1);
        vector<const char *> items(v->len);
        for (iint i = 0; i < v->len; i++) {
            items[i] = v->AtS(i).sval()->data();
        }
        ImGui::ListBox(Label(vm, text), &sel, items.data(), (int)items.size(), height.intval());
        return Value(sel);
    });

nfr("sliderint", "label,i,min,max", "SIII", "I",
    "",
    [](StackPtr &, VM &vm, Value text, Value integer, Value min, Value max) {
        IsInit(vm);
        int i = integer.intval();
        ImGui::SliderInt(Label(vm, text), &i, min.intval(), max.intval());
        return Value(i);
    });

nfr("sliderfloat", "label,f,min,max", "SFFF", "F",
    "",
    [](StackPtr &, VM &vm, Value text, Value flt, Value min, Value max) {
        IsInit(vm);
        float f = flt.fltval();
        ImGui::SliderFloat(Label(vm, text), &f, min.fltval(), max.fltval());
        return Value(f);
    });

    #define VECSLIDER(Type, type, typeval, T, N)                                               \
    nfr("slider" #type #N, "label,"#T #N ",min,max", "S" #T "}:" #N #T #T, #T "}:" #N, "",  \
        [](StackPtr &sp, VM &vm) {                                                             \
            IsInit(vm);                                                                        \
            auto max = Pop(sp).typeval();                                                      \
            auto min = Pop(sp).typeval();                                                      \
            auto v = PopVec<type ## N>(sp);                                                    \
            auto label = Label(vm, Pop(sp));                                                   \
            ImGui::Slider ## Type ## N(label, &v.c[0], min, max);                              \
            PushVec(sp, v);                                                                    \
        }) // no semicolon.
    VECSLIDER(Int, int, intval, I, 2);
    VECSLIDER(Int, int, intval, I, 3);
    VECSLIDER(Int, int, intval, I, 4);
    VECSLIDER(Float, float, fltval, F, 2);
    VECSLIDER(Float, float, fltval, F, 3);
    VECSLIDER(Float, float, fltval, F, 4);
    #undef VECSLIDER

nfr("coloredit", "label,color", "SF}", "A2",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto c = PopVec<float4>(sp);
        ImGui::ColorEdit4(Label(vm, Pop(sp)), (float *)c.data());
        PushVec(sp, c);
    });

nfr("image", "tex,size,flip", "R:textureF}:2B?", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto flip = Pop(sp).True();
        auto sz = PopVec<float2>(sp);
        auto t = GetTexture(Pop(sp));
        ImGui::Image((ImTextureID)(size_t)t.id, ImVec2(sz.x, sz.y), ImVec2(0.0f, flip ? 0.0f : 1.0f),
                                                                    ImVec2(1.0f, flip ? 1.0f : 0.0f));
    });

nfr("image_button", "label,tex,size,bgcol,flip", "SR:textureF}:2F}:4?B?", "B",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto flip = Pop(sp).True();
        auto bgcol = PopVec<float4>(sp);
        auto sz = PopVec<float2>(sp);
        auto t = GetTexture(Pop(sp));
        auto label = Pop(sp);
        auto press =
            ImGui::ImageButton(Label(vm, label), (ImTextureID)(size_t)t.id, ImVec2(sz.x, sz.y),
                               ImVec2(0.0f, flip ? 0.0f : 1.0f),
                               ImVec2(1.0f, flip ? 1.0f : 0.0f),
                               ImVec4(bgcol.x, bgcol.y, bgcol.z, bgcol.w));
        Push(sp, press);
    });

nfr("image_mouseclick", "tex,size", "R:textureF}:2", "F}:2I",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto sz = PopVec<float2>(sp);
        auto t = GetTexture(Pop(sp));
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(sz.x, sz.y);
        ImGui::Image((ImTextureID)(size_t)t.id, size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        if (ImGui::IsMouseHoveringRect(cursor, cursor + size)) {
            auto pos = (ImGui::GetMousePos() - cursor) / size;
            PushVec<float, 2>(sp, float2(pos.x, pos.y));
            // Create an all-in-one event value similar to gl.button().
            int event = -1;
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) event = 1;
            else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) event = 0;
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) event = 2;
            Push(sp, event);
        } else {
            PushVec<float, 2>(sp, -float2_1);
            Push(sp, -1);
        }
    });

nfr("rect", "min,max,color,rounding,thickness", "F}:2F}:2F}:4FF", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto thickness = Pop(sp).fltval();
        auto rounding = Pop(sp).fltval();
        auto color = PopVec<float4>(sp);
        auto p_max = PopVec<float2>(sp);
        auto p_min = PopVec<float2>(sp);
        ImDrawFlags flags = rounding >= 0.0 ? ImDrawFlags_RoundCornersAll : ImDrawFlags_None;
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(p_min.x, p_min.y),
            ImVec2(p_max.x, p_max.y),
            ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, color.w)),
            rounding,
            flags,
            thickness
        );
    });

nfr("rect_filled", "min,max,color,rounding", "F}:2F}:2F}:4F", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto rounding = Pop(sp).fltval();
        auto color = PopVec<float4>(sp);
        auto p_max = PopVec<float2>(sp);
        auto p_min = PopVec<float2>(sp);
        ImDrawFlags flags = rounding >= 0.0 ? ImDrawFlags_RoundCornersAll : ImDrawFlags_None;
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(p_min.x, p_min.y),
            ImVec2(p_max.x, p_max.y),
            ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, color.w)),
            rounding,
            flags
        );
    });

nfr("set_next_item_open", "is_open,cond", "BI", "", "Set the open state of the next treenode",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto cond = Pop(sp).ival();
        auto is_open = Pop(sp).True();
        ImGui::SetNextItemOpen(is_open, (ImGuiCond)cond);
    });

nfr("treenode_start", "label,flags", "SI", "B",
    "(use im.treenode instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto flags = (ImGuiTreeNodeFlags)Pop(sp).intval();
        auto title = Pop(sp);
        bool open = ImGui::TreeNodeEx(Label(vm, title), flags);
        Push(sp, open);
        if (open) {
            NPush(N_TREE);
            if (!(flags & ImGuiTreeNodeFlags_IndentHalf)) {
                // By default, ImGui does not line up the contents of a treenode with its label,
                // this fixes that.
                ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing() / 2.0f);
            }
        }
    });

nfr("treenode_end", "flags", "I", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto flags = (ImGuiTreeNodeFlags)Pop(sp).intval();
        if (!(flags & ImGuiTreeNodeFlags_IndentHalf)) {
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() / 2.0f);
        }
        NPop(vm, N_TREE);
    });

nfr("tab_bar_start", "label", "S", "B",
    "(use im.tab_bar instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto title = Pop(sp);
        //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tabs_extra_padding);
        ImGui::GetStyle().FramePadding += tabs_extra_padding;
        bool open = ImGui::BeginTabBar(Label(vm, title), 0);
        ImGui::GetStyle().FramePadding -= tabs_extra_padding;
        //ImGui::PopStyleVar();
        Push(sp, open);
        if (open) NPush(N_TAB_BAR);
    });

nfr("tab_bar_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_TAB_BAR);
    });

nfr("tab_select", "label", "S", "",
    "call before any tabs to make a tab other than the first selected",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto title = Pop(sp).sval()->strv();
        auto tb = ImGui::GetCurrentTabBar();
        if (!tb) vm.BuiltinError("imgui: tab_bar_select not in contect of tab bar");
        ImGui::TabBarQueueFocus(tb, title.data());
    });

nfr("tab_start", "label,flags", "SI", "B",
    "(use im.tab instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto flags = Pop(sp).intval();
        auto title = Pop(sp);
        ImGui::GetStyle().FramePadding += tabs_extra_padding;
        bool open = ImGui::BeginTabItem(Label(vm, title), nullptr, (ImGuiTabItemFlags)flags);
        ImGui::GetStyle().FramePadding -= tabs_extra_padding;
        Push(sp, open);
        if (open) NPush(N_TAB);
    });

nfr("tab_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_TAB);
    });

nfr("menu_bar_start", "main", "B", "B",
    "(use im.menu_bar instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto main = Pop(sp).True();
        bool open = main ? ImGui::BeginMainMenuBar() : ImGui::BeginMenuBar();
        Push(sp, open);
        if (open) NPush(main ? N_MAIN_MENU_BAR: N_MENU_BAR);
    });

nfr("menu_bar_end", "main", "B", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto main = Pop(sp).True();
        NPop(vm, main ? N_MAIN_MENU_BAR: N_MENU_BAR);
    });

nfr("menu_start", "label,disabled", "SB?", "B",
    "(use im.menu instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, {});
        RequireMenuNesting(vm);
        auto disabled = Pop(sp).True();
        auto title = Pop(sp);
        bool open = ImGui::BeginMenu(Label(vm, title), !disabled);
        Push(sp, open);
        if (open) NPush(N_MENU);
    });

nfr("menu_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm, {});
        RequireMenuNesting(vm);
        NPop(vm, N_MENU);
    });

nfr("menu_item", "label,shortcut,disabled", "SS?B?", "B",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_MENU, N_POPUP });
        auto disabled = Pop(sp).True();
        auto shortcut = Pop(sp);
        auto title = Pop(sp);
        auto press = ImGui::MenuItem(Label(vm, title),
                                     shortcut.True() ? shortcut.sval()->data() : nullptr,
                                     false,
                                     !disabled);
        Push(sp, press);
    });

nfr("menu_item_toggle", "label,selected,disabled", "SB?B?", "B",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_MENU, N_POPUP });
        auto disabled = Pop(sp).True();
        auto selected = Pop(sp).True();
        auto title = Pop(sp);
        ImGui::MenuItem(Label(vm, title), nullptr, &selected, !disabled);
        Push(sp, selected);
    });

nfr("id_start", "label", "Ss", "",
    "an invisble group around some widgets, useful to ensure these widgets are unique"
    " (if they have the same label as widgets in another group that has a different group"
    " label). Use im.id instead",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto title = Pop(sp);
        ImGui::PushID(Label(vm, title));
        NPush(N_ID);
    });

nfr("id_start", "label", "I", "",
    "(integer version)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto i = Pop(sp);
        ImGui::PushID(i.intval());
        NPush(N_ID);
    });

nfr("id_start", "label", "A", "",
    "(reference version)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto r = Pop(sp);
        ImGui::PushID((const void *)r.ref());
        NPush(N_ID);
    });

nfr("id_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_ID);
    });

nfr("child_start", "title,size,flags", "SF}:2I", "B",
    "create a self-contained scrolling/clipping region with a window. use im.child instead",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto flags = Pop(sp);
        auto sz = PopVec<float2>(sp);
        auto title = Pop(sp);
        bool open = ImGui::BeginChild(Label(vm, title), ImVec2(sz.x, sz.y), false, (ImGuiWindowFlags)flags.ival());
        Push(sp, open);
        // Unlike other begin/end pairs, we must always call end unconditionally, so we always push this.
        NPush(N_CHILD);
    });

nfr("child_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_CHILD);
    });

nfr("group_start", "", "", "",
    "lock the horizontal starting position, and capture all contained widgets"
    " into one item. Use im.group instead",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::BeginGroup();
        NPush(N_VGROUP);
    });

nfr("group_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_VGROUP);
    });

nfr("popup_start", "label,winflags,rmbprevitem", "SIB?", "B",
    "(use im.popup instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto rmb = Pop(sp).True();
        auto flags = (ImGuiWindowFlags)Pop(sp).intval();
        auto title = Pop(sp);
        bool open = rmb ? ImGui::BeginPopupContextItem(Label(vm, title))
            : ImGui::BeginPopup(Label(vm, title), flags);
        Push(sp, open);
        if (open) NPush(N_POPUP);
    });

nfr("popup_modal_start", "label,winflags", "SI", "B",
    "(use im.popup_modal instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto flags = (ImGuiWindowFlags)Pop(sp).intval();
        auto title = Pop(sp);
        bool open = ImGui::BeginPopupModal(Label(vm, title), nullptr, flags);
        Push(sp, open);
        if (open) NPush(N_POPUP);
    });

nfr("popup_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_POPUP);
    });

nfr("popup_open", "label", "S", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        auto title = Pop(sp);
        ImGui::OpenPopup(Label(vm, title));
    });

nfr("close_current_popup", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::CloseCurrentPopup();
    });

nfr("disabled_start", "disabled", "B", "",
    "(use im.disabled instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        const auto disabled = Pop(sp).True();
        ImGui::BeginDisabled(disabled);
        NPush(N_DISABLED);
    });

nfr("disabled_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_DISABLED);
    });

nfr("button_repeat_start", "repeat", "B", "",
    "(use im.button_repeat instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        const auto repeat = Pop(sp).True();
        ImGui::PushButtonRepeat(repeat);
    });

nfr("button_repeat_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        ImGui::PopButtonRepeat();
    });

nfr("dummy", "size", "F}:2", "add a dummy item of a given size",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto size = PopVec<float2>(sp);
        ImGui::Dummy(ImVec2(size.x, size.y));
    });

nfr("drag_drop_source_start", "flags", "I", "B",
    "(use im.drag_drop_source instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        const auto flags = Pop(sp).intval();
        bool open = ImGui::BeginDragDropSource(flags);
        Push(sp, open);
        if (open) NPush(N_DRAG_DROP_SOURCE);
    });

nfr("drag_drop_source_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_DRAG_DROP_SOURCE);
    });

nfr("set_drag_drop_payload", "type,data", "SS", "",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto data = Pop(sp).sval()->strv();
        auto type = Pop(sp).sval()->strvnt();
        ImGui::SetDragDropPayload(type.c_str(), data.data(), data.size());
    });

nfr("drag_drop_target_start", "", "", "B",
    "(use im.drag_drop_target instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        bool open = ImGui::BeginDragDropTarget();
        Push(sp, open);
        if (open) NPush(N_DRAG_DROP_TARGET);
    });

nfr("drag_drop_main_viewport_target_start", "", "", "B",
    "(use im.drag_drop_main_viewport_target instead)",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        // See: https://github.com/ocornut/imgui/issues/5204
        auto viewport = ImGui::GetMainViewport();
        bool open = !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
                    ImGui::BeginDragDropTargetViewport(viewport);
        Push(sp, open);
        if (open) NPush(N_DRAG_DROP_TARGET);
    });

nfr("drag_drop_target_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_DRAG_DROP_TARGET);
    });

nfr("accept_drag_drop_payload", "type,flags", "SI", "S?",
    "",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        const auto flags = Pop(sp).intval();
        auto type = Pop(sp).sval()->strvnt();
        auto *payload = ImGui::AcceptDragDropPayload(type.c_str(), flags);
        if (payload) {
            Push(sp, vm.NewString(string_view { (const char*)payload->Data, (size_t)payload->DataSize }));
        } else {
            Push(sp, NilVal());
        }
    });

nfr("width_start", "width", "F", "",
    "Sets the width of an item: 0 = default, -1 = use full width without label,"
    " any other value is custom width. Use im.width instead",
    [](StackPtr &sp, VM &vm) {
        IsInit(vm);
        auto width = Pop(sp).fltval();
        ImGui::PushItemWidth(width);
        NPush(N_WIDTH);
    });

nfr("width_end", "", "", "", "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_WIDTH);
    });

nfr("text_table", "id,num_colums,labels", "SIS]", "", "",
    [](StackPtr &, VM &vm, Value id, Value num_colums, Value labels) {
        IsInit(vm);
        auto nc = num_colums.intval();
        if (!ImGui::BeginTable(id.sval()->strvnt().c_str(), nc,
                               ImGuiTableFlags_SizingFixedFit |
                               ImGuiTableFlags_NoHostExtendX |
                               ImGuiTableFlags_Borders))
            return NilVal();
        for (iint i = 0; i < labels.vval()->len; i++) {
            if (!(i % nc)) ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(i % nc);
            Text(labels.vval()->AtS(i).sval()->strv());
        }
        ImGui::EndTable();
        return NilVal();
    });

nfr("table_start", "id,num_colums,flags", "SII", "B",
    "(use im.table instead)",
    [](StackPtr &, VM &vm, Value id, Value num_colums, Value flags) {
        IsInit(vm);
        auto nc = num_colums.intval();
        auto visible = ImGui::BeginTable(id.sval()->strvnt().c_str(), nc, flags.intval());
        if (visible) NPush(N_TABLE);
        return Value(visible);
    });

nfr("table_setup_column", "label,flags,init_width_or_weight", "SIF", "",
    "(use im.table instead)",
    [](StackPtr &, VM &vm, Value label, Value flags, Value init) {
        IsInit(vm);
        if (NTop() == N_TABLE)
            ImGui::TableSetupColumn(label.sval()->strvnt().c_str(), flags.intval(), init.fltval());
        return NilVal();
    });

nfr("table_headers_row", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        if (NTop() == N_TABLE) ImGui::TableHeadersRow();
    });

nfr("table_next_row", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        if (ImGui::TableGetColumnCount()) ImGui::TableNextRow();
    });

nfr("table_next_column", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        if (ImGui::TableGetColumnCount()) ImGui::TableNextColumn();
    });

nfr("table_end", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        NPop(vm, N_TABLE);
    });

nfr("edit_anything", "value,label", "AkS?", "A1",
    "creates a UI for any lobster reference value, and returns the edited version",
    [](StackPtr &, VM &vm, Value v, Value label) {
        IsInit(vm);
        // FIXME: would be good to support structs, but that requires typeinfo, not just len.
        auto &ti = vm.GetTypeInfo(v.True() ? v.ref()->tti : TYPE_ELEM_ANY);
        ValToGUI(vm, &v, &ti, label.True() ? label.sval()->strvnt() : string_view_nt(""), true,
                 false);
        return v;
    });

nfr("graph", "label,values,ishistogram", "SF]I", "",
    "",
    [](StackPtr &, VM &vm, Value label, Value vals, Value histogram) {
        IsInit(vm);
        auto getter = [](void *data, int i) -> float {
            return ((Value *)data)[i].fltval();
        };
        if (histogram.True()) {
            ImGui::PlotHistogram(Label(vm, label), getter, vals.vval()->Elems(),
                (int)vals.vval()->len);
        } else {
            ImGui::PlotLines(Label(vm, label), getter, vals.vval()->Elems(),
                (int)vals.vval()->len);
        }
        return NilVal();
    });

nfr("show_flexbuffer", "value", "S", "",
    "",
    [](StackPtr &, VM &vm, Value v) {
        IsInit(vm);
        auto sv = v.sval()->strv();
        auto root = flexbuffers::GetRoot((const uint8_t *)sv.data(), sv.size());
        FlexBufferGUI(root, "Stack trace", false);
        return NilVal();
    });

nfr("show_vars", "", "", "",
    "shows an automatic editing UI for each global variable in your program",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        VarsToGUI(vm);
        return NilVal();
    });

nfr("show_engine_stats", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm);
        EngineStatsGUI();
        return NilVal();
    });

nfr("show_debug_metrics_window", "", "", "",
    "",
    [](StackPtr &, VM &vm) {
        IsInit(vm, { N_FRAME, N_NONE });
        // There is also ShowDebugLogWindow() and others, but all of these can be
        // opened from this window, so really only one needed:
        ImGui::ShowMetricsWindow();
        return NilVal();
    });

nfr("show_profiling_stats", "num,reset,histogram,paused", "IBBB", "",
    "",
    [](StackPtr &, VM &vm, Value num, Value reset, Value histogram, Value paused) {
        IsInit(vm);
        #if LOBSTER_FRAME_PROFILER == 1
        prof_db.paused = paused.True();
        if (reset.True()) prof_db.stats.clear();
        vector<pair<const struct ___tracy_source_location_data *, ProfStat *>> display;
        for (auto &it : prof_db.stats) {
            display.push_back({ it.first, &it.second });
        }
        sort(display.begin(), display.end(),
             [](pair<const struct ___tracy_source_location_data *, ProfStat *> &a,
                pair<const struct ___tracy_source_location_data *, ProfStat *> &b) -> bool {
                 return a.second->highest >= b.second->highest;
            });
        if (!ImGui::BeginTable("show_profiling_stats", histogram.True() ? 4 : 3,
                               ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX |
                                   ImGuiTableFlags_Borders))
            return NilVal();
        ImGui::TableSetupColumn("Function");
        ImGui::TableSetupColumn("AVG ms");
        ImGui::TableSetupColumn("HI ms");
        if (histogram.True()) {
            ImGui::TableSetupColumn("Histogram");
        }
        ImGui::TableHeadersRow();
        auto i = num.ival();
        for (auto &it : display) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            Text(it.first->function);
            ImGui::TableSetColumnIndex(1);
            Text(to_string_float(it.second->time * 1000.0 / it.second->n, 3));
            ImGui::TableSetColumnIndex(2);
            Text(to_string_float(it.second->highest * 1000.0, 3));
            if (histogram.True()) {
                ImGui::TableSetColumnIndex(3);
                ImGui::PlotHistogram(cat("##", it.first->function).c_str(),
                                     prof_histogram_values_getter, it.second->window,
                                     PROF_WINDOW_SIZE, (int)prof_db.window_pos + 1,
                                     nullptr, FLT_MAX, FLT_MAX,
                                     ImVec2 { PROF_WINDOW_SIZE * 4.0f, 0.0f });
            }
            if (!--i) break;
        }
        ImGui::EndTable();
        #else
        (void)num;
        (void)reset;
        #endif
        return NilVal();
    });

#define IMGUI_STYLE_PARAMS(FV, VV, BV, DV) \
    FV(Alpha                      ) /* Global alpha applies to everything in Dear ImGui. */ \
    FV(DisabledAlpha              ) /* Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha. */ \
    VV(WindowPadding              ) /* Padding within a window. */ \
    FV(WindowRounding             ) /* Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended. */ \
    FV(WindowBorderSize           ) /* Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly). */ \
    VV(WindowMinSize              ) /* Minimum window size. This is a global setting. If you want to constrain individual windows, use SetNextWindowSizeConstraints(). */ \
    VV(WindowTitleAlign           ) /* Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered. */ \
    DV(WindowMenuButtonPosition   ) /* Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to ImGuiDir_Left. */ \
    FV(ChildRounding              ) /* Radius of child window corners rounding. Set to 0.0f to have rectangular windows. */ \
    FV(ChildBorderSize            ) /* Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly). */ \
    FV(PopupRounding              ) /* Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding) */ \
    FV(PopupBorderSize            ) /* Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly). */ \
    VV(FramePadding               ) /* Padding within a framed rectangle (used by most widgets). */ \
    FV(FrameRounding              ) /* Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets). */ \
    FV(FrameBorderSize            ) /* Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly). */ \
    VV(ItemSpacing                ) /* Horizontal and vertical spacing between widgets/lines. */ \
    VV(ItemInnerSpacing           ) /* Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label). */ \
    VV(CellPadding                ) /* Padding within a table cell. Cellpadding.x is locked for entire table. CellPadding.y may be altered between different rows. */ \
    VV(TouchExtraPadding          ) /* Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much! */ \
    FV(IndentSpacing              ) /* Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2). */ \
    FV(ColumnsMinSpacing          ) /* Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1). */ \
    FV(ScrollbarSize              ) /* Width of the vertical scrollbar, Height of the horizontal scrollbar. */ \
    FV(ScrollbarRounding          ) /* Radius of grab corners for scrollbar. */ \
    FV(GrabMinSize                ) /* Minimum width/height of a grab box for slider/scrollbar. */ \
    FV(GrabRounding               ) /* Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs. */ \
    FV(LogSliderDeadzone          ) /* The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero. */ \
    FV(TabRounding                ) /* Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs. */ \
    FV(TabBorderSize              ) /* Thickness of border around tabs. */ \
    FV(TabBarBorderSize           ) /* Thickness of tab-bar separator, which takes on the tab active color to denote focus. */ \
    FV(TabBarOverlineSize         ) /* Thickness of tab-bar overline, which highlights the selected tab-bar. */ \
    FV(TableAngledHeadersAngle    ) /* Angle of angled headers (supported values range from -50.0f degrees to +50.0f degrees). */ \
    VV(TableAngledHeadersTextAlign) /* Alignment of angled headers within the cell */ \
    DV(ColorButtonPosition        ) /* Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right. */ \
    VV(ButtonTextAlign            ) /* Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered). */ \
    VV(SelectableTextAlign        ) /* Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line. */ \
    FV(SeparatorTextBorderSize    ) /* Thickness of border in SeparatorText() */ \
    VV(SeparatorTextAlign         ) /* Alignment of text within the separator. Defaults to (0.0f, 0.5f) (left aligned, center). */ \
    VV(SeparatorTextPadding       ) /* Horizontal offset of text from each edge of the separator + spacing on other axis. Generally small values. .y is recommended to be == FramePadding.y. */ \
    VV(DisplayWindowPadding       ) /* Apply to regular windows: amount which we enforce to keep visible when moving near edges of your screen. */ \
    VV(DisplaySafeAreaPadding     ) /* Apply to every windows, menus, popups, tooltips: amount where we avoid displaying contents. Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured). */ \
    FV(DockingSeparatorSize       ) /* Thickness of resizing border between docked windows */ \
    FV(MouseCursorScale           ) /* Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). We apply per-monitor DPI scaling over this scale. May be removed later. */ \
    BV(AntiAliasedLines           ) /* Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList). */ \
    BV(AntiAliasedLinesUseTex     ) /* Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to ImDrawList). */ \
    BV(AntiAliasedFill            ) /* Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList). */ \
    FV(CurveTessellationTol       ) /* Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality. */ \
    FV(CircleTessellationMaxError ) /* Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry. */

nfr("set_style_param_slow", "name,vals", "SF]", "",
    "",
    [](StackPtr &, VM &vm, Value name, Value vals) {
        IsInit(vm, { N_NONE, N_NONE });
        auto n = name.sval()->strv();
        auto &st = ImGui::GetStyle();
        auto vs = vals.vval();
        // This is a reaaaally dumb function, but quickest way I could just expose all this functionality.
        auto fv = 0.0f;
        auto vv = ImVec2(0.0f, 0.0f);
        auto bv = false;
        auto dv = ImGuiDir_None;
        if (vs->len > 0) {
            fv = vs->AtS(0).fltval();
            vv.x = fv;
            bv = fv != 0.0f;
            dv = (ImGuiDir) int(fv);
            if (vs->len > 1) {
                vv.y = vs->AtS(1).fltval();
            }
        }
        #define PARAM_FV(Name) if (n == #Name) st.Name = fv;
        #define PARAM_VV(Name) if (n == #Name) st.Name = vv;
        #define PARAM_BV(Name) if (n == #Name) st.Name = bv;
        #define PARAM_DV(Name) if (n == #Name) st.Name = dv;
        IMGUI_STYLE_PARAMS(PARAM_FV, PARAM_VV, PARAM_BV, PARAM_DV)
        #undef PARAM_FV
        #undef PARAM_VV
        #undef PARAM_BV
        #undef PARAM_DV
        return NilVal();
    });

nfr("get_style_param_slow", "name,vals", "SF]", "",
    "",
    [](StackPtr &, VM &vm, Value name, Value vals) {
        IsInit(vm, { N_NONE, N_NONE });
        auto n = name.sval()->strv();
        auto &st = ImGui::GetStyle();
        auto vs = vals.vval();
        #define PARAM_FV(Name) if (n == #Name) { vs->Push(vm, st.Name); }
        #define PARAM_VV(Name) if (n == #Name) { vs->Push(vm, st.Name.x); vs->Push(vm, st.Name.y); }
        #define PARAM_BV(Name) if (n == #Name) { vs->Push(vm, (float)st.Name); }
        #define PARAM_DV(Name) if (n == #Name) { vs->Push(vm, (float)st.Name); }
        IMGUI_STYLE_PARAMS(PARAM_FV, PARAM_VV, PARAM_BV, PARAM_DV)
        #undef PARAM_FV
        #undef PARAM_VV
        #undef PARAM_BV
        #undef PARAM_DV
        return NilVal();
    });


}  // AddIMGUI

void AddIMGUIDebug(NativeRegistry & nfr) {

nfr("breakpoint", "condition", "I", "",
    "stops the program in the debugger if passed true."
    " debugger needs --runtime-stack-trace on, and im.init() to have run.",
    [](StackPtr &, VM &vm, Value c) {
        if (c.True()) {
            auto err = BreakPoint(vm, "Conditional breakpoint hit!");
            if (!err.empty()) vm.Error(err);
        }
        return NilVal();
    });

nfr("breakpoint", "", "", "",
    "stops the program in the debugger always."
    " debugger needs --runtime-stack-trace on, and im.init() to have run.",
    [](StackPtr &, VM &vm) {
        auto err = BreakPoint(vm, "Breakpoint hit!");
        if (!err.empty()) vm.Error(err);
        return NilVal();
    });

}  // AddIMGUIDebug
