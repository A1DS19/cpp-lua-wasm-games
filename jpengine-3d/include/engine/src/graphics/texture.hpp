#pragma once

#include <GL/glew.h>
#include <memory>
#include <string>
namespace engine {

class Texture {

public:
    Texture(int width, int height, int numchannels, unsigned char* data);
    ~Texture();
    [[nodiscard]] GLuint get_id() const { return texture_id_; }

    void init(int width, int height, int numchannels, unsigned char* data);
    static std::shared_ptr<class Texture> load(const std::string& path);

private:
    GLuint texture_id_ = 0;
    int width_ = 0;
    int height_ = 0;
    int numchannels_ = 0;
};

} // namespace engine
