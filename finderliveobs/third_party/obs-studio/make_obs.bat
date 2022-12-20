@echo off
REM usage: make_obs.bat [x84|x64] [Debug|RelWithDebInfo]
echo "libobs and obs plugin build"
cd %~dp0

if "%1" == "x86" (
  set BUILD_DIR=build32
) else if "%1" == "x64" (
  set BUILD_DIR=build64
)

if not exist "%cd%\%BUILD_DIR%" (
  mkdir "%cd%\%BUILD_DIR%"
)

echo "obs src: %cd%\src"
echo "obs build: %cd%\%BUILD_DIR%"

set BUILD_CONFIG=%2
echo BUILD_CONFIG: %BUILD_CONFIG%

set DepsPath32=%cd%\dependencies2019\win32
set DepsPath64=%cd%\dependencies2019\win64

set SRC_HELP_DIR=%cd%\src\plugins\win-capture\inject-helper
set BUILD_HELP_DIR=%cd%\%BUILD_DIR%\plugins\win-capture\inject-helper

set SRC_OFFETS_DIR=%cd%\src\plugins\win-capture\get-graphics-offsets
set BUILD_OFFSETS_DIR=%cd%\%BUILD_DIR%\plugins\win-capture\get-graphics-offsets

set SRC_GRAPHICS_DIR=%cd%\src\plugins\win-capture\graphics-hook
set BUILD_GRAPHICS_HOOK=%cd%\%BUILD_DIR%\plugins\win-capture\graphics-hook

cd %cd%\%BUILD_DIR%

if "%1" == "x86" (
  echo "building x86"
  cmake -G "Visual Studio 16 2019" -A Win32 -DCMAKE_SYSTEM_VERSION=10.0.20348.0 -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DENABLE_VLC=false -DCOMPILE_D3D12_HOOK=true -DBUILD_BROWSER=false -DENABLE_UI=false -DDISABLE_UI=true -DBUILD_VST=false -DENABLE_SCRIPTING=false -DCOMPILE_LUA=false -DCOMPILE_PYTHON=false "%cd%\..\src"
) else if "%1" == "x64" (
  echo "building x64"
  cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_VERSION=10.0.20348.0 -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DENABLE_VLC=false -DCOMPILE_D3D12_HOOK=true -DBUILD_BROWSER=false -DENABLE_UI=false -DDISABLE_UI=true -DBUILD_VST=false -DENABLE_SCRIPTING=false -DCOMPILE_LUA=false -DCOMPILE_PYTHON=false "%cd%\..\src"
  @REM set static bin
  @REM use msvc clang to compile libyuv to use the CPU SMID  
  @REM cmake -G "Visual Studio 16 2019" -A x64 -T ClangCL -DCMAKE_SYSTEM_VERSION=10.0.20348.0 -S "%cd%\..\src\plugins\wechat_private\third_party\libyuv" -B "%cd%\..\%BUILD_DIR%\deps\libyuv"
) else (
  echo "selecte platform"
  exit
)

cd ../

echo "start build"

@REM set static bin
@REM first build private deps
@REM cmake --build "%BUILD_DIR%\deps\libyuv" --config RelWithDebInfo

@REM other
cmake --build %BUILD_DIR% --config %BUILD_CONFIG%
cmake --build %BUILD_HELP_DIR% --config %BUILD_CONFIG%
cmake --build %BUILD_OFFSETS_DIR% --config %BUILD_CONFIG%
cmake --build %BUILD_GRAPHICS_HOOK% --config %BUILD_CONFIG%