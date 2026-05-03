#pragma once

#include <optional>
#include <utility>

#include "core/spaceinvaders.h"
#include "platform.h"
#include "renderer.h"

class App {
public:
    static std::optional<App> create();

    bool run(std::string_view romPath);

private:
    SpaceInvaders m_spaceInvaders;
    Platform      m_platform;
    Renderer      m_renderer;

    explicit App(Platform platform, Renderer renderer)
        : m_platform(std::move(platform))
        , m_renderer(std::move(renderer)) {}

    bool m_quit { false };

    void handleEvents();
};