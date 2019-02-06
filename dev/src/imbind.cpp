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
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "lobster/stdafx.h"

#include "lobster/natreg.h"
#include "lobster/vmdata.h"

#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "lobster/bytecode_generated.h"

#include "lobster/sdlincludes.h"
#include "lobster/sdlinterface.h"

using namespace lobster;

extern SDL_Window *_sdl_window;
extern SDL_GLContext _sdl_context;

bool imgui_init = false;

void IMGUICleanup() {
    if (!imgui_init) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    imgui_init = false;
}

void IsInit(VM &vm) {
    if (!imgui_init) vm.BuiltinError("IMGUI not running: call im_init first");
}

pair<bool, bool> IMGUIEvent(SDL_Event *event) {
    if (!imgui_init) return { false, false };
    ImGui_ImplSDL2_ProcessEvent(event);
    return { ImGui::GetIO().WantCaptureMouse, ImGui::GetIO().WantCaptureKeyboard };
}

LString *LStringInputText(VM &vm, const char *label, LString *str, ImGuiInputTextFlags flags = 0) {
    struct InputTextCallbackData {
        LString *str;
        VM &vm;
        static int InputTextCallback(ImGuiInputTextCallbackData *data) {
            if (data->EventFlag != ImGuiInputTextFlags_CallbackResize) return 0;
            auto cbd = (InputTextCallbackData *)data->UserData;
            IM_ASSERT(data->Buf == cbd->str->data());
            auto str = cbd->vm.NewString(string_view { data->Buf, (size_t)data->BufTextLen });
            cbd->str->Dec(cbd->vm);
            cbd->str = str;
            data->Buf = (char *)str->data();
            return 0;
        }
    };
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallbackData cbd { str, vm };
    ImGui::InputText(label, (char *)str->data(), str->len + 1, flags,
                     InputTextCallbackData::InputTextCallback, &cbd);
    return cbd.str;
}

void ValToGUI(VM &vm, Value &v, const TypeInfo &ti, string_view label, bool expanded) {
    auto l = null_terminated(label);
    auto flags = expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    switch (ti.t) {
        case V_INT: {
            int i = v.intval();  // FIXME: what if int64_t?
            if (ImGui::InputInt(l, &i)) v = i;
            return;
        }
        case V_FLOAT: {
            float f = v.fltval();  // FIXME: what if double?
            if (ImGui::InputFloat(l, &f)) v = f;
            return;
        }
        case V_VECTOR:
            if (!v.True()) break;
            if (ImGui::TreeNodeEx(*l ? l : "[]", flags)) {
                auto &sti = vm.GetTypeInfo(ti.subt);
                for (intp i = 0; i < v.vval()->len; i++) {
                    ValToGUI(vm, v.vval()->At(i), sti, to_string(i), false);
                }
                ImGui::TreePop();
            }
            return;
        case V_STRUCT: {
            if (!v.True()) break;
            auto st = vm.bcf->structs()->Get(ti.structidx);
            // Special case for numeric structs & colors.
            if (ti.len >= 2 && ti.len <= 4) {
                for (int i = 1; i < ti.len; i++)
                    if (ti.elems[i] != ti.elems[0]) goto generic;
                if (ti.elems[0] == TYPE_ELEM_INT) {
                    auto nums = ValueToI<4>(vm, v);
                    if (ImGui::InputScalarN(
                            l, sizeof(intp) == sizeof(int) ? ImGuiDataType_S32 : ImGuiDataType_S64,
                            (void *)nums.data(), ti.len, NULL, NULL, "%d", flags)) {
                        v.LTDECRT(vm);
                        v = ToValueI(vm, nums, ti.len);
                    }

                    return;
                } else if (ti.elems[0] == TYPE_ELEM_FLOAT) {
                    if (strcmp(st->name()->c_str(), "color") == 0) {
                        auto c = ValueToFLT<4>(vm, v);
                        if (ImGui::ColorEdit4(l, (float *)c.data())) {
                            v.LTDECRT(vm);
                            v = ToValueFLT(vm, c, ti.len);
                        }
                    } else {
                        auto nums = ValueToF<4>(vm, v);
                        // FIXME: format configurable.
                        if (ImGui::InputScalarN(
                                l,
                                sizeof(floatp) == sizeof(float) ? ImGuiDataType_Float
                                                                : ImGuiDataType_Double,
                                (void *)nums.data(), ti.len, NULL, NULL, "%.3f", flags)) {
                            v.LTDECRT(vm);
                            v = ToValueF(vm, nums, ti.len);
                        }
                    }
                    return;
                }
            }
        generic:
            if (ImGui::TreeNodeEx(*l ? l : st->name()->c_str(), flags)) {
                auto fields = st->fields();
                for (int i = 0; i < ti.len; i++) {
                    ValToGUI(vm, v.stval()->AtS(i), vm.GetTypeInfo(ti.elems[i]),
                             fields->Get(i)->name()->string_view(), false);
                }
                ImGui::TreePop();
            }
            return;
        }
        case V_STRING: {
            if (!v.True()) break;
            v = LStringInputText(vm, l, v.sval());
            return;
        }
    }
    ostringstream ss;
    v.ToString(vm, ss, ti.t, vm.debugpp);
    ImGui::LabelText(l, "%s", ss.str().c_str());  // FIXME: no formatting?
}

void VarsToGUI(VM &vm) {
    auto DumpVars = [&](bool constants) {
        for (uint i = 0; i < vm.bcf->specidents()->size(); i++) {
            auto val = vm.vars[i];
            auto sid = vm.bcf->specidents()->Get(i);
            auto id = vm.bcf->idents()->Get(sid->ididx());
            if (!id->global() || id->readonly() != constants) continue;
            auto name = id->name()->string_view();
            auto &ti = vm.GetVarTypeInfo(i);
            #if RTT_ENABLED
            if (ti.t != val.type) continue;  // Likely uninitialized.
            #endif
            ValToGUI(vm, val, ti, name, true);
        }
    };
    DumpVars(false);
    if (ImGui::TreeNodeEx("Constants", 0)) {
        DumpVars(true);
        ImGui::TreePop();
    }
}

void EngineStatsGUI() {
    auto &ft = SDLGetFrameTimeLog();
    ImGui::PlotLines("gl_deltatime", ft.data(), (int)ft.size());
}

void AddIMGUI(NativeRegistry &natreg) {
    STARTDECL(im_init) (VM &, Value &darkstyle) {
        if (imgui_init) return Value();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        if (darkstyle.True()) ImGui::StyleColorsDark(); else ImGui::StyleColorsClassic();
        ImGui_ImplSDL2_InitForOpenGL(_sdl_window, _sdl_context);
        ImGui_ImplOpenGL3_Init("#version 150");
        imgui_init = true;
        return Value();
    }
    ENDDECL1(im_init, "dark_style", "I", "", "");

    STARTDECL(im_add_font) (VM &vm, Value &fontname, Value &size) {
        IsInit(vm);
        string buf;
        auto l = LoadFile(fontname.sval()->strv(), &buf);
        if (l < 0) return Value();
        auto mb = malloc(buf.size());  // FIXME.
        memcpy(mb, buf.data(), buf.size());
        ImFontConfig imfc;
        imfc.FontDataOwnedByAtlas = true;
        auto font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(mb, (int)buf.size(),
                                                               size.fltval(), &imfc);
        return font != nullptr;
    }
    ENDDECL2(im_add_font, "font_path,size", "SF", "I", "");

    STARTDECL(im_frame) (VM &vm, Value &body) {
        IsInit(vm);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(_sdl_window);
        ImGui::NewFrame();
        return body;
    }
    MIDDECL(im_frame) (VM &) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    ENDDECL1CONTEXIT(im_frame, "body", "B", "", "");

    STARTDECL(im_window_demo) (VM &vm) {
        IsInit(vm);
        bool show = true;
        ImGui::ShowDemoWindow(&show);
        return show;
    }
    ENDDECL0(im_window_demo, "", "", "I", "");

    STARTDECL(im_window) (VM &vm, Value &title, Value &body) {
        IsInit(vm);
        ImGui::Begin(title.sval()->data());
        return body;
    }
    MIDDECL(im_window) (VM &) {
        ImGui::End();
    }
    ENDDECL2CONTEXIT(im_window, "title,body", "SB", "", "");

    STARTDECL(im_button) (VM &, Value &title, Value &body) {
        return ImGui::Button(title.sval()->data()) ? body : Value();
    }
    MIDDECL(im_button) (VM &) {
    }
    ENDDECL2CONTEXIT(im_button, "label,body", "SB", "", "");

    STARTDECL(im_same_line) (VM &) {
        ImGui::SameLine();
        return Value();
    }
    ENDDECL0(im_same_line, "", "", "", "");

    STARTDECL(im_separator) (VM &) {
        ImGui::Separator();
        return Value();
    }
    ENDDECL0(im_separator, "", "", "", "");

    STARTDECL(im_text) (VM &, Value &text) {
        ImGui::Text("%s", text.sval()->data());
        return Value();
    }
    ENDDECL1(im_text, "label", "S", "", "");

    STARTDECL(im_tooltip) (VM &, Value &text) {
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", text.sval()->data());
        return Value();
    }
    ENDDECL1(im_tooltip, "label", "S", "", "");

    STARTDECL(im_checkbox) (VM &, Value &text, Value &boolean) {
        bool b = boolean.True();
        ImGui::Checkbox(text.sval()->data(), &b);
        return Value(b);
    }
    ENDDECL2(im_checkbox, "label,bool", "SI", "I", "");

    STARTDECL(im_input_text) (VM &vm, Value &text, Value &str) {
        return LStringInputText(vm, text.sval()->data(), str.sval());
    }
    ENDDECL2(im_input_text, "label,str", "SSk", "S", "");

    STARTDECL(im_radio) (VM &, Value &strs, Value &active, Value &horiz) {
        int sel = active.intval();
        for (intp i = 0; i < strs.vval()->len; i++) {
            if (i && horiz.True()) ImGui::SameLine();
            ImGui::RadioButton(strs.vval()->At(i).sval()->data(), &sel, (int)i);
        }
        return Value(sel);
    }
    ENDDECL3(im_radio, "labels,active,horiz", "S]II", "I", "");

    STARTDECL(im_combo) (VM &, Value &text, Value &strs, Value &active) {
        int sel = active.intval();
        vector<const char *> items(strs.vval()->len);
        for (intp i = 0; i < strs.vval()->len; i++) {
            items[i] = strs.vval()->At(i).sval()->data();
        }
        ImGui::Combo(text.sval()->data(), &sel, items.data(), (int)items.size());
        return Value(sel);
    }
    ENDDECL3(im_combo, "label,labels,active", "SS]I", "I", "");

    STARTDECL(im_listbox) (VM &, Value &text, Value &strs, Value &active) {
        int sel = active.intval();
        vector<const char *> items(strs.vval()->len);
        for (intp i = 0; i < strs.vval()->len; i++) {
            items[i] = strs.vval()->At(i).sval()->data();
        }
        ImGui::ListBox(text.sval()->data(), &sel, items.data(), (int)items.size());
        return Value(sel);
    }
    ENDDECL3(im_listbox, "label,labels,active", "SS]I", "I", "");

    STARTDECL(im_sliderint) (VM &, Value &text, Value &integer, Value &min, Value &max) {
        int i = integer.intval();
        ImGui::SliderInt(text.sval()->data(), &i, min.intval(), max.intval());
        return Value(i);
    }
    ENDDECL4(im_sliderint, "label,i,min,max", "SIII", "I", "");

    STARTDECL(im_sliderfloat) (VM &, Value &text, Value &flt, Value &min, Value &max) {
        float f = flt.fltval();
        ImGui::SliderFloat(text.sval()->data(), &f, min.fltval(), max.fltval());
        return Value(f);
    }
    ENDDECL4(im_sliderfloat, "label,f,min,max", "SFFF", "F", "");

    STARTDECL(im_coloredit) (VM &vm, Value &text, Value &col) {
        auto c = ValueToFLT<4>(vm, col);
        ImGui::ColorEdit4(text.sval()->data(), (float *)c.data());
        return Value(ToValueFLT(vm, c));
    }
    ENDDECL2(im_coloredit, "label,color", "SF}", "A2", "");

    STARTDECL(im_edit_anything) (VM &vm, Value &v, Value &label) {
        ValToGUI(vm, v, vm.GetTypeInfo(v.True() ? v.ref()->tti : TYPE_ELEM_ANY),
                 label.True() ? label.sval()->strv() : "", true);
        return v;
    }
    ENDDECL2(im_edit_anything, "value,label", "AkS?", "A1", "");

    STARTDECL(im_show_vars) (VM &vm) {
        VarsToGUI(vm);
        return Value();
    }
    ENDDECL0(im_show_vars, "", "", "", "");

    STARTDECL(im_show_engine_stats) (VM &) {
        EngineStatsGUI();
        return Value();
    }
    ENDDECL0(im_show_engine_stats, "", "", "", "");
}
