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
int imgui_windows = 0;

void IMGUICleanup() {
    if (!imgui_init) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    imgui_init = false;
    imgui_windows = 0;
}

void IsInit(StackPtr &sp, VM &vm, bool require_window = true) {
    if (!imgui_init) vm.BuiltinError(sp, "IMGUI not running: call im_init first");
    if (!imgui_windows && require_window) vm.BuiltinError(sp, "no window: call im_window first");
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

void ValToGUI(VM &vm, Value *v, const TypeInfo &ti, string_view label, bool expanded) {
    auto l = null_terminated(label);
    auto flags = expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    switch (ti.t) {
        case V_INT: {
            if (ti.enumidx == 0) {
                assert(vm.EnumName(ti.enumidx) == "bool");
                bool b = v->True();
                if (ImGui::Checkbox(l, &b)) *v = b;
            } else if (ti.enumidx >= 0) {
                int val = v->intval();
                int sel = 0;
                auto &vals = *vm.bcf->enums()->Get(ti.enumidx)->vals();
                vector<const char *> items(vals.size());
                int i = 0;
                for (auto vi : vals) {
                    items[i] = vi->name()->c_str();
                    if (val == vi->val()) sel = i;
                    i++;
                }
                ImGui::Combo(l, &sel, items.data(), (int)items.size());
                *v = vals[sel]->val();
            } else {
                int i = v->intval();  // FIXME: what if int64_t?
                if (ImGui::InputInt(l, &i)) *v = i;
            }
            return;
        }
        case V_FLOAT: {
            float f = v->fltval();  // FIXME: what if double?
            if (ImGui::InputFloat(l, &f)) *v = f;
            return;
        }
        case V_VECTOR:
            if (v->False()) break;
            if (ImGui::TreeNodeEx(*l ? l : "[]", flags)) {
                auto &sti = vm.GetTypeInfo(ti.subt);
                auto vec = v->vval();
                for (iint i = 0; i < vec->len; i++) {
                    ValToGUI(vm, vec->AtSt(i), sti, to_string(i), false);
                }
                ImGui::TreePop();
            }
            return;
        case V_CLASS:
            if (v->False()) break;
            v = v->oval()->Elems();  // To iterate it like a struct.
        case V_STRUCT_R:
        case V_STRUCT_S: {
            auto st = vm.bcf->udts()->Get(ti.structidx);
            // Special case for numeric structs & colors.
            if (ti.len >= 2 && ti.len <= 4) {
                for (int i = 1; i < ti.len; i++)
                    if (ti.elemtypes[i] != ti.elemtypes[0]) goto generic;
                if (ti.elemtypes[0] == TYPE_ELEM_INT) {
                    auto nums = ValueToI<4>(v, ti.len);
                    if (ImGui::InputScalarN(
                            l, ImGuiDataType_S64,
                            (void *)nums.data(), ti.len, NULL, NULL, "%d", flags)) {
                        ToValue(v, ti.len, nums);
                    }

                    return;
                } else if (ti.elemtypes[0] == TYPE_ELEM_FLOAT) {
                    if (strcmp(st->name()->c_str(), "color") == 0) {
                        auto c = ValueToFLT<4>(v, ti.len);
                        if (ImGui::ColorEdit4(l, (float *)c.data())) {
                            ToValue(v, ti.len, c);
                        }
                    } else {
                        auto nums = ValueToF<4>(v, ti.len);
                        // FIXME: format configurable.
                        if (ImGui::InputScalarN(
                                l,
                                sizeof(double) == sizeof(float) ? ImGuiDataType_Float
                                                                : ImGuiDataType_Double,
                                (void *)nums.data(), ti.len, NULL, NULL, "%.3f", flags)) {
                            ToValue(v, ti.len, nums);
                        }
                    }
                    return;
                }
            }
        generic:
            if (ImGui::TreeNodeEx(*l ? l : st->name()->c_str(), flags)) {
                auto fields = st->fields();
                int fi = 0;
                for (int i = 0; i < ti.len; i++) {
                    auto &sti = vm.GetTypeInfo(ti.GetElemOrParent(i));
                    ValToGUI(vm, v + i, sti,
                             fields->Get(fi++)->name()->string_view(), false);
                    if (IsStruct(sti.t)) i += sti.len - 1;
                }
                ImGui::TreePop();
            }
            return;
        }
        case V_STRING: {
            if (v->False()) break;
            *v = LStringInputText(vm, l, v->sval());
            return;
        }
        case V_NIL:
            ValToGUI(vm, v, vm.GetTypeInfo(ti.subt), label, expanded);
            return;
    }
    string sd;
    v->ToString(vm, sd, ti, vm.debugpp);
    ImGui::LabelText(l, "%s", sd.c_str());  // FIXME: no formatting?
}

void VarsToGUI(VM &vm) {
    auto DumpVars = [&](bool constants) {
        for (uint32_t i = 0; i < vm.bcf->specidents()->size(); i++) {
            auto &val = vm.vars[i];
            auto sid = vm.bcf->specidents()->Get(i);
            auto id = vm.bcf->idents()->Get(sid->ididx());
            if (!id->global() || id->readonly() != constants) continue;
            auto name = id->name()->string_view();
            auto &ti = vm.GetVarTypeInfo(i);
            #if RTT_ENABLED
            if (ti.t != val.type) continue;  // Likely uninitialized.
            #endif
            ValToGUI(vm, &val, ti, name, false);
            if (IsStruct(ti.t)) i += ti.len - 1;
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

void AddIMGUI(NativeRegistry &nfr) {

nfr("im_init", "dark_style,flags", "B?I?", "", "",
    [](StackPtr &, VM &, Value &darkstyle, Value &flags) {
        if (imgui_init) return Value();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= (ImGuiConfigFlags)flags.ival();
        if (darkstyle.True()) ImGui::StyleColorsDark(); else ImGui::StyleColorsClassic();
        ImGui_ImplSDL2_InitForOpenGL(_sdl_window, _sdl_context);
        ImGui_ImplOpenGL3_Init("#version 150");
        imgui_init = true;
        return Value();
    });

nfr("im_add_font", "font_path,size", "SF", "B", "",
    [](StackPtr &sp, VM &vm, Value &fontname, Value &size) {
        IsInit(sp, vm, false);
        string buf;
        auto l = LoadFile(fontname.sval()->strv(), &buf);
        if (l < 0) return Value();
        auto mb = malloc(buf.size());  // FIXME.
        memcpy(mb, buf.data(), buf.size());
        ImFontConfig imfc;
        imfc.FontDataOwnedByAtlas = true;
        auto font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(mb, (int)buf.size(),
                                                               size.fltval(), &imfc);
        return Value(font != nullptr);
    });

nfr("im_frame", "body", "L", "", "",
    [](StackPtr &sp, VM &vm, Value &body) {
        IsInit(sp, vm, false);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(_sdl_window);
        ImGui::NewFrame();
        Push(sp,  Value());  // State value.
        return body;
    }, [](StackPtr &sp, VM &) {
        Pop(sp);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    });

nfr("im_window_demo", "", "", "B", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(sp, vm, false);
        bool show = true;
        ImGui::ShowDemoWindow(&show);
        return Value(show);
    });

nfr("im_window", "title,flags,body", "SIL", "", "",
    [](StackPtr &sp, VM &vm, Value &title, Value &flags, Value &body) {
        IsInit(sp, vm, false);
        ImGui::Begin(title.sval()->data(), nullptr, (ImGuiWindowFlags)flags.ival());
        imgui_windows++;
        Push(sp,  Value());  // State value.
        return body;
    }, [](StackPtr &sp, VM &) {
        Pop(sp);
        ImGui::End();
        imgui_windows--;
    });

nfr("im_button", "label,body", "SL", "", "",
    [](StackPtr &sp, VM &vm, Value &title, Value &body) {
        IsInit(sp, vm);
        auto press = ImGui::Button(title.sval()->data());
        Push(sp,  Value());  // State value.
        return press ? body : Value();
    }, [](StackPtr &sp, VM &) {
        Pop(sp);
    });

nfr("im_same_line", "", "", "", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(sp, vm);
        ImGui::SameLine();
        return Value();
    });

nfr("im_separator", "", "", "", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(sp, vm);
        ImGui::Separator();
        return Value();
    });

nfr("im_text", "label", "S", "", "",
    [](StackPtr &sp, VM &vm, Value &text) {
        IsInit(sp, vm);
        ImGui::Text("%s", text.sval()->data());
        return Value();
    });

nfr("im_tooltip", "label", "S", "", "",
    [](StackPtr &sp, VM &vm, Value &text) {
        IsInit(sp, vm);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", text.sval()->data());
        return Value();
    });

nfr("im_checkbox", "label,bool", "SI", "I2", "",
    [](StackPtr &sp, VM &vm, Value &text, Value &boolval) {
        IsInit(sp, vm);
        bool b = boolval.True();
        ImGui::Checkbox(text.sval()->data(), &b);
        return Value(b);
    });

nfr("im_input_text", "label,str", "SSk", "S", "",
    [](StackPtr &sp, VM &vm, Value &text, Value &str) {
        IsInit(sp, vm);
        return Value(LStringInputText(vm, text.sval()->data(), str.sval()));
    });

nfr("im_radio", "labels,active,horiz", "S]II", "I", "",
    [](StackPtr &sp, VM &vm, Value &strs, Value &active, Value &horiz) {
        IsInit(sp, vm);
        int sel = active.intval();
        for (iint i = 0; i < strs.vval()->len; i++) {
            if (i && horiz.True()) ImGui::SameLine();
            ImGui::RadioButton(strs.vval()->At(i).sval()->data(), &sel, (int)i);
        }
        return Value(sel);
    });

nfr("im_combo", "label,labels,active", "SS]I", "I", "",
    [](StackPtr &sp, VM &vm, Value &text, Value &strs, Value &active) {
        IsInit(sp, vm);
        int sel = active.intval();
        vector<const char *> items(strs.vval()->len);
        for (iint i = 0; i < strs.vval()->len; i++) {
            items[i] = strs.vval()->At(i).sval()->data();
        }
        ImGui::Combo(text.sval()->data(), &sel, items.data(), (int)items.size());
        return Value(sel);
    });

nfr("im_listbox", "label,labels,active,height", "SS]II", "I", "",
    [](StackPtr &sp, VM &vm, Value &text, Value &strs, Value &active, Value &height) {
        IsInit(sp, vm);
        int sel = active.intval();
        vector<const char *> items(strs.vval()->len);
        for (iint i = 0; i < strs.vval()->len; i++) {
            items[i] = strs.vval()->At(i).sval()->data();
        }
        ImGui::ListBox(text.sval()->data(), &sel, items.data(), (int)items.size(), height.intval());
        return Value(sel);
    });

nfr("im_sliderint", "label,i,min,max", "SIII", "I", "",
    [](StackPtr &sp, VM &vm, Value &text, Value &integer, Value &min, Value &max) {
        IsInit(sp, vm);
        int i = integer.intval();
        ImGui::SliderInt(text.sval()->data(), &i, min.intval(), max.intval());
        return Value(i);
    });

nfr("im_sliderfloat", "label,f,min,max", "SFFF", "F", "",
    [](StackPtr &sp, VM &vm, Value &text, Value &flt, Value &min, Value &max) {
        IsInit(sp, vm);
        float f = flt.fltval();
        ImGui::SliderFloat(text.sval()->data(), &f, min.fltval(), max.fltval());
        return Value(f);
    });

nfr("im_coloredit", "label,color", "SF}", "A2", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(sp, vm);
        auto c = PopVec<float4>(sp);
        ImGui::ColorEdit4(Pop(sp).sval()->data(), (float *)c.data());
        PushVec(sp, c);
    });

nfr("im_treenode", "label,body", "SL", "", "",
    [](StackPtr &sp, VM &vm, Value &title, Value &body) {
        IsInit(sp, vm);
        bool open = ImGui::TreeNode(title.sval()->data());
        PushAnyAsString(sp, vm, open);
        return open ? body : Value();
    }, [](StackPtr &sp, VM &) {
        bool open;
        PopAnyFromString(sp, open);
        if (open) ImGui::TreePop();
    });

nfr("im_group", "label,body", "SsL", "",
    "an invisble group around some widgets, useful to ensure these widgets are unique"
    " (if they have the same label as widgets in another group that has a different group"
    " label)",
    [](StackPtr &sp, VM &vm, Value &title, Value &body) {
        IsInit(sp, vm);
        ImGui::PushID(title.sval()->data());
        Push(sp,  Value());  // State value.
        return body;
    }, [](StackPtr &sp, VM &) {
        Pop(sp);
        ImGui::PopID();
    });

nfr("im_edit_anything", "value,label", "AkS?", "A1",
    "creates a UI for any lobster reference value, and returns the edited version",
    [](StackPtr &sp, VM &vm, Value &v, Value &label) {
        IsInit(sp, vm);
        // FIXME: would be good to support structs, but that requires typeinfo, not just len.
        auto &ti = vm.GetTypeInfo(v.True() ? v.ref()->tti : TYPE_ELEM_ANY);
        ValToGUI(vm, &v, ti,
                 label.True() ? label.sval()->strv() : "", true);
        return v;
    });

nfr("im_graph", "label,values,ishistogram", "SF]I", "", "",
    [](StackPtr &sp, VM &vm, Value &label, Value &vals, Value &histogram) {
        IsInit(sp, vm);
        auto getter = [](void *data, int i) -> float {
            return ((Value *)data)[i].fltval();
        };
        if (histogram.True()) {
            ImGui::PlotHistogram(label.sval()->data(), getter, vals.vval()->Elems(),
                (int)vals.vval()->len);
        } else {
            ImGui::PlotLines(label.sval()->data(), getter, vals.vval()->Elems(),
                (int)vals.vval()->len);
        }
        return Value();
    });

nfr("im_show_vars", "", "", "",
    "shows an automatic editing UI for each global variable in your program",
    [](StackPtr &sp, VM &vm) {
        IsInit(sp, vm);
        VarsToGUI(vm);
        return Value();
    });

nfr("im_show_engine_stats", "", "", "", "",
    [](StackPtr &sp, VM &vm) {
        IsInit(sp, vm);
        EngineStatsGUI();
        return Value();
    });

}  // AddIMGUI
