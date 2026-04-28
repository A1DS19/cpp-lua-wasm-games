# 11 — Textures

> Previous: [10 — Asset paths](./engine-10-asset-paths.md) · Next: [12 — Loading meshes from glTF](./engine-12-mesh-loading.md)

A texture is **image data living on the GPU** that a fragment shader can sample. Loading a PNG and getting it onto the GPU has more steps than you'd expect because images, GL, and your engine all have different conventions to reconcile.

This note covers:

- The `Texture` class — RAII wrapper around a `GLuint`.
- Decoding PNGs with `stb_image`.
- The single-header library "implementation TU" pattern.
- The vertical-flip convention and why it matters.

---

## The `Texture` class

```cpp
// include/engine/src/graphics/texture.hpp
namespace engine {
class Texture {
public:
    Texture(int width, int height, int numchannels, unsigned char* data);
    ~Texture();
    [[nodiscard]] GLuint get_id() const { return texture_id_; }

    static std::shared_ptr<Texture> load(const std::string& path);

private:
    void init(int w, int h, int c, unsigned char* data);
    GLuint texture_id_ = 0;
    int width_ = 0, height_ = 0, numchannels_ = 0;
};
} // namespace engine
```

The constructor takes already-decoded pixel data (width × height × channels). Most code uses the static `load()` factory, which decodes a file and constructs the object.

Why a static `load()` plus a raw constructor? Two callers:

- **Common case**: load from a file. Use `Texture::load("textures/brick.png")`.
- **Special case**: build a texture from in-memory pixel data (a procedurally generated image, a screenshot, a font atlas). Construct directly.

The class is move-only (the GL handle has unique ownership) and the destructor calls `glDeleteTextures(1, &texture_id_)`.

---

## stb_image — single-header decoding

`stb_image.h` ships as one header containing both **declarations** and (gated by a macro) **implementation**:

```c
// inside stb_image.h
unsigned char* stbi_load(const char* path, int* w, int* h, int* c, int desired);

#ifdef STB_IMAGE_IMPLEMENTATION
unsigned char* stbi_load(...) { /* … function body … */ }
#endif
```

If two `.cpp` files both `#define STB_IMAGE_IMPLEMENTATION` before including the header, they each emit the same function bodies, and the linker reports duplicate symbols.

The fix is the **single-implementation TU** pattern:

```cpp
// src/utils/stb-impl.cpp
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
```

That one file is the *only* place the macro is defined. Every other `.cpp` that uses stb_image just `#include`s the header. Same pattern applies to `cgltf.h` (see [doc 12](./engine-12-mesh-loading.md)).

---

## Loading a PNG

```cpp
std::shared_ptr<Texture> Texture::load(const std::string& path) {
    const auto resolved = utils::asset_path(path);
    if (!std::filesystem::exists(resolved)) {
        return nullptr;
    }

    stbi_set_flip_vertically_on_load(true);          // see "UV origin" below

    int w = 0, h = 0, c = 0;
    unsigned char* data = stbi_load(
        resolved.string().c_str(),
        &w, &h, &c,
        4);                                           // force RGBA
    if (data == nullptr) return nullptr;

    auto tex = std::make_shared<Texture>(w, h, 4, data);
    stbi_image_free(data);
    return tex;
}
```

Three details worth understanding:

### Forcing 4 channels

`stbi_load` lets you pass `desired_channels` to convert on the fly. We always pass `4` (RGBA) because:

- Avoids branching on file format (PNG with alpha vs JPG without).
- Lets the GL upload always use `GL_RGBA` — no per-format `glTexImage2D` switching.
- Matches what most modern shaders expect.

The cost is ~25% extra GPU memory for textures that didn't actually have alpha. For a hobby engine, that's fine.

### `stbi_image_free`

stb_image allocates with its own internal allocator. `delete[]` would be wrong; you must call `stbi_image_free`. After uploading to the GPU, the CPU copy is no longer needed.

### Vertical flip

This is the subtle one — see the dedicated section below.

---

## The GPU upload (`init`)

```cpp
void Texture::init(int w, int h, int c, unsigned char* data) {
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
```

Five GL calls per texture:

1. **`glGenTextures`** — get a fresh handle.
2. **`glBindTexture`** — make it the current 2D texture (target-based state).
3. **`glTexImage2D`** — upload the pixel data. Internal format `GL_RGBA` matches the source format `GL_RGBA` so no conversion happens.
4. **`glGenerateMipmap`** — the GPU builds the mip chain (¼ resolution, ¹⁄₁₆, …) for nice minification.
5. **`glTexParameteri`** — sampling state: wrap mode and filter mode.

| Parameter | Value | Meaning |
|---|---|---|
| `GL_TEXTURE_WRAP_S/T` | `GL_REPEAT` | UVs outside `[0,1]` tile the texture |
| `GL_TEXTURE_MIN_FILTER` | `GL_LINEAR_MIPMAP_LINEAR` | trilinear filtering when minified |
| `GL_TEXTURE_MAG_FILTER` | `GL_LINEAR` | bilinear when magnified |

Other common choices: `GL_CLAMP_TO_EDGE` for UI textures, `GL_NEAREST` for pixel art.

---

## UV origin: image vs OpenGL

A pixel image is stored top-down (row 0 = top of image). OpenGL textures use bottom-up convention (UV `v=0` = first uploaded row). If you upload a PNG as-is, sampling at UV `(0, 0)` returns the top-left pixel — but most engines expect `(0, 0)` to be the bottom-left.

`stbi_set_flip_vertically_on_load(true)` flips the rows on decode, so row 0 is the *bottom* of the image. After upload, GL UV `(0, 0)` = bottom-left of the image, `(0, 1)` = top-left. This matches what most artists draw and what manual UV authoring assumes.

```
Image file row order:   PNG file (top-down)
   row 0  ────▶  TOP    of image
   row 1  ────▶  ...
   row N  ────▶  BOTTOM of image

After stbi flip:
   row 0  ────▶  BOTTOM of image
   row N  ────▶  TOP    of image

Sampled in GL at UV (u, 0):
                  ────▶  row 0 (= BOTTOM of image, after flip)
```

This is consistent with OpenGL's clip-space Y-up convention and with how the cube's UVs are written in `test-obj.cpp`.

### glTF caveat

glTF spec stores UVs **already in image-space** (V=0 at the top, like the file). So when loading a glTF mesh, you've now got two flips:

1. The texture is flipped on load (rows reversed).
2. The glTF UV says "V=0 at top" but that means "first row of original file" = "last row in GL space".

Net effect: glTF UVs sample the wrong vertical position. The fix is to flip V at load time *for glTF UVs only*: `v = 1.0 - v`. See [doc 12](./engine-12-mesh-loading.md) for where this lives.

---

## Binding for a draw call

In the render loop:

```cpp
texture->bind(0);                           // glActiveTexture + glBindTexture for unit 0
glUniform1i(sampler_loc, 0);                // tell the shader to read from unit 0
glDrawElements(GL_TRIANGLES, ...);
```

OpenGL has 16+ texture units (`GL_TEXTURE0` through `GL_TEXTURE15` minimum). A material with multiple textures (base color + normal map + roughness) binds each to a different unit and tells the shader where each one lives.

In our `Material::bind()`, this is wrapped:

```cpp
for (auto& [name, texture] : textures_) {
    pshader_program_->set_texture(name, texture.get());
}
```

`ShaderProgram::set_texture` does the `glActiveTexture` + `glBindTexture` + `glUniform1i` triplet, picking unit numbers automatically.

---

## What could go wrong

1. **No `STB_IMAGE_IMPLEMENTATION` defined.** Linker errors like `undefined symbol: _stbi_load`. Make sure exactly one TU has the `#define`.

2. **Two TUs both defining `STB_IMAGE_IMPLEMENTATION`.** Linker errors like `duplicate symbol _stbi_load`. Only `stb-impl.cpp` should define it.

3. **Texture upside-down.** Forgot `stbi_set_flip_vertically_on_load(true)`, or set it but your UVs assume image-space (origin top-left). Pick one convention and stick with it project-wide.

4. **Garbled colors / stripe pattern.** Channel mismatch — file is RGB, you told GL it's RGBA (or vice versa). Force `desired_channels = 4` and always upload as `GL_RGBA`.

5. **Black texture.** Most common cause: the sampler uniform isn't being set. The default value of an unset `sampler2D` uniform is `0`, which means "use texture unit 0" — but if nothing was bound to unit 0, you get black or undefined output.

6. **Driver warning: "unit 0 GLD_TEXTURE_INDEX_2D is unloadable and bound to sampler type (Float)"** — same root cause as #5; the shader's sampler is reading from unit 0, but no valid texture is bound there. Either bind a texture or set the sampler's uniform to a unit that has one.

7. **Uniform name typo.** `material.set_param("brick-texture", ...)` ≠ shader's `uniform sampler2D brick_texture;`. GLSL forbids dashes; `glGetUniformLocation` returns -1 and the binding silently no-ops.

---

## See also

- [05 — Shaders & Materials](./engine-05-shaders-materials.md) — how `Material` stores textures.
- [10 — Asset paths](./engine-10-asset-paths.md) — `utils::asset_path` resolves the texture file path.
- [12 — Loading meshes from glTF](./engine-12-mesh-loading.md) — where the V-flip-on-glTF rule applies.
- [meshes-and-models.md](./meshes-and-models.md) — file format primer.
