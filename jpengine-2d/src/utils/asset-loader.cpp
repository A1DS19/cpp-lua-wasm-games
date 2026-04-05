#include "utils/asset-loader.hpp"

#include "rendering/font.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_video.h>
#include <SDL_mixer.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <stb/stb_truetype.h>
#include <vector>

using namespace jpengine;
using namespace jpengine::utils;

std::shared_ptr<jpengine::Shader> AssetLoader::load_shader_from_memory(const char* vertex_shader,
                                                                       const char* frag_shader) {
    const GLuint program = glCreateProgram();

    // vertex shader
    const GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader, nullptr);
    glCompileShader(vert_shader);
    GLint status;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint max_length;
        glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(static_cast<size_t>(max_length), ' ');
        glGetShaderInfoLog(vert_shader, max_length, &max_length, error_log.data());
        std::cerr << "GLSL compile failed: " << std::string{error_log} << std::endl;
        glDeleteShader(vert_shader);
        glDeleteProgram(program);
        return nullptr;
    }

    // fragment shader
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_shader, nullptr);
    glCompileShader(fragment_shader);
    GLint frag_status;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &frag_status);
    if (frag_status != GL_TRUE) {
        GLint max_length;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(static_cast<size_t>(max_length), ' ');
        glGetShaderInfoLog(fragment_shader, max_length, &max_length, error_log.data());
        std::cerr << "GLSL compile failed: " << std::string{error_log} << std::endl;
        glDeleteShader(vert_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
        return nullptr;
    }

    if (vert_shader == 0 || fragment_shader == 0) {
        return nullptr;
    }

    glAttachShader(program, vert_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint max_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(static_cast<size_t>(max_length), ' ');
        glGetProgramInfoLog(program, max_length, &max_length, error_log.data());
        std::cerr << "GLSL link failed: " << std::string{error_log} << std::endl;
        glDeleteShader(vert_shader);
        glDeleteShader(fragment_shader);
        return nullptr;
    }

    if (program == 0) {
        std::cerr << "failed to load shader from memory, program invalid\n";
        return nullptr;
    }

    return std::make_shared<jpengine::Shader>(program);
}

std::shared_ptr<jpengine::Texture> AssetLoader::load_texture(std::string_view filename,
                                                             bool pixel_art) {
    GLuint texture_id{0};
    SDL_Surface* p_surface = IMG_Load(filename.data());
    if (!p_surface) {
        std::cerr << "failed to create surface from texture file: " << filename << "\n";
        return nullptr;
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    GLenum format{GL_RGBA};
    SDL_Surface* p_formatted_surface = nullptr;
    if (p_surface->format->BytesPerPixel == 3) {
        p_formatted_surface = SDL_ConvertSurfaceFormat(p_surface, SDL_PIXELFORMAT_RGB24, 0);
        format = GL_RGB;
    } else {
        p_formatted_surface = SDL_ConvertSurfaceFormat(p_surface, SDL_PIXELFORMAT_RGBA32, 0);
        format = GL_RGBA;
    }

    int width = p_formatted_surface->w;
    int heigth = p_formatted_surface->h;

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, heigth, 0, format,
                 GL_UNSIGNED_BYTE, p_formatted_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pixel_art ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixel_art ? GL_NEAREST : GL_LINEAR);

    SDL_FreeSurface(p_formatted_surface);
    SDL_FreeSurface(p_surface);

    return std::make_shared<jpengine::Texture>(texture_id, width, heigth, filename);
}

std::shared_ptr<jpengine::Font> AssetLoader::load_font(std::string_view filename, float font_size) {
    int width = 1024;
    int heigth = 1024;

    std::ifstream font_stream{filename.data(), std::ios::binary};
    if (font_stream.fail()) {
        std::cerr << "failed to load font from path: " << filename << "\n";
        return nullptr;
    }

    font_stream.seekg(0, font_stream.end);
    auto length = static_cast<std::size_t>(font_stream.tellg());

    font_stream.seekg(0, font_stream.beg);
    std::vector<unsigned char> buffer;
    buffer.resize(length);

    std::vector<unsigned char> bitmap;
    bitmap.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(heigth));

    font_stream.read((char*)(&buffer[0]), static_cast<std::streamsize>(length));

    auto data = std::make_unique<stbtt_bakedchar[]>(96);
    int result = stbtt_BakeFontBitmap(buffer.data(), 0, font_size, bitmap.data(), width, heigth, 32,
                                      96, data.get());

    if (result <= 0) {
        std::cerr << "Font baking failed for: " << filename << "\n";
        return nullptr;
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, heigth, 0, GL_ALPHA, GL_UNSIGNED_BYTE,
                 bitmap.data());

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return std::make_shared<jpengine::Font>(id, width, heigth, font_size, (void*)data.release());
}

Mix_Music* AssetLoader::load_music(std::string_view filename) {
    Mix_Music* pmusic = Mix_LoadMUS(filename.data());

    if (!pmusic) {
        std::cerr << "failed to load: " << filename << "\nerror: " << Mix_GetError() << "\n";
        return nullptr;
    }

    return pmusic;
}

Mix_Chunk* AssetLoader::load_soundfx(std::string_view filename) {
    Mix_Chunk* pchunk = Mix_LoadWAV(filename.data());

    if (!pchunk) {
        std::cerr << "failed to load: " << filename << "\nerror: " << Mix_GetError() << "\n";
        return nullptr;
    }

    return pchunk;
}
