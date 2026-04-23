#pragma once

namespace engine {

class Application {

public:
    virtual ~Application() = default;

    virtual bool init() = 0;
    // deltatime is in seconds
    virtual void update(float deltatime = 0.0F) = 0;
    virtual void destroy() = 0;

    void set_needs_to_be_closed(bool value) noexcept { needs_to_be_closed_ = value; };
    [[nodiscard]] bool needs_to_be_closed() const noexcept { return needs_to_be_closed_; };

private:
    bool needs_to_be_closed_;
};

} // namespace engine
