// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include "lobster/stdafx.h"
#include "lobster/engine.h"

#include "lobster/gltf_scene.h"

using namespace lobster;

typedef fgltf::Scene Scene;

ResourceType scene_type = {"scene", [](void *s) { delete (Scene *)s; }};

Scene &GetScene(VM &vm, Value &res) {
  return *GetResourceDec<Scene *>(vm, res, &scene_type);
}

Scene *LoadGLTF(string_view gltf_filename, string_view schema_filename) {
  Scene *scene = new fgltf::Scene;
  *scene = fgltf::LoadScene(gltf_filename, schema_filename);
  if (scene->roots.size() == 0) {
    delete scene;
    return nullptr;
  }
  return scene;
}

// found in graphics.cpp
extern void TestGL(VM &vm);

void AddGLTF(NativeRegistry &nfr) {

  nfr("gltf_new_scene", "gltf_filename,schema_filename", "SS", "R?",
      "Loads a .gltf file into a scene, returns scene or nil on failure to load.",
      [](VM &vm, Value &gfn, Value &sfn) {
        TestGL(vm);
        auto s = LoadGLTF(gfn.sval()->strv(), sfn.sval()->strv());
        return s ? Value(vm.NewResource(s, &scene_type)) : Value();
      });

  nfr("gltf_render_scene", "s", "R", "", "Renders the specified scene.",
      [](VM &vm, Value &i) {
        TestGL(vm);
        return Value(fgltf::DrawScene(GetScene(vm, i), otransforms.object2view, view2clip));
      });

} // AddGLTF
