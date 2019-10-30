[![Build Status](https://travis-ci.com/Anton94/modal_logic_formula_prover.svg?branch=master)](https://travis-ci.com/Anton94/modal_logic_formula_prover)
[![Build status](https://ci.appveyor.com/api/projects/status/0bjq49fxgpb66jkn?svg=true)](https://ci.appveyor.com/project/Anton94/modal-logic-formula-prover)

# modal_logic_formula_prover
TODO: Master degree project for prooving/disprooving formulas (space and probability)

# Dependencies:
- [CppRestSDK](https://github.com/microsoft/cpprestsdk) only for the web client. Note that for Unix it will require boost's System library
- CMake 3.7 or newer
- C++ compiler which supports cpp14 (you can check the used compilers in the builders at the top)

# Building
For build system, we are using CMake.

For example, if using vcpkg to install the cpprestsdk, a VS 2015 project can be generated by the following:
- repo_dir> mkdir build
- repo_dir> cd build
- repo_dir> cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg_repo>/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 14 2015 Win64"

The generated files are going to be in the `build` directory, where you can start `ModalLogicFormulaProover.sln`

# Sanitizers
We are using [google's sanitizers](https://github.com/google/sanitizers) via CMake.
Note that for memory sanitizer you will need e.g. llvm-symbolizer to see the sources & lines and not just the addresses.

# References
[CppRestSDK](https://github.com/microsoft/cpprestsdk) 

[CxxOpts](https://github.com/jarro2783/cxxopts)

[Catch2](https://github.com/catchorg/Catch2)

[Boost](https://www.boost.org/)

[Filesystem](https://github.com/gulrak/filesystem/blob/master/include/ghc/filesystem.hpp)

[Kiwi](https://github.com/nucleic/kiwi)
