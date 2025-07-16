#include "HWIF.h"
#include <cstdint>

HWIF::HWIF(const std::string &filename, bool create) {
    if (create) {
        _width = 0;
        _height = 0;
        _pixels.clear();

        save(filename);
    } else {
        load(filename);
    }
}

void HWIF::setPixel(uint32_t x, uint32_t y, const Pixel &pixel) {
    if (_pixels.size() < y * _width + x + 1) {
        _pixels.resize(y * _width + x + 1);
    }

    _pixels[y * _width + x] = pixel;
}

void HWIF::save(const std::string &filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    file.write(SIGNATURE.data(), SIGNATURE.size());
    file.write(reinterpret_cast<const char *>(&VERSION), sizeof(VERSION));
    file.write(reinterpret_cast<const char *>(&_width), sizeof(_width));
    file.write(reinterpret_cast<const char *>(&_height), sizeof(_height));

    for (const auto &pixel : _pixels) {
        uint8_t r = pixel.r;
        uint8_t g = pixel.g;
        uint8_t b = pixel.b;

        std::string encodedR = _encode(r);
        std::string encodedG = _encode(g);
        std::string encodedB = _encode(b);

        file.write(encodedR.data(), static_cast<std::streamsize>(encodedR.size()));
        file.write(encodedG.data(), static_cast<std::streamsize>(encodedG.size()));
        file.write(encodedB.data(), static_cast<std::streamsize>(encodedB.size()));
    }
}

void HWIF::load(const std::string &filename) {
    // Open the file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    // Make sure it's a HWIF file
    std::string signature(SIGNATURE.size(), '\0');
    file.read(&signature[0], SIGNATURE.size());
    if (signature != SIGNATURE) {
        throw std::runtime_error("Not a HWIF file (invalid signature)");
    }

    // Read the version
    uint32_t version;
    file.read(reinterpret_cast<char *>(&version), sizeof(version));
    if (version != VERSION) {
        std::cout << "WARNING: Different HWIF versions (file: " << version << ", expected: " << VERSION << "). Backward/forward compatibility not guaranteed."
                  << std::endl;
    }

    // Read the width and the height
    uint32_t width;
    uint32_t height;

    file.read(reinterpret_cast<char *>(&width), sizeof(width));
    file.read(reinterpret_cast<char *>(&height), sizeof(height));

    _width = width;
    _height = height;

    // Read the strings, convert them into bytes and then pixels
    std::string byte;
    std::vector<uint8_t> bytes;
    while (!file.eof()) {
        std::string str(ENCODING_PLUS.size(), '\0');
        file.read(&str[0], ENCODING_PLUS.size());

        byte += str;

        if (str == ENCODING_NEXT) {
            uint8_t value = _decode(byte);

            bytes.push_back(value);
            byte.clear();

            // R + G + B
            if (bytes.size() == 3) {
                _pixels.push_back({bytes[0], bytes[1], bytes[2]});
                bytes.clear();
            }
        }
    }
}

uint8_t HWIF::_decode(const std::string &encoded) {
    return encoded.size() / ENCODING_PLUS.size();
}

std::string HWIF::_encode(uint8_t value) {
    std::string encoded;

    for (int i = 0; i < value; i++) {
        encoded += ENCODING_PLUS;
    }

    encoded += ENCODING_NEXT;

    return encoded;
}