# cpp_project_template

![CMake](https://github.com/Abstract-Everything/cpp_project_template/workflows/CMake/badge.svg)

## Getting Started

### Usage
First, create a repository from this github template by using the `Use this template` button. Fill in the necessary details and clone.
After cloning the repository remove any unneeded frameworks using `git rm`:
	`$ git rm -r ./path/to/framework`

## Dependencies

### Necessary Dependencies
1. A C++ compiler that supports C++17.
See [cppreference.com](https://en.cppreference.com/w/cpp/compiler_support)
to see which features are supported by each compiler.
The following compilers should work:

    - [gcc 7+](https://gcc.gnu.org/)
    - [clang 6+](https://clang.llvm.org/)
    - [Visual Studio 2019 or higher](https://visualstudio.microsoft.com/)

2. [Conan](https://conan.io/)

3. [CMake 3.15+](https://cmake.org/)

### Optional Dependencies
#### C++ Tools
  * [Doxygen](http://doxygen.nl/)
  * [ccache](https://ccache.dev/)
  * [include-what-you-use](https://include-what-you-use.org/)
  * [Cppcheck](http://cppcheck.sourceforge.net/)
  * [clang-tidy](https://clang.llvm.org/extra/clang-tidy/)

#### Testing
- See [Catch2 tutorial](https://github.com/catchorg/Catch2/blob/master/docs/tutorial.md)
- See [Google Test](https://google.github.io/googletest/primer.html#simple-tests)
- See [libFuzzer Tutorial](https://github.com/google/fuzzing/blob/master/tutorial/libFuzzerTutorial.md)

#### GUI libraries
This project can be made to work with several optional GUI frameworks.

If desired, you should install the following optional dependencies as
directed by their documentation, linked here:

- [FLTK](https://www.fltk.org/doc-1.4/index.html)
- [GTKMM](https://www.gtkmm.org/en/documentation.html)
- [QT](https://doc.qt.io/)

The following dependencies can be downloaded automatically by CMake and Conan.
All you need to do to install them is to turn on a CMake flag during
configuration.
If you run into difficulty using them, please refer to their documentation,
linked here:

- [IMGUI](https://github.com/ocornut/imgui/tree/master/docs):
  This framework depends on SFML, and if you are using Linux, you may need
  to install several of SFML's dependencies using your package manager. See
  [the SFML build tutorial](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php)
  for specifics.

## Acknowledgments

This template is an adaption of the one from [lefticus](https://github.com/lefticus/cpp_starter_project). Refer to it for a more comprehensive template.
