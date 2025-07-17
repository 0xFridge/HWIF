#include "HWIF.h"
#include <chrono>
#include <iostream>
#include <lodepng.h>
#include <random>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>
#include <windows.h>

constexpr unsigned int SECONDS_MS = 1000;
constexpr unsigned int FPS = 30;
constexpr std::string_view WINDOW_TITLE = "HW Image Format Demo";

void showUsage();
void showError(const std::string &message);

struct CommandArgs {
    std::string command;
    std::string filepathIn;
    std::string filepathOut;
    uint32_t width = 0;
    uint32_t height = 0;
};

CommandArgs parseArguments(int argc, char *argv[]) {
    if (argc < 2) {
        showUsage();
        throw std::invalid_argument("No command provided");
    }

    CommandArgs args;
    args.command = argv[1];

    try {
        if (args.command == "open") {
            if (argc != 3) {
                throw std::invalid_argument("Expected: HWIF open <filepath>");
            }
            args.filepathIn = argv[2];
        } else if (args.command == "convert-from" || args.command == "convert-to") {
            if (argc != 4) {
                throw std::invalid_argument("Expected: HWIF " + args.command + " <input-file> <output-file>");
            }
            args.filepathIn = argv[2];
            args.filepathOut = argv[3];
        } else if (args.command == "generate") {
            if (argc != 5) {
                throw std::invalid_argument("Expected: HWIF generate <output-file> <width> <height>");
            }
            args.filepathOut = argv[2];
            args.width = std::stoul(argv[3]);
            args.height = std::stoul(argv[4]);

            if (args.width == 0 || args.height == 0) {
                throw std::invalid_argument("Width and height must be greater than 0");
            }
        } else if (args.command == "bench") {
            // No args
        } else {
            throw std::invalid_argument("Unknown command: " + args.command);
        }
    } catch (const std::exception &e) {
        showError(e.what());
        throw;
    }

    return args;
}

void showUsage() {
    std::string usage =
        "Usage:\n"
        "  HWIF open <filepath>\n"
        "  HWIF convert-from <input-file> <output-file>\n"
        "  HWIF convert-to <input-file> <output-file>\n"
        "  HWIF generate <output-file> <width> <height>\n"
        "  HWIF bench";

    MessageBoxA(nullptr, usage.c_str(), WINDOW_TITLE.data(), MB_ICONINFORMATION);
}

void showError(const std::string &message) {
    std::string fullMessage = "Error: " + message + "\n\n";
    MessageBoxA(nullptr, fullMessage.c_str(), WINDOW_TITLE.data(), MB_ICONERROR);
}

std::vector<Pixel> generateRandomPixels(uint32_t width, uint32_t height) {
    std::vector<Pixel> pixels;
    pixels.resize(width * height);

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<int> dis(0, 255);
            pixels[y * width + x] = {static_cast<uint8_t>(dis(gen)), static_cast<uint8_t>(dis(gen)), static_cast<uint8_t>(dis(gen))};
        }
    }

    return pixels;
}

int main(int argc, char *argv[]) {
    CommandArgs args;

    try {
        args = parseArguments(argc, argv);
    } catch (const std::exception &) {
        return EXIT_FAILURE;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "[SDL_Init] ERROR: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    HWIF hwif;

    auto start = std::chrono::high_resolution_clock::now();

    try {
        if (args.command == "open") {
            hwif = HWIF(args.filepathIn);
        } else if (args.command == "convert-from") {
            // Converts a PNG to a HWIF
            std::vector<unsigned char> pixels;
            unsigned int width, height;

            unsigned int error = lodepng::decode(pixels, width, height, args.filepathIn, LodePNGColorType::LCT_RGB);
            if (error) {
                showError(lodepng_error_text(error));
                return EXIT_FAILURE;
            }

            // Convert the pixels to HWIF-compatible pixels
            std::vector<Pixel> hwifPixels;
            for (int i = 0; i < pixels.size(); i += 3) {
                hwifPixels.push_back({pixels[i], pixels[i + 1], pixels[i + 2]});
            }

            hwif = HWIF(hwifPixels, width, height);

            hwif.save(args.filepathOut);
        } else if (args.command == "convert-to") {
            // Converts a HWIF to a PNG
            std::vector<unsigned char> pixels;
            unsigned int width, height;

            hwif = HWIF(args.filepathIn);

            width = hwif.width();
            height = hwif.height();

            for (auto pixel : hwif.pixels()) {
                pixels.push_back(pixel.r);
                pixels.push_back(pixel.g);
                pixels.push_back(pixel.b);
            }

            unsigned int error = lodepng::encode(args.filepathOut, pixels, width, height, LodePNGColorType::LCT_RGB);
            if (error) {
                showError(lodepng_error_text(error));
                return EXIT_FAILURE;
            }

        } else if (args.command == "generate") {
            // Generates a random HWIF
            hwif = HWIF(generateRandomPixels(args.width, args.height), args.width, args.height);

            hwif.save(args.filepathOut);
        } else if (args.command == "bench") {
            // 32x32 benchmark
            auto startBench32 = std::chrono::high_resolution_clock::now();
            hwif = HWIF(generateRandomPixels(32, 32), 32, 32);
            auto endBench32 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diffBench32 = endBench32 - startBench32;
            std::cout << "32x32 benchmark: " << diffBench32.count() << " seconds." << std::endl;

            hwif.save("bench_32x32.hwif");

            // 128x128 benchmark
            auto startBench128 = std::chrono::high_resolution_clock::now();
            hwif = HWIF(generateRandomPixels(128, 128), 128, 128);
            auto endBench128 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diffBench128 = endBench128 - startBench128;
            std::cout << "128x128 benchmark: " << diffBench128.count() << " seconds." << std::endl;

            hwif.save("bench_128x128.hwif");

            // 512x512 benchmark
            auto startBench512 = std::chrono::high_resolution_clock::now();
            hwif = HWIF(generateRandomPixels(512, 512), 512, 512);
            auto endBench512 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diffBench512 = endBench512 - startBench512;
            std::cout << "512x512 benchmark: " << diffBench512.count() << " seconds." << std::endl;

            hwif.save("bench_512x512.hwif");
        }

    } catch (const std::exception &e) {
        showError(e.what());
        return EXIT_FAILURE;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Done in " << diff.count() << " seconds." << std::endl;

    SDL_Window *window = SDL_CreateWindow(
        args.filepathIn.empty() ? args.filepathOut.data() : args.filepathIn.data(),
        static_cast<int>(hwif.width()), static_cast<int>(hwif.height()),
        SDL_WINDOW_RESIZABLE);

    if (!window) {
        std::cerr << "[SDL_CreateWindow]" << "ERROR: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "[SDL_CreateRenderer]" << "ERROR: " << SDL_GetError() << std::endl;

        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        for (uint32_t y = 0; y < hwif.height(); y++) {
            for (uint32_t x = 0; x < hwif.width(); x++) {
                const auto &pixel = hwif.pixel(x, y);
                SDL_SetRenderDrawColor(renderer, pixel.r, pixel.g, pixel.b, 255); // Max alpha
                SDL_RenderPoint(renderer, static_cast<float>(x), static_cast<float>(y));
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(SECONDS_MS / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
