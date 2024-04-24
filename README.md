## Retro Remake Framework

This is a cross-platform framework designed for creating retro remakes, encompassing demos, cracktros, intros, and various nostalgic artifacts from platforms like Amiga, Atari, C64, and more.

### Project Structure

The framework generates three distinct types of files:

1. **Loader (`loader` or `loader.exe`):**
   This component scans the `remakes/` subdirectory and compiles an array of `remakes_*.so/.dll` files. These files are then randomly presented by a `selector_*.so/.dll`, mimicking the experience of browsing an old-school demo-disk, where users can select a remake to view.

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

To contribute a new selector or remake:

1. Navigate to the root of the project.

2. Run either of the following scripts:
   - For a new remake:
     ```bash
     ./new_remake.sh <name>
     ```
   - For a new selector:
     ```bash
     ./new_selector.sh <name>
     ```

3. A template will be copied to the respective `remakes/` or `selectors/` subdirectory with the chosen name. Additionally, the `build.sh` script within the corresponding project will be updated to produce a shared library named `remake_<name>.so/.dll` or `selector_<name>.so/.dll`.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

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
