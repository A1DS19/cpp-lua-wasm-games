#include "utils/asset-loader.hpp"

#include "rendering/shader.hpp"
#include "rendering/texture.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_video.h>
#include <iostream>

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

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, heigth, 0, format, GL_UNSIGNED_BYTE,
                 p_formatted_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pixel_art ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixel_art ? GL_NEAREST : GL_LINEAR);

    SDL_FreeSurface(p_formatted_surface);
    SDL_FreeSurface(p_surface);

    return std::make_shared<jpengine::Texture>(texture_id, width, heigth, filename);
}
