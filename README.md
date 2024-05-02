## Retro Remake Framework

This is a cross-platform framework to create retro remakes which is remakes of demos, cracktros, intros, and more from platforms like Amiga, Atari, C64 and so on.

### Project Structure

The framework generates three distinct types of files:

1. **Loader (`loader` or `loader.exe`):**
   This component scans the `remakes/` subdirectory and compiles an array of `remakes_*.so/.dll` files. These files are then presented with a randomly selected `selector_*.so/.dll` in a way similar to an old-school demo-disk, where users can select a remake to watch.

### Getting Started

#### Prerequisites

To utilize this project, you will need either a Linux environment or Windows Subsystem for Linux (WSL) 1 or 2. Additionally, ensure you have GCC installed, along with the Mingw cross-compilation toolchain.

#### Installation

1. Clone the repository:
   ```bash
   git clone <repository>

2. Initialize submodules:
   - For GLFW v3.4 support:
     ```bash
     git submodule update --init /loader/glfw
     ```
   - For both GLFW v3.4 and example selector/remake:
     ```bash
     git submodule update --init --recursive
     ```

3. Run the bootstrap script:
   ```bash
   ./bootstrap.sh

4. Build the project:
   ```bash
   ./build_all.sh


### Adding Remakes and Selectors

To start writing a new selector or remake:

1. Navigate to the root of the framework.

2. Run either of the following scripts:
   - For a new remake:
     ```bash
     ./new_remake.sh <name>
     ```
   - For a new selector:
     ```bash
     ./new_selector.sh <name>
     ```
A template will be copied to the respective `remakes/` or `selectors/` subdirectory with the chosen name. Additionally, the `build.sh` script within the corresponding project will be updated to produce a shared library named `remake_<name>.so/.dll` or `selector_<name>.so/.dll`.

## License

Retro Remake Framework is licensed under the MIT License. This means that you are free to use, modify, and distribute the framework for any purpose, including commercial projects, without needing to release your source code. There are no requirements to make your modifications or derived works open-source, and you are not obligated to share your changes with the community unless you choose to do so voluntarily.

The MIT License provides maximum freedom and flexibility, allowing you to use Retro Remake Framework in ways that best suit your needs, without worrying about restrictive licensing terms. Whether you're building personal projects, commercial applications, or contributing to open-source initiatives, Retro Remake Framework is designed to empower developers and encourage collaboration without imposing unnecessary restrictions.

For more details, please refer to the [LICENSE](LICENSE) file included with the framework.

### MIT License

MIT License

Copyright (c) 2024 Peter Fors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
