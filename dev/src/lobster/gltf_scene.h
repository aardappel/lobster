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

#pragma once
#ifndef GLTF_SCENE_H
#    define GLTF_SCENE_H

#    include <functional>
#    include <string>
#    include <vector>

#    include <flatbuffers/idl.h>
#    include <stb/stb_image.h>

#    include "glincludes.h"
#    include "glinterface.h"

#    include "gltf_generated.h"

// Found in graphics.cpp
extern Shader *currentshader;

namespace fgl {

// TODO: pbr_anim shader
struct Vertex {
    float4 position;  // vec4 position is not necessary; GLSL auto-converts vec3 to vec4.
    float3 normal;
    float2 uv;
    float4 color;  // Not used in glTF but whatever, it's in the phong_anim shader.
    float4 weights;
    float4 indices;  // Floating-point integers. Thanks GL 2.0, aren't we past that?
};

typedef GLuint Index;

struct Mesh {
    bool success;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLsizei index_count;
};

Mesh new_mesh = { false, 0, 0, 0, 0 };

Mesh LoadMesh(std::vector<Vertex> &vertices, std::vector<Index> &indices, bool streaming = false) {
    Mesh mesh = new_mesh;

    LOG_DEBUG(to_string(streaming ? "STREAMING " : "") + "VERTEX ARRAY OBJECT\n");

    // Generate a VAO (1 per mesh).
    GL_CALL(glGenVertexArrays(1, &mesh.vao));
    // Switch graphics memory to the VAO.
    GL_CALL(glBindVertexArray(mesh.vao));

    LOG_DEBUG("VAO address:           " + to_string(mesh.vao));

    // Turn on 6 attribute arrays.
    GL_CALL(glEnableVertexAttribArray(0));  // Position
    GL_CALL(glEnableVertexAttribArray(1));  // Normal
    GL_CALL(glEnableVertexAttribArray(2));  // UV
    GL_CALL(glEnableVertexAttribArray(3));  // Color (not used)
    GL_CALL(glEnableVertexAttribArray(4));  // Weights
    GL_CALL(glEnableVertexAttribArray(5));  // Indices

    // Generate a vertex buffer and an index buffer.
    GL_CALL(glGenBuffers(1, &mesh.vbo));
    GL_CALL(glGenBuffers(1, &mesh.ibo));

    LOG_DEBUG("Vertex buffer address: " + to_string(mesh.vbo));
    LOG_DEBUG("Index buffer address:  " + to_string(mesh.ibo));

    // Switch to the vertex buffer.
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo));
    // Upload the vertices, passing the size in bytes.
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
                         streaming ? GL_STREAM_DRAW : GL_STATIC_DRAW));
    LOG_DEBUG("Number of vertices: " + to_string(vertices.size()));
    LOG_DEBUG("Size of Vertex:     " + to_string(sizeof(Vertex)));

    // Perhaps an inconsistent use of data here but I'm not sure what to do.
    // sizeof returns a size_t which can be cast to anything since it's just a
    // number.
    std::size_t pointer = 0;
    GL_CALL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (const GLvoid *)pointer));  // Position
    pointer += sizeof(vertices[0].position);
    GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (const GLvoid *)pointer));  // Normal
    pointer += sizeof(vertices[0].normal);
    GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (const GLvoid *)pointer));  // UV
    pointer += sizeof(vertices[0].uv);
    GL_CALL(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (const GLvoid *)pointer));  // Color (not used)
    pointer += sizeof(vertices[0].color);
    GL_CALL(glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (const GLvoid *)pointer));  // Weights
    pointer += sizeof(vertices[0].weights);
    GL_CALL(glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (const GLvoid *)pointer));  // Indices

    // Switch to the index buffer.
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo));
    // Upload the indices, passing the size in bytes.
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), &indices[0],
                         streaming ? GL_STREAM_DRAW : GL_STATIC_DRAW));
    LOG_DEBUG("Number of indices: " + to_string(indices.size()));
    LOG_DEBUG("Size of Index:     " + to_string(sizeof(Index)) + "\n");

    // Switch out of the VAO.
    GL_CALL(glBindVertexArray(0));

    // Failure conditions are currently unimplemented.
    mesh.success = true;

    // Store the number of indices.
    mesh.index_count = indices.size();

    return mesh;
}

void DrawMesh(Mesh &mesh, float4x4 model_view_mat, float4x4 proj_mat) {
    if (!mesh.success) return;

    // TODO FEATURE: No model matrix means no normal matrix, but sacrifices must
    // be made. :/
    float4x4 mvp = model_view_mat * proj_mat;

    GL_CALL(glUniformMatrix4fv(currentshader->mvp_i, 1, GL_FALSE, mvp.data()));
    GL_CALL(glUniform4f(currentshader->col_i, 1.0f, 1.0f, 1.0f, 1.0f));

    // Switch graphics memory to the VAO.
    GL_CALL(glBindVertexArray(mesh.vao));

    GL_CALL(glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, (const GLvoid *)0));
}

}  // namespace fgl

namespace fgltf {

size_t max_tree_depth = 1000000;

struct SceneMaterial {
    int base_color_image;
    GLfloat metallic_factor;
    GLfloat roughness_factor;
};

SceneMaterial new_scene_material = { -1, 0.0f, 0.5f };

struct ScenePrim {
    fgl::Mesh mesh;
    int material;
    int mode;
};

typedef std::vector<ScenePrim> SceneMesh;

struct SceneNode {
    int mesh;
    float3 translation;
    float4 rotation;
    float3 scale;
    std::vector<int> children;
};

struct SceneAnimationData {
    int node;
    int type;
    int interpolation;
    std::vector<float3> translations;
    std::vector<float4> rotations;
    std::vector<float3> scales;
    std::vector<float> weights;
};

struct SceneAnimation {
    std::string name;
    std::vector<SceneAnimationData> channels;
};

struct Scene {
    std::vector<SceneMaterial> materials;
    std::vector<SceneMesh> meshes;
    std::vector<SceneNode> nodes;
    std::vector<SceneAnimation> animations;
    std::vector<int> roots;
};

Scene new_scene = { {}, {}, {}, {}, {} };

struct Image {
    std::string path;
    Texture texture;
};

std::vector<Image> images;

int LoadImage(string_view gltf_path, string_view img_name, int tf = TF_NONE) {
    // loads an image relative to the specified gltf path.
    // Never loads the same image twice in the same process.

    // Set the file path to the same folder as the glTF file.
    std::string img_file_path = gltf_path.substr(0, gltf_path.find_last_of('/') + 1) + img_name;
    // Try to find the image among already loaded images.
    for (size_t i = 0; i < images.size(); i++)
        if (images[i].path == img_file_path) return i;
    // No image found, so load the named file.
    // TODO: Produce an error if texture loading fails.
    Texture img_texture = CreateTextureFromFile(img_file_path, tf);
    images.push_back({ img_file_path, img_texture });
    return images.size() - 1;
}

Scene LoadScene(string_view gltf_path, string_view schema_path) {
    LOG_DEBUG("GLTF SCENE");
    try {
        std::string gltf_text;
        if (LoadFile(gltf_path, &gltf_text) < 0) {
            LOG_ERROR("Failed to open " + gltf_path);
            return new_scene;
        }

        std::string schema_text;
        if (LoadFile(schema_path, &schema_text) < 0) {
            LOG_ERROR("Failed to open " + schema_path);
            return new_scene;
        }

        flatbuffers::Parser parser;

        if (!parser.Parse(schema_text.c_str()) || !parser.Parse(gltf_text.c_str())) {
            LOG_ERROR("Failed to parse " + gltf_path + " via " + schema_path + " schema");
            return new_scene;
        }

        // Info contains the document hierarchy.
        auto info = gltf::GetGLTF(parser.builder_.GetBufferPointer());

        LOG_DEBUG("glTF version: " + info->asset()->version()->str());

        std::vector<std::string> buffers;

        if (info->buffers()) {
            // Load binary buffers from files.
            for (size_t i = 0; i < info->buffers()->size(); i++) {
                auto b = info->buffers()->Get(i);
                // Set the bin file path to the same folder as the glTF file.
                std::string bin_file_path =
                    gltf_path.substr(0, gltf_path.find_last_of('/') + 1) + b->uri()->str();
                // Load the data into the buffer.
                buffers.resize(buffers.size() + 1);
                if (LoadFile(bin_file_path.c_str(), &buffers.back()) < 0) {
                    LOG_ERROR("Failed to open " + bin_file_path);
                    return new_scene;
                }
            }
        }

        LOG_DEBUG("Buffers:      " + to_string(buffers.size()));

        struct BufferView {
            int buffer;
            int length;
            int offset;
            int stride;
        };

        std::vector<BufferView> bufferviews;

        // Copy bufferViews into bufferviews, non-existent values becoming 0.
        if (info->bufferViews()) {
            for (size_t i = 0; i < info->bufferViews()->size(); i++) {
                auto v = info->bufferViews()->Get(i);
                bufferviews.push_back(
                    { v->buffer(), v->byteLength(), v->byteOffset(), v->byteStride() });
            }
        }

        LOG_DEBUG("BufferViews:  " + to_string(bufferviews.size()));

        struct Accessor {
            int buffer_view;
            int offset;
            int component_type;
            int count;
            std::string type;
            std::vector<float> minf;
            std::vector<float> maxf;
            std::vector<int> mini;
            std::vector<int> maxi;
        };

        std::vector<Accessor> accessors;

        if (info->accessors()) {
            for (size_t i = 0; i < info->accessors()->size(); i++) {
                auto a = info->accessors()->Get(i);
                int ct = a->componentType();
                std::vector<float> minf, maxf;
                std::vector<int> mini, maxi;

                // TODO: FlatBuffers is strongly typed, glTF is not.
                // floats are not ints. (Maybe use unions?)
                if (a->min() && a->max()) {
                    for (size_t x = 0; x < a->min()->size(); x++) {
                        if (ct == 5126) {
                            minf.push_back(a->min()->Get(x));
                            maxf.push_back(a->max()->Get(x));
                        } else {
                            mini.push_back((int)a->min()->Get(x));
                            maxi.push_back((int)a->max()->Get(x));
                        }
                    }
                }

                accessors.push_back({ a->bufferView(), a->byteOffset(), ct, a->count(),
                                      a->type()->str(), minf, maxf, mini, maxi });
            }
        }

        LOG_DEBUG("Accessors:    " + to_string(accessors.size()) + "\n");

        Scene scene = new_scene;

        if (info->materials()) {
            for (size_t i = 0; i < info->materials()->size(); i++) {
                auto m = info->materials()->Get(i);
                SceneMaterial mat;

                if (m->pbrMetallicRoughness()) {
                    auto pbr = m->pbrMetallicRoughness();
                    if (pbr->baseColorTexture()) {
                        int idx = info->textures()->Get(pbr->baseColorTexture()->index())->source();
                        std::string img_name = info->images()->Get(idx)->uri()->str();
                        LOG_DEBUG("Loading \"" + img_name + "\" for baseColorTexture\n");
                        mat.base_color_image = LoadImage(gltf_path, img_name.c_str());
                    }
                    mat.metallic_factor = pbr->metallicFactor();
                    mat.roughness_factor = pbr->roughnessFactor();
                }

                scene.materials.push_back(mat);
            }
        }

        if (info->meshes()) {
            for (size_t i = 0; i < info->meshes()->size(); i++) {
                auto m = info->meshes()->Get(i);
                SceneMesh primarr;
                for (size_t j = 0; j < m->primitives()->size(); j++) {
                    auto p = m->primitives()->Get(j);
                    int mode = p->mode() > 0 ? p->mode() : 4;
                    LOG_DEBUG("Primitive is " + to_string(mode == 4 ? "" : "not ") +
                              "made out of triangles.");
                    std::vector<fgl::Vertex> verts;
                    std::vector<fgl::Index> inds;

                    // TODO: Maybe verify this.
                    // if( p->attributes()->Verify( (((a verifier))) ) ){
                    // TODO: The 0-indexed accessors array probably results in bogus
                    // values for undefined attributes.
                    // FlatBuffers sets undefined values to 0 if undefined attributes aren't used,
                    // it's fine.
                    int at_indices[4] = { p->attributes()->POSITION(), p->attributes()->NORMAL(),
                                          p->attributes()->TANGENT(),
                                          p->attributes()->TEXCOORD_0() };

                    for (int at_type = 0; at_type < 4; at_type++) {
                        Accessor a = accessors[at_indices[at_type]];
                        int count = a.count;
                        BufferView bv = bufferviews[a.buffer_view];
                        int b_stride = bv.stride;
                        // According to the glTF 2.0 spec:
                        //   "When `byteStride` of referenced `bufferView` is not defined,
                        //   it means that... effective stride equals the size of the
                        //   element."
                        if (b_stride == 0 && a.type == "VEC3") {
                            // 3 components * 4 bytes
                            b_stride = 12;
                        } else if (b_stride == 0 && a.type == "VEC2") {
                            // 2 components * 4 bytes
                            b_stride = 8;
                        }
                        // We only use floats for vertex attributes.
                        if (a.component_type != 5126) count = 0;
                        if (count > (int)verts.size()) verts.resize(count);
                        // Create a GLfloat array from the vertex attribute buffer offset.
                        GLfloat *v_buf =
                            reinterpret_cast<GLfloat *>(&buffers[bv.buffer][bv.offset + a.offset]);
                        LOG_DEBUG("Buffer: " + to_string(bv.buffer));
                        LOG_DEBUG("Byte offset: " + to_string(bv.offset + a.offset));
                        LOG_DEBUG("Byte stride: " + to_string(b_stride));
                        LOG_DEBUG(to_string(count) + " vertex attribute vectors");
                        for (int i = 0; i < count; i++) {
                            GLfloat *v_dat = &v_buf[b_stride / 4 * i];
                            switch (at_type) {
                                case 0:
                                    verts[i].position[0] = v_dat[0];
                                    verts[i].position[1] = v_dat[1];
                                    verts[i].position[2] = v_dat[2];
                                    verts[i].position[3] = 1.0f;  // Better to divide by 1 than 0.
                                    break;
                                case 1:
                                    verts[i].normal[0] = v_dat[0];
                                    verts[i].normal[1] = v_dat[1];
                                    verts[i].normal[2] = v_dat[2];
                                    break;
                                case 2:  // Tangents are not supported by the phong_anim shader.
                                    // verts[i].tangent[0] = v_dat[0];
                                    // verts[i].tangent[1] = v_dat[1];
                                    // verts[i].tangent[2] = v_dat[2];
                                    break;
                                case 3:
                                    verts[i].uv[0] = v_dat[0];
                                    verts[i].uv[1] = v_dat[1];
                            }
                        }
                        LOG_DEBUG(" ");  // A blank line.
                    }
                    //}else{
                    //	_ERROUT_ << "Primitive does not have vertex attributes." <<
                    // std::endl;
                    //}

                    // TODO: Really need to verify this.
                    // if( p["indices"].IsNumeric() ){
                    Accessor a = accessors[p->indices()];
                    int ind_type = a.component_type;
                    int num_inds = a.count;
                    BufferView bv = bufferviews[a.buffer_view];
                    int b_stride = bv.stride;
                    // Create an untyped char array from the index buffer offset, to be
                    // converted to a type.
                    char *ind_buf = &buffers[bv.buffer][bv.offset + a.offset];
                    for (int i = 0; i < num_inds; i++) {
                        fgl::Index index;
                        // Get a value based on the data type indType, the array index i,
                        // and the byte stride bStride.
                        switch (ind_type) {
                            case GL_BYTE: index = (fgl::Index)ind_buf[(b_stride || 1) * i]; break;
                            case GL_UNSIGNED_BYTE:
                                index = (fgl::Index)(reinterpret_cast<unsigned char *>(
                                    ind_buf))[(b_stride || 1) * i];
                                break;
                            case GL_SHORT:
                                index = (fgl::Index)(
                                    reinterpret_cast<short *>(ind_buf))[(b_stride / 2 || 1) * i];
                                break;
                            case GL_UNSIGNED_SHORT:
                                index = (fgl::Index)(reinterpret_cast<unsigned short *>(
                                    ind_buf))[(b_stride / 2 || 1) * i];
                                break;
                            case GL_UNSIGNED_INT:
                                index = (fgl::Index)(reinterpret_cast<unsigned int *>(
                                    ind_buf))[(b_stride / 4 || 1) * i];
                                break;
                            default: index = 0;
                        }
                        inds.push_back(index);
                    }
                    //}else{
                    //	_ERROUT_ << "Primitive does not have vertex indices." <<
                    // std::endl;
                    //}

                    if (inds.size() >= 6) {
                        LOG_DEBUG("Triangle 0: " + to_string(inds[0]) + " " + to_string(inds[1]) +
                                  " " + to_string(inds[2]));
                        LOG_DEBUG("Triangle 1: " + to_string(inds[3]) + " " + to_string(inds[4]) +
                                  " " + to_string(inds[5]));
                    }

                    LOG_DEBUG(" ");  // A blank line.

                    primarr.push_back({ fgl::LoadMesh(verts, inds), p->material(), mode });
                }
                scene.meshes.push_back(primarr);
            }
        }

        LOG_DEBUG("Meshes:       " + to_string(scene.meshes.size()));

        // Copy nodes into scene.nodes
        if (info->nodes()) {
            for (size_t i = 0; i < info->nodes()->size(); i++) {
                auto n = info->nodes()->Get(i);
                std::vector<int> children;
                if (n->children())
                    for (size_t j = 0; j < n->children()->size(); j++)
                        children.push_back(n->children()->Get(j));
                SceneNode new_node = { n->mesh(),
                                       { 0.0f, 0.0f, 0.0f },        // translation
                                       { 0.0f, 0.0f, 0.0f, 1.0f },  // rotation
                                       { 1.0f, 1.0f, 1.0f },        // scale
                                       children };
                // Not all glTF files have translation, rotation, and scale for each node.
                // Here we check for the existence of each property.
                if (n->translation() && n->translation()->size() >= 3)
                    new_node.translation = { n->translation()->Get(0), n->translation()->Get(1),
                                             n->translation()->Get(2) };
                if (n->rotation() && n->rotation()->size() >= 4)
                    new_node.rotation = { n->rotation()->Get(0), n->rotation()->Get(1),
                                          n->rotation()->Get(2), n->rotation()->Get(3) };
                if (n->scale() && n->scale()->size() >= 3)
                    new_node.scale = { n->scale()->Get(0), n->scale()->Get(1), n->scale()->Get(2) };
                scene.nodes.push_back(new_node);
            }
        }

        LOG_DEBUG("Total nodes:  " + to_string(scene.nodes.size()) + "\n");

        if (info->animations()) {
            for (size_t i = 0; i < info->animations()->size(); i++) {
                auto an = info->animations()->Get(i);
                scene.animations.resize(scene.animations.size() + 1);
                scene.animations.back().name = an->name()->str();
                for (size_t j = 0; j < an->samplers()->size(); j++) {
                    auto s = an->samplers()->Get(j);
                    LOG_DEBUG("Input max:    " + to_string(accessors[s->input()].maxf[0]));
                    // TODO: Make this not crash the program.
                    // LOG_DEBUG("Output max:    " + to_string(accessors[s->output()].maxf[0]));
                }
                for (size_t j = 0; j < an->channels()->size(); j++) {
                    auto c = an->channels()->Get(i);
                }
            }
        }

        LOG_DEBUG("Animations:   " + to_string(scene.animations.size()) + "\n");

        // Copy scenes[scene].nodes into scene.roots
        auto info_scene_nodes = info->scenes()->Get(info->scene())->nodes();
        for (size_t i = 0; i < info_scene_nodes->size(); i++)
            scene.roots.push_back(info_scene_nodes->Get(i));

        LOG_DEBUG("Root nodes:   " + to_string(scene.roots.size()) + "\n");

        return scene;
    } catch (const std::exception &e) { LOG_ERROR("Caught exception: " + to_string(e.what())); }
    return new_scene;
}

void SetSceneAnimation(Scene &scene, int anim, double anim_time) {
    SceneAnimation &animation = scene.animations[anim];
}

int DrawScene(Scene scene, float4x4 model_view_mat, float4x4 proj_mat) {
    int error = 0;
    int current_material = -1;
    // https://shaharmike.com/cpp/lambdas-and-functions/
    std::function<void(int, float4x4, size_t)> DrawTree = [&](int n, float4x4 m, size_t depth) {
        depth++;
        if (depth > max_tree_depth) {
            error = 1;
            return;
        }
        SceneNode &node = scene.nodes[n];
        // T * R * S * M
        // mul( M, mul( mul( S, R ), T ) )
        /*
        m = linalg::mul(
                m,
                linalg::mul(
                        linalg::mul(
                                linalg::scaling_matrix( node.scale ),
                                linalg::rotation_matrix( node.rotation )
                        ),
                        linalg::translation_matrix( node.translation )
                )
        );
        */
        m = translation(node.translation) * float3x3to4x4(rotation(node.rotation)) *
            scaling(node.scale) * m;
        for (ScenePrim &p : scene.meshes[node.mesh])
            if (p.mode == 4) {
                if (p.material != current_material) {
                    current_material = p.material;
                    auto &mat = scene.materials[p.material];
                    if (mat.base_color_image > -1)
                        SetTexture(0, images[mat.base_color_image].texture, TF_NONE);
                }
                fgl::DrawMesh(p.mesh, m, proj_mat);
            }
        for (int c : node.children) DrawTree(c, m, depth);
    };
    for (int r : scene.roots) DrawTree(r, model_view_mat, 0);
    return error;
}

}  // namespace fgltf

#endif  // GLTF_SCENE_H
