@echo off
echo "graphics hook build"
cd %~dp0

set DepsPath32=%cd%\dependencies2019\win32
set DepsPath64=%cd%\dependencies2019\win64

set BUILD_DIR=build
set COPY_DIR=build
if "%1" == "x86" (
  set BUILD_DIR=build32
  set COPY_DIR=build64
) else if "%1" == "x64" (
  set BUILD_DIR=build64
  set COPY_DIR=build32
)
echo %BUILD_DIR%

if not exist %cd%\%BUILD_DIR% (
  mkdir %BUILD_DIR%
)

set SRC_HELP_DIR=plugins\win-capture\inject-helper
set BUILD_HELP_DIR=%BUILD_DIR%\plugins\win-capture\inject-helper

set SRC_OFFETS_DIR=plugins\win-capture\get-graphics-offsets
set BUILD_OFFSETS_DIR=%BUILD_DIR%\plugins\win-capture\get-graphics-offsets

set SRC_GRAPHICS_DIR=plugins\win-capture\graphics-hook
set BUILD_GRAPHICS_HOOK=%BUILD_DIR%\plugins\win-capture\graphics-hook

cd ./%BUILD_DIR%
if "%1" == "x86" (
  echo "generate x86"
  cmake -G "Visual Studio 16 2019" -A Win32 -DCMAKE_SYSTEM_VERSION=10.0 -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DENABLE_VLC=false -DCOMPILE_D3D12_HOOK=true -DBUILD_BROWSER=false -DENABLE_UI=false -DDISABLE_UI=true -DBUILD_VST=false -DENABLE_SCRIPTING=false -DCOMPILE_LUA=false -DCOMPILE_PYTHON=false "%cd%\..\src"
) else if "%1" == "x64" (
  echo "generate x64"
  cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_VERSION=10.0 -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DENABLE_VLC=false -DCOMPILE_D3D12_HOOK=true -DBUILD_BROWSER=false -DENABLE_UI=false -DDISABLE_UI=true -DBUILD_VST=false -DENABLE_SCRIPTING=false -DCOMPILE_LUA=false -DCOMPILE_PYTHON=false "%cd%\..\src"
) else (
  echo "selecte platform"
)

cd ../
cmake --build %BUILD_HELP_DIR% --config %2% -j12
cmake --build %BUILD_OFFSETS_DIR% --config %2% -j12
cmake --build %BUILD_GRAPHICS_HOOK% --config %2% -j12

:: copy file back to output path
set SRC_DIR=%cd%\%BUILD_DIR%\rundir\%2%\data\obs-plugins\win-capture
set DES_DIR=%cd%\%COPY_DIR%\rundir\%2%\data\obs-plugins\win-capture
xcopy %SRC_DIR% %DES_DIR% /y /f




