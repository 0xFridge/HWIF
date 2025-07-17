#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <array>

struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class HWIF {
  private:
    static constexpr std::string_view SIGNATURE = "🐗🔨➡️🏰HEHEHAWIF";
    static constexpr uint32_t VERSION = 0x00000001;
    // ENCODING_PLUS and ENCODING_NEXT MUST be the same size (in bytes)
    static constexpr std::string_view ENCODING_PLUS = "HEHE🐗";
    static constexpr std::string_view ENCODING_NEXT = "HAAW🔨";
    static const std::array<std::string, 256> ENCODING_LUT; 

    std::vector<Pixel> _pixels;
    uint32_t _width;
    uint32_t _height;

  public:
    explicit HWIF(std::vector<Pixel> pixels, uint32_t width, uint32_t height)
        : _pixels(std::move(pixels))
        , _width(width)
        , _height(height) {}
    explicit HWIF(const std::string &filename, bool create = false);
    explicit HWIF() : _width(0), _height(0) {}

    [[nodiscard]] uint32_t width() const { return _width; }
    [[nodiscard]] uint32_t height() const { return _height; }
    [[nodiscard]] const std::vector<Pixel> &pixels() const { return _pixels; }
    [[nodiscard]] const Pixel &pixel(uint32_t x, uint32_t y) const { return _pixels[y * _width + x]; }

    void setWidth(uint32_t width) { _width = width; }
    void setHeight(uint32_t height) { _height = height; }
    void setPixels(const std::vector<Pixel> &pixels) { _pixels = pixels; }
    void setPixel(uint32_t x, uint32_t y, const Pixel &pixel);

    void load(const std::string &filename);
    void save(const std::string &filename) const;

  private:
    static uint8_t _decode(const std::string &encoded);
    static std::string _encode(uint8_t value);
};
