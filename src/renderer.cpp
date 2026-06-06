#include "renderer.h"

#include <cstring>

#include "common/log.h"

std::optional<Renderer> Renderer::create(SDL_Window* window, int framebufferWidth, int framebufferHeight) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer) {
        log::fatal("Failed to create SDL renderer. SDL_Error: {}", SDL_GetError());
        return std::nullopt;
    }

    SDL_Texture* framebuffer = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        framebufferWidth,
        framebufferHeight
    );

    if (!framebuffer) {
        log::fatal("Failed to create SDL texture. SDL_Error: {}", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        return std::nullopt;
    }

    // Present the frame buffer using integer scaling to make it responsive to window size
    // Letterboxing will be applied to the window if the framebuffer doesn't scale cleanly
    if (!SDL_SetRenderLogicalPresentation(renderer, framebufferWidth, framebufferHeight, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
        log::err("Failed to set renderer logical presentation mode. SDL_Error: {}", SDL_GetError());
    }

    // Ensure pixels scale up cleanly using nearest neighbour
    if (!SDL_SetTextureScaleMode(framebuffer, SDL_SCALEMODE_NEAREST)) {
        log::err("Failed to set framebuffer scale mode. SDL_Error: {}", SDL_GetError());
    }

    const auto rendererName = SDL_GetRendererName(renderer);
    log::info("SDL Renderer: {}", rendererName);

    return Renderer{renderer, framebuffer};
}

Renderer::~Renderer() {
    if (m_framebuffer) {
        SDL_DestroyTexture(m_framebuffer);
        m_framebuffer = nullptr;
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
}

Renderer::Renderer(Renderer&& other) noexcept
    : m_renderer(other.m_renderer), m_framebuffer(other.m_framebuffer) {
    other.m_renderer = nullptr;
    other.m_framebuffer = nullptr;
}

Renderer& Renderer::operator=(Renderer&& other) noexcept {
    if (this != &other) {
        if (m_framebuffer) SDL_DestroyTexture(m_framebuffer);
        if (m_renderer) SDL_DestroyRenderer(m_renderer);

        m_renderer = other.m_renderer;
        m_framebuffer  = other.m_framebuffer;
        other.m_renderer = nullptr;
        other.m_framebuffer  = nullptr;
    }
    return *this;
}

void Renderer::drawPixelBuffer(std::span<const u32> pixels) {
    // Copy pixel data to framebuffer
    void* texturePixels;
    int pitch;
    SDL_LockTexture(m_framebuffer, NULL, &texturePixels, &pitch);
    std::memcpy(texturePixels, pixels.data(), pixels.size_bytes());
    SDL_UnlockTexture(m_framebuffer);

    // Render framebuffer to window
    SDL_SetRenderTarget(m_renderer, NULL);
    SDL_RenderTexture(m_renderer, m_framebuffer, NULL, NULL);
    SDL_RenderPresent(m_renderer);
}
