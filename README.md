[![Build Status](https://travis-ci.com/Anton94/modal_logic_formula_prover.svg?branch=master)](https://travis-ci.com/Anton94/modal_logic_formula_prover)
[![Build status](https://ci.appveyor.com/api/projects/status/0bjq49fxgpb66jkn?svg=true)](https://ci.appveyor.com/project/Anton94/modal-logic-formula-prover)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/e0c037d7e81d4c9fadef114d0a0bb534)](https://www.codacy.com/manual/Anton94/modal_logic_formula_prover?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Anton94/modal_logic_formula_prover&amp;utm_campaign=Badge_Grade)
![language](https://img.shields.io/badge/language-c++-blue.svg) ![c++](https://img.shields.io/badge/std-c++14-blue.svg)
![msvc2014+](https://img.shields.io/badge/MSVC-2014+-ff69b4.svg) ![mingw-5.3+](https://img.shields.io/badge/MINGW-5.3+-ff69b4.svg) 
![gcc-5.0+](https://img.shields.io/badge/GCC-5.0+-ff69b4.svg) ![clang-4+](https://img.shields.io/badge/CLANG-4+-ff69b4.svg)

[![](https://codescene.io/projects/5855/status.svg)](https://codescene.io/projects/5855/jobs/latest-successful/results)

# modal_logic_formula_prover
TODO: Master degree project for prooving/disprooving formulas (space and probability)

# Dependencies:
- CMake 3.7 or newer
- (optional) [CppRestSDK](https://github.com/microsoft/cpprestsdk) only for the web client. Note that for Unix it will require boost's System library. If CppRestSDK is not isntalled, then the rest server project will be skipped.

Probably earlier compiler versions are also supported. We do not have any platfrom dependent code.

# Building
For build system, we are using CMake.

For example, if using vcpkg to install the cpprestsdk, a VS 2015 project can be generated by the following:
- repo_dir> mkdir build
- repo_dir> cd build
- repo_dir/build> cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg_repo>/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 14 2015 Win64"

The generated files are going to be in the `build` directory, where you can start `ModalLogicFormulaProover.sln`

# Sanitizers
We are using [google's sanitizers](https://github.com/google/sanitizers) via CMake.
Note that for memory sanitizer you will need e.g. llvm-symbolizer to see the sources & lines and not just the addresses.

# Building WebAssembly app for developers

This section is experimental and in progress.

## Linux:

Install emscripten - https://emscripten.org/docs/getting_started/downloads.html#installation-instructions

Don't forget to run > source ./emsdk_env.sh after activating the SDK.

- repo_dir> mkdir build_wasm
- repo_dir> cd build_wasm
- repo_dir/build_wasm> emcmake cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_TOOLCHAIN_FILE=<path_to_your_emsdk_repo>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DEMSCRIPTEN_INCLUDE_DIR=<path_to_your_emsdk_repo>/upstream/emscripten/cache/sysroot/ ..
- repo_dir/build_wasm> cmake --build .

Note that EMSCRIPTEN_INCLUDE_DIR points to sysroot/ directory.

Go to repo_dir/build_wasm/formula_proover. There are the html file and JS.
Note that running directly the html page in the browser will not work.
As mentioned here - https://developer.mozilla.org/en-US/docs/WebAssembly/C_to_wasm#running_your_example
You need a local http server to run it - https://developer.mozilla.org/en-US/docs/Learn/Common_questions/Tools_and_setup/set_up_a_local_testing_server

If you are using python 3
- repo_dir/build_wasm/formula_proover> python3 -m http.server

Click on the formula_proover.html

The formula_proover's main function should be called with some sample formula and you should see the output.

## Windows:
Not tested - better use WSL (Windows Subsystem for Linux)

More info regarding the build as WebAssembly
https://marcoselvatici.github.io/WASM_tutorial/#WASM_workflow
https://github.com/emscripten-core/emscripten/blob/main/cmake/Modules/Platform/Emscripten.cmake
https://github.com/floooh/pacman.c
https://groups.google.com/g/emscripten-discuss/c/CMrgauxwGfE


# References
[CppRestSDK](https://github.com/microsoft/cpprestsdk) 

[CxxOpts](https://github.com/jarro2783/cxxopts)

[Catch2](https://github.com/catchorg/Catch2)

[Boost](https://www.boost.org/)

[Filesystem](https://github.com/gulrak/filesystem/blob/master/include/ghc/filesystem.hpp)

[Kiwi](https://github.com/nucleic/kiwi)
