# Meshes & 3D Models

## What a 3D model actually is

A **mesh** is a list of triangles, stored efficiently as two GPU buffers:

- **Vertex buffer** — array of vertices. Each vertex has a position, plus (usually):
  - normal (for lighting)
  - UV coords (for textures)
  - tangent/bitangent (for normal mapping)
  - vertex color / skin weights (for skeletal animation)
- **Index buffer** — array of integers, 3 per triangle, each indexing into the vertex buffer. Avoids duplicating vertices shared between triangles.

Models exported from Blender/Maya/etc. typically carry more than raw geometry:

| Attached data       | Purpose                                                                  |
|---------------------|--------------------------------------------------------------------------|
| **Materials**       | Per-submesh shader inputs: base color, metallic/roughness, emissive, etc. |
| **Textures**        | Images referenced by materials — albedo, normal, roughness, AO.          |
| **Skeleton / bones**| Hierarchy of joints for rigged models.                                   |
| **Skin weights**    | Per-vertex bone influences.                                              |
| **Animations**      | Keyframed bone poses over time.                                          |
| **Node graph**      | Scene-level transforms — a model can have multiple meshes parented.      |

Quads and n-gons in Blender get **triangulated on export** — GPUs only draw triangles.

## How a triangle is stored

A flat array of floats, interpreted 3 at a time as `(x, y, z)`:

```cpp
std::vector<float> vertices{
     0.0F,  0.5F, 0.0F,   // vertex 0 — top
    -0.5F, -0.5F, 0.0F,   // vertex 1 — bottom-left
     0.5F, -0.5F, 0.0F,   // vertex 2 — bottom-right
};
```

Visualized on the XY plane (Z = 0):

```
          (0, 0.5, 0)
               •
              / \
             /   \
            /     \
           /       \
          •_________•
  (-0.5,-0.5,0)   (0.5,-0.5,0)
```

The values `-1..+1` are **normalized device coordinates (NDC)**: the whole window maps to `[-1, +1]` on X and Y. So this triangle sits centered, filling ~half the window height.

### Telling OpenGL how to read the buffer

Two pieces of metadata:

**1. Attribute layout** — "every 3 floats = one position":

```cpp
glVertexAttribPointer(
    0,                  // attribute location in the shader
    3,                  // components per vertex (x, y, z)
    GL_FLOAT,           // component type
    GL_FALSE,           // no normalization
    3 * sizeof(float),  // stride — bytes between vertices
    (void*)0            // offset to first component
);
```

**2. Draw call** — "group every 3 vertices into a triangle":

```cpp
glDrawArrays(GL_TRIANGLES, 0, 3);
```

Swap `GL_TRIANGLES` for `GL_LINE_LOOP` → outline. For `GL_POINTS` → three dots. The buffer is just numbers; the primitive type gives them meaning.

## A real vertex

Once you add normals + UVs, one vertex is 8 floats:

```
[ px py pz ][ nx ny nz ][ u v ]
   position    normal     uv
```

Stride becomes `8 * sizeof(float)`, and you call `glVertexAttribPointer` three times — once per attribute — with different offsets.

## Indexed drawing

Real meshes share vertices between triangles. Use an **index buffer (EBO)** and call `glDrawElements` instead:

```cpp
float vertices[] = { /* 4 unique quad corners */ };
unsigned indices[] = {
    0, 1, 2,   // triangle 1
    0, 2, 3,   // triangle 2
};
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
```

## File formats

| Format          | When to use |
|-----------------|-------------|
| **glTF 2.0** (`.gltf` / `.glb`) | **Modern default.** Khronos standard, PBR-native, great Blender exporter. |
| **FBX**         | Industry standard for authoring. Best animation/rigging support. Proprietary. |
| **OBJ** + MTL   | Ancient, text-based, simple. No animation or skeletons. Great for learning. |
| **DAE** (Collada) | XML-based. Mostly superseded by glTF. |
| **STL / PLY**   | 3D printing / scanning. No materials. |

Recommendation for jpengine-3d: **OBJ first**, then **glTF** once you need animation and PBR.

## Importing into C++

### 1. Assimp — the standard

Reads ~40 formats into a unified `aiScene` tree:

```cpp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Assimp::Importer importer;
const aiScene* scene = importer.ReadFile("assets/model.glb",
    aiProcess_Triangulate |
    aiProcess_FlipUVs |
    aiProcess_CalcTangentSpace |
    aiProcess_GenNormals);

for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[i];
    // mesh->mVertices, mesh->mNormals, mesh->mTextureCoords[0], mesh->mFaces
}
```

### 2. Format-specific single-header libraries (lean)

- **cgltf** — single-header C library for glTF. ~200 KB.
- **tinyobjloader** — single-header for OBJ. ~100 KB.
- **ufbx** — recent, impressive single-header FBX loader.

## Load pipeline

```
Blender  →  export .glb  →  Assimp/cgltf parses  →  upload to GPU (VBO + EBO)
                                                 →  load textures (stb_image)
                                                 →  shader draws triangles
```

For each mesh at load time:
1. Walk `aiScene` / glTF nodes.
2. Upload vertex + index buffers via `glBufferData` → VBO + EBO wrapped in a VAO.
3. Load referenced textures and upload with `glTexImage2D`.

At draw time:
1. Bind the VAO.
2. Bind textures to texture units.
3. Set shader uniforms (model / view / projection matrices).
4. `glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0)`.
