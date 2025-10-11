# Snaps Physics

Snaps is a 2D physics engine in which every object ultimately "snaps" to a grid.\
Its purpose is to enable 2D tile-based games to have smooth and reasonably realistic physics.

### Playing with sand

![Demo](https://github.com/user-attachments/assets/7a890a51-4708-44ad-84a6-1ac1431cc91b)

🚧 This project is currently a work in progress 🚧

## Features

- Collision detection and resolution
- Velocity, Acceleration and Forces
- Friction and Drag

**Planned but not ready**
- Fluid simulation
- Gas simulation
- Constraints

## Building

This project uses CMake to generate project files.

```bash
git clone https://github.com/KyrietS/Snaps.git
cmake -S Snaps -B Snaps-build
cmake --build Snaps-build
```

## Tests

Tests are written using Google Test. The library is fetched automatically by CMake during the configuration step of the tests.

### Scene tests

There are special type of tests called Scene Tests. They simulate a physical world for a number of steps and make assertions on the way. Such tests have a graphical preview with debugging interface.

<img width="502" height="502" alt="image" src="https://github.com/user-attachments/assets/6e59aee7-92f5-46ec-b7d4-66f477ae7512" />

## License

Copyright © 2023-2025 Sebastian Fojcik \
Use of this software is granted under the terms of the MIT License.

See the [LICENCE](LICENSE) for the full license text.
