# 🐗🔨➡️🏰 HWIF

**H**EHEHEHA**W** **I**mage **F**ormat - A simple, suprisngly fast and storage inefficient image format, originally inspired by [BRUH](https://github.com/face-hh/bruh/).

## 🔍 How does it work?

Instead of saving pixels in a normal, boring way, HWIF uses a counter mechanism to manually count the byte value of a certain color (done 3 times; R, G, B). 

For example, if wanted to encode the color red (255, 0, 0), we would count up to 255, then 0, and 0 again:

```text
HEHE🐗
HEHE🐗
HEHE🐗
...
HAAW🔨    <- This indicates to move onto the next byte
HAAW🔨
HAAW🔨
```

This is then repeated for each pixel in the image, and is obviously **VERY** inefficient in terms of storage, but it's only meant to have some fun :)

## ⚡ Benchmarks

Note: _The png generation was done in Python, and the hwif generation was done in HWIF (C++)_

Note 2: _These tests were on my system._

### A random 32x32 image

| Format | Size | Time (s) |
| --- | --- | --- |
| PNG | 3,172 bytes| 0.0173 |
| HWIF | 3,083,583 bytes | 0.0083035  |

### A random 128x128 image

| Format | Size | Time (s) |
| --- | --- | --- |
| PNG | 49,353 bytes| 0.0188 |
| HWIF | 50,670,679 bytes | 0.141212  |

### A random 512x512 image

| Format | Size | Time (s) |
| --- | --- | --- |
| PNG | 787,598 bytes| 0.0399 |
| HWIF | 808,380,087 bytes | 2.03416  |


## 🚀 Usage

1. Clone the repo
2. Open the solution in Visual Studio
3. Build the solution (in `release` mode to make it faster)
4. Run the executable

### Commands:

```bash
hwif open <filepath>
hwif convert-from <input-file> <output-file>    <-- Converts a PNG to HWIF
hwif convert-to <input-file> <output-file>      <-- Converts a HWIF to PNG
hwif generate <output-file> <width> <height>    <-- Generates a HWIF with random pixels
```

## 🙏 Thanks

- [BRUH](https://github.com/face-hh/bruh/)
- [LodePNG](https://github.com/lvandeve/lodepng)
- [SDL](https://github.com/libsdl-org/SDL)

## 📜 License

This project is licensed under the MIT license. More info can be found in the [LICENSE](LICENSE) file.
