# Windows Build

## Visual Studio solution
1. Open `FantasySportsApplication.sln` in Visual Studio 2022 (toolset v143; retarget if you use 2019).
2. Build the `FantasySportsApplication` project (Debug or Release).
3. Run with F5 / Ctrl+F5. The debugger working directory is set to the repository root, so the default
   `data/data.csv` is found.

If you run the built `.exe` directly, start it from the repository root or pass the data file path:
`FantasySportsApplication.exe data\data.csv`.

## CMake (Visual Studio "Open Folder", or a Developer Command Prompt)
```bat
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
build\Release\ppp.exe data\data.csv
```

## Live data
Downloading from nflverse uses `curl.exe`, which is included with Windows 10 (version 1803) and later.
Run `curl --version` in a terminal to check.
