#pragma once

#include <optional>
#include <span>

#include <SDL3/SDL.h>

#include "common/types.h"

class Renderer {
public:
    static std::optional<Renderer> create(SDL_Window* window, int framebufferWidth, int framebufferHeight);

    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(Renderer&& other) noexcept;
    Renderer& operator=(Renderer&& other) noexcept;

    void drawPixelBuffer(std::span<const u32> pixels);

private:
    explicit Renderer(SDL_Renderer* renderer, SDL_Texture* framebuffer)
        : m_renderer(renderer), m_framebuffer(framebuffer) {}

    SDL_Renderer* m_renderer    {nullptr};
    SDL_Texture*  m_framebuffer {nullptr};
};
