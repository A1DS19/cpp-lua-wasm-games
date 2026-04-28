# 12 — Loading Meshes from glTF

> Previous: [11 — Textures](./engine-11-textures.md) · Next: [13 — Scene graph and components](./engine-13-scene-graph.md)

Hardcoding vertex arrays is fine for one cube. For anything authored in Blender (a character, a prop, a level mesh), you load a 3D file format. We use **glTF 2.0** as the engine's input format, and **cgltf** as the loader.

This note covers:

- Why glTF and cgltf.
- The shape of a glTF file.
- The `Mesh::load(path)` implementation.
- Coordinate-system gotchas (UV flip, attribute slot collisions).

---

## Why glTF, why cgltf

| | Why |
|---|---|
| **glTF 2.0** | Khronos-standard, JSON+binary, PBR-native, excellent Blender exporter, modern. |
| **cgltf** | Single-header C library (`vendor/cgltf/cgltf.h` — ~5k lines). Parses glTF into a clean C struct tree. No CMake dependency cost. |

The alternative, **Assimp**, supports 40+ formats but is a 1.5M-line C++ project with its own subdirectory build. Overkill if you control your own art pipeline. cgltf integrates in two CMake lines and one impl file (`src/utils/cgltf-impl.cpp` — same single-header pattern as stb_image, see [doc 11](./engine-11-textures.md)).

If you ever need FBX or OBJ, you swap loaders behind the same `Mesh::load` API.

---

## What's in a glTF file

A `.gltf` file is JSON describing the scene structure. It references `.bin` files (vertex/index/animation data) and image files (textures), all by relative path:

```
assets/models/suzanne/
├── Suzanne.gltf                  ← JSON: meshes, materials, references
├── Suzanne.bin                   ← raw binary buffers
├── Suzanne_BaseColor.png
└── Suzanne_MetallicRoughness.png
```

Inside `Suzanne.gltf`, accessors describe how to slice the binary buffer:

```json
{
    "meshes": [
        {
            "primitives": [{
                "attributes": {
                    "POSITION":   0,    // index into `accessors`
                    "NORMAL":     1,
                    "TEXCOORD_0": 2
                },
                "indices":  3,
                "material": 0
            }]
        }
    ],
    "accessors": [
        { "bufferView": 0, "componentType": 5126, "count": 507, "type": "VEC3" },
        // ...
    ],
    "buffers": [{"uri": "Suzanne.bin", "byteLength": 12000}]
}
```

Each accessor is an `(offset, count, type)` view into a binary buffer. cgltf walks all of this and gives you typed pointers — you don't parse JSON yourself.

### Hierarchy in glTF

| Level | What it is |
|---|---|
| **Buffer** | A blob of bytes (a `.bin` file or embedded base64). |
| **BufferView** | A range inside a buffer (offset + length + stride). |
| **Accessor** | A typed view of a BufferView ("507 vec3 floats starting at offset 0"). |
| **Primitive** | One drawable thing: pos/normal/uv accessors, an index accessor, a material reference. |
| **Mesh** | A list of primitives (e.g. body, eyes, hair — each with their own material). |
| **Node** | A transform in the scene tree, optionally pointing at a mesh. |

Our loader handles meshes with **one primitive** for now. Multi-primitive meshes (e.g. one for the body, one for the eyes) need per-primitive materials — see "What's missing" at the bottom.

---

## `Mesh::load(path)` — the high-level shape

```cpp
std::shared_ptr<Mesh> Mesh::load(const std::string& path) {
    // 1. Read the .gltf JSON as text.
    auto contents = utils::read_asset_text(path);
    if (contents.empty()) return nullptr;

    // 2. Parse the JSON.
    cgltf_data* data = nullptr;
    cgltf_options options = {};
    if (cgltf_parse(&options, contents.data(), contents.size(), &data) != cgltf_result_success) {
        return nullptr;
    }

    // 3. Load the binary buffers (.bin, image URIs).
    //    The 3rd arg is the *path of the .gltf*; cgltf takes dirname() of it
    //    to resolve the relative URIs inside the JSON ("Suzanne.bin").
    if (cgltf_load_buffers(&options, data, utils::asset_path(path).string().c_str())
            != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    // 4. Walk meshes/primitives, build interleaved vertex data, build a Mesh.
    std::shared_ptr<Mesh> result;
    for (cgltf_size mi = 0; mi < data->meshes_count; ++mi) {
        for (cgltf_size pi = 0; pi < data->meshes[mi].primitives_count; ++pi) {
            // …assemble VertexLayout, vertices vector, indices vector…
            result = std::make_shared<Mesh>(layout, vertices, indices);
            if (result) break;
        }
        if (result) break;
    }

    cgltf_free(data);
    return result;
}
```

The hard part is step 4 — converting glTF's "separate accessors per attribute" into our engine's "interleaved float array" format.

---

## Building the interleaved buffer

glTF stores each attribute (position, normal, uv) as its own accessor with its own buffer offset. Our `Mesh` expects one interleaved buffer where each vertex is a contiguous block:

```
glTF storage:                      engine storage:
positions: [p0,p1,p2,...]          vertex 0: [px,py,pz, nx,ny,nz, u,v]
normals:   [n0,n1,n2,...]          vertex 1: [px,py,pz, nx,ny,nz, u,v]
uvs:       [u0,u1,u2,...]          ...
```

For each attribute the loader does:

1. Find the right slot (position, color, uv, or normal).
2. Compute its offset within the vertex (running count of bytes used so far).
3. Push a `VertexElement` into the layout describing that slot.

```cpp
VertexElement element;
element.type_ = GL_FLOAT;
switch (attr.type) {
    case cgltf_attribute_type_position: {
        accesors[VertexElement::position_index_] = acc;
        element.index_ = VertexElement::position_index_;
        element.size_  = 3;
        break;
    }
    case cgltf_attribute_type_normal: {
        accesors[VertexElement::normal_index_] = acc;
        element.index_ = VertexElement::normal_index_;
        element.size_  = 3;
        break;
    }
    case cgltf_attribute_type_texcoord: {
        if (attr.index != 0) continue;     // we only use TEXCOORD_0
        accesors[VertexElement::uv_index_] = acc;
        element.index_ = VertexElement::uv_index_;
        element.size_  = 2;                // UVs are vec2
        break;
    }
    case cgltf_attribute_type_color: {
        if (attr.index != 0) continue;
        accesors[VertexElement::color_index_] = acc;
        element.index_ = VertexElement::color_index_;
        element.size_  = 3;
        break;
    }
}

element.offset_ = vertex_layout.stride_;
vertex_layout.stride_ += element.size_ * sizeof(float);
vertex_layout.elements_.push_back(element);
```

Then a doubly-nested loop fills the interleaved buffer:

```cpp
auto vertex_count = accesors[VertexElement::position_index_]->count;
std::vector<float> vertices((vertex_layout.stride_ / sizeof(float)) * vertex_count);

for (cgltf_size vi = 0; vi < vertex_count; ++vi) {
    for (auto& el : vertex_layout.elements_) {
        auto idx = (vi * vertex_layout.stride_ + el.offset_) / sizeof(float);
        cgltf_accessor_read_float(accesors[el.index_], vi, &vertices[idx], el.size_);

        // glTF UV origin is top-left; OpenGL is bottom-left. Reconcile.
        if (el.index_ == VertexElement::uv_index_ && el.size_ >= 2) {
            vertices[idx + 1] = 1.0F - vertices[idx + 1];
        }
    }
}
```

`cgltf_accessor_read_float` does the heavy lifting — it handles different component types (float/int8/uint16) and normalization rules, returning a normalized float result.

---

## Three gotchas you will hit

Each of these took real time to track down. Worth knowing in advance.

### 1. Distinct attribute slots

```cpp
// vertex-layout.hpp
struct VertexElement {
    static constexpr int position_index_ = 0;
    static constexpr int color_index_    = 1;
    static constexpr int uv_index_       = 2;
    static constexpr int normal_index_   = 3;
};
```

These constants serve **two purposes simultaneously**:

- They're the index into the `accesors[]` array in `Mesh::load`.
- They're the `layout(location = N)` value passed to `glVertexAttribPointer`.

If they all collide (e.g. all `0`), every glTF attribute writes to `accesors[0]`, overwriting the previous one. Only the last attribute survives, and the rest of the data (color, normal) silently disappears. Symptom: model has correct topology but everything is at the origin or has zero color.

Each constant must be a distinct integer, and the values must match the `layout(location = N)` declarations in your shaders.

### 2. `cgltf_load_buffers` base path

```cpp
// CORRECT:
cgltf_load_buffers(&options, data, utils::asset_path(path).string().c_str());
// path = "models/suzanne/Suzanne.gltf"
// arg  = "/<assets>/models/suzanne/Suzanne.gltf"
// cgltf does dirname() → "/<assets>/models/suzanne/"
// resolves "Suzanne.bin" → "/<assets>/models/suzanne/Suzanne.bin" ✓
```

```cpp
// WRONG:
cgltf_load_buffers(&options, data, utils::asset_path("models").string().c_str());
// arg  = "/<assets>/models"
// cgltf does dirname() → "/<assets>/"  (strips "models")
// tries to resolve "Suzanne.bin" → "/<assets>/Suzanne.bin" ✗ NOT FOUND
```

cgltf treats the third argument as the **path of the .gltf file itself** and uses `dirname()` to find the binary buffers. Pass anything else and the buffers fail to load. Symptom: `Mesh::load` returns nullptr, no model appears.

### 3. UV vertical flip for glTF

glTF stores UVs with V=0 at the top of the image. OpenGL convention (and your `stbi_set_flip_vertically_on_load(true)`) treats V=0 as the bottom. Without compensating, glTF UVs sample the wrong vertical region of the texture.

The fix is one line in the vertex-copy loop:

```cpp
if (el.index_ == VertexElement::uv_index_ && el.size_ >= 2) {
    vertices[idx + 1] = 1.0F - vertices[idx + 1];   // V → 1-V
}
```

Symptom: most of the model looks roughly textured, but small distinctive features (like Suzanne's eyes) sample the wrong pixels — they end up the wrong color while gradient-y regions look acceptable because nearby pixels are similar.

---

## What's missing (for now)

- **Multi-primitive meshes.** A mesh with eyes-as-a-separate-primitive can't be represented by one `Mesh` object today. The loader just keeps the first primitive. To handle this properly: `Mesh::load` returns a `std::vector<std::shared_ptr<Mesh>>`, each with its own material reference; `MeshComponent` holds one of each.
- **Material extraction.** glTF carries PBR material info (`baseColorTexture`, `metallicRoughnessTexture`, `baseColorFactor`). We currently ignore it — you still author a separate `.mat` JSON. Bridging would mean reading `primitive.material->pbr_metallic_roughness.*` and constructing a Material from it.
- **Skinning / animation.** Vertex skinning attributes (`JOINTS_0`, `WEIGHTS_0`) and the animation sampler accessors are completely unwired.
- **Embedded buffers (`.glb`).** Binary glTF stores everything in one file. cgltf supports it, but our `read_asset_text` only handles JSON `.gltf`. To support `.glb`, switch the parsing path to `cgltf_parse_file` directly.

Each of these is a fork in the road — pick one when the engine needs it.

---

## What could go wrong

1. **Symbol-collision linker errors.** `mesh.cpp` defining `CGLTF_IMPLEMENTATION` AND `cgltf-impl.cpp` defining it. Only the impl TU should.
2. **Crashes in `cgltf_accessor_read_float`.** Usually means the `accesors` array is too small (you added a 4th attribute but the array is `[3]`). Resize to match the highest index.
3. **Model renders but is at the origin and tiny.** Probably attribute slot collision (gotcha #1). Position attr was overwritten; vertices defaulted to (0,0,0).
4. **Model appears correctly shaped but textures are wrong.** UV flip (gotcha #3) or texture not loaded.
5. **`Mesh::load` returns nullptr.** `cgltf_load_buffers` failed (gotcha #2) or the file doesn't exist.

---

## See also

- [06 — Meshes](./engine-06-meshes.md) — the Mesh class itself; the constructor `Mesh::load` calls.
- [11 — Textures](./engine-11-textures.md) — sister loader for the image side.
- [meshes-and-models.md](./meshes-and-models.md) — broader file-format primer.
- [coordinates.md](./coordinates.md) — right-handed vs left-handed; the UV-origin caveat lives here too.
