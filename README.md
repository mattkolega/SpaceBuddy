# SpaceBuddy

An emulator of the original Space Invaders arcade game.

## Building

This project requires:

- A C++ compiler with support for **C++23**. So far only tested with Clang
- A version of CMake as outlined in the root [CMakeLists.txt](CMakeLists.txt)
- Ninja

First, clone this repo and navigate into the directory. Then run the following:

```sh
cmake --preset {preset}
cmake --build build/{preset}
```

Available presets can be found in the project's [CMakePresets.json](CMakePresets.json)

## References

These resources greatly helped with the development of the emulator core.

- [Intel 8080 Assembly Programming Manual](https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf)
- [Intel 8080 Data Sheet](https://altairclone.com/downloads/manuals/8080%20Data%20Sheet.pdf)
- [Intel 8080 Opcode Table](https://www.pastraiser.com/cpu/i8080/i8080_opcodes.html)
- [Space Invaders Hardware Guide](https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html)

## Disclaimer

"Space Invaders" is a trademark of TAITO CORPORATION. This project is an
independent, non-commercial work created for educational and preservation
purposes. It is not affiliated with, endorsed by, or sponsored by TAITO
CORPORATION or any of its subsidiaries.

## License

[MIT](LICENSE)
