# Windows development

Prerequisites:

- Visual Studio 2022 with the Desktop development with C++ workload and a Windows SDK.
  The MSVC standard library must support C++23 `std::views::enumerate`.
- CMake 3.29 or newer.
- Git and a bootstrapped vcpkg checkout.

From the repository root, build with VST3 support, run the tests, and package a
portable copy:

```powershell
./scripts/build-windows.ps1 -VcpkgRoot C:/dev/vcpkg -OutputDirectory C:/dev/giada-portable
```

If CMake is not on PATH, supply its executable with `-CMake`. Dependency versions
come from `vcpkg.json`. The build uses the upstream Visual Studio 2022 generator
and an isolated manifest installation under `build/vcpkg_installed`.

For subsequent builds, add `-SkipConfigure`. Use `-Configuration Debug` for a
debug build or `-Launch` to open the resulting app after its tests pass. All
builds enable the existing test suite and VST3 hosting. The portable copy includes
transitive codec DLLs from vcpkg, including `libmp3lame.dll`.

If Visual Studio's installed compiler is older, `-ToolsetDirectory` can point to
a newer complete MSVC toolset directory (containing `include`, `lib`, and
`bin/Hostx64/x64`). The helper supplies its compiler, linker, headers, and
libraries to MSBuild. This local setup uses MSVC 14.44 alongside the installed
Visual Studio 2022 build host. Dependencies built with older compatible v143
tools can be reused; see Microsoft's [binary compatibility guidance](https://learn.microsoft.com/en-us/cpp/porting/binary-compat-2015-2017).

The initial configuration downloads and compiles dependencies. Later builds reuse
them. Close the development app before rebuilding or packaging over its running
executable.

## Saving workflow investigation

Project saving currently uses `src/gui/dialogs/browser/browserSave.cpp`, built on
the custom browser in `browserBase.cpp`. Its folder path input is read-only.
The Save callback is in `src/glue/storage.cpp`; actual project serialization is
in `src/core/api/storageApi.cpp`.

A project is a `.gprj` directory containing a `.gptc` document and assets, rather
than one standalone project file. On Windows, Open Project and Save Project now
use the system folder picker. Save adds a native project-name field: choose a
destination folder and enter a name to create a `.gprj` bundle. Selecting an
existing `.gprj` folder saves that bundle after overwrite confirmation. Open
selects the `.gprj` folder itself. Other platforms retain their existing browser.

Regression tests cover names and paths containing spaces and dots, selecting an
existing bundle without nesting projects, and serializing/reopening project data.
The Windows dialogs still need interactive confirmation, including cancellation
and reopening projects with samples and synth settings intact.
