#pragma once

#include <array>
#include <span>

#include <common/types.h>

class MMU {
public:
    virtual ~MMU() = default;
    virtual void loadRom() = 0;
    virtual u8 read(u16 address) const = 0;
    virtual void write(u16 address, u8 value) = 0;
    virtual u8& getRef(u16 address) = 0;
    virtual std::span<const u8> getFrameBuffer() const = 0;
};

class RealMMU : public MMU {
public:
    void loadRom() override;
    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;
    u8& getRef(u16 address) override;
    std::span<const u8> getFrameBuffer() const override;
private:
    std::array<u8, 1024 * 8> rom {};
    std::array<u8, 1024> ram {};
    std::array<u8, 1024 * 7> vram {};
};

class TestMMU : public MMU {
public:
    void loadRom() override;
    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;
    u8& getRef(u16 address) override;
    std::span<const u8> getFrameBuffer() const override;
private:
    std::array<u8, 1024 * 32> memory {};
};