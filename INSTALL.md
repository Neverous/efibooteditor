# Install EFIBootEditor

## Build from source

### Dependencies

Necessary tools:

- [CMake](//cmake.org) (>= 3.16)
- recent C/C++ compiler with C++17 support, recommended [GCC](//gcc.gnu.org/) (>= 7.5.0) or [Clang](//clang.llvm.org/) (>= 9.0.0), or [MSVC](//learn.microsoft.com/en-us/cpp/) (>= 19.29.30146.0) on Windows

Required development libraries:

- [Qt5](//doc.qt.io/qt-5/gettingstarted.html) (>= 5.15) or [Qt6](//doc.qt.io/qt-6/get-and-install-qt.html) (>= 6.2)
- [efivar](//github.com/rhboot/efivar) (>= 37) on Linux

### Build steps

1. Configure:

```
cmake -B build . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      [-Dparameter=value ...]

-- The C compiler identification is GNU 12.2.0
-- The CXX compiler identification is GNU 12.2.0
...
-- Build files have been written to: /efibooteditor/build
```

Available parameters:

- `CMAKE_BUILD_TYPE=Debug,Release,RelWithDebInfo,MinSizeRel` - specifies the build type, can be used to overwrite custom/default C/C++ compiler flags with recommended values
- `QT_VERSION_MAJOR=5,6` - force Qt5 or Qt6 build, useful if both are installed

2. Build
```
cmake --build build --config Release

[  5%] Automatic MOC and UIC for target efibooteditor
...
[100%] Built target efibooteditor
```

3. Install
```
cmake --install build

-- Install configuration: ""
-- Installing: /usr/bin/efibooteditor
...
```

## Pre-built packages

[Releases](//github.com/Neverous/efibooteditor/releases) automatically build a set of packages - they're mostly considered for testing purposes / making sure that the code compiles correctly on various environments, but they should also work just fine for normal usage. Just keep in mind they might have some specific requirements inherited from the build environment.

Packages follow a specific naming pattern: EFIBootEditor-[{VERSION}](#version)-[{OS}](#os)-[{QT_VERSION}](#qt_version)-[{COMPILER}](#compiler).[{EXTENSION}](#extension).

#### VERSION

Release version.

#### OS

Operating system used during build and generally which was targeted for the runtime. The package might work on other systems with similar versions of system libraries.

#### QT_VERSION

Targeted Qt version, generally required to have compatible Qt version installed, though some packages include all the necessary libraries in the bundles.

#### COMPILER

Compiler used during compilation, generally shouldn't matter but there might be some bugs caught in one but not the other.

#### EXTENSION

Assets are delivered in various formats:

- `.dmg` - macOS App Bundle.
- `.deb` - Debian package - should also work on any Debian derivative as long as dependencies are met.
- `.ddeb` - Debian debug symbol package - primarily useful during troubleshooting.
- `.msi` - Windows installer.
- `.zip`, `.tar.zst` - simple archive files, should contain all necessary files, ready to use in-place after decompression (`.zip` is for Windows and `.tar.zst` is for macOS and Linux).

# Notes

- Using `.deb` packages on old Ubuntu (< 21.10) or Debian (< bullseye) releases might require manual Qt installation as versions in the official repositories are older than the minimum requirements. In the CI [aqtinstall](//github.com/miurahr/aqtinstall) is used for installation, but then package install needs to be probably forced. Quick search through the internet also reveals PPAs with pre-built packages from [Stephan Binner](//launchpad.net/~beineri) that might be useful.
- There is also a [package](//aur.archlinux.org/packages/efibooteditor) in the AUR for Arch Linux.
