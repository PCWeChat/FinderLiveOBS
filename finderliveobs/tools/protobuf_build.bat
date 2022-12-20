@echo off
REM usage: protobuf_build.bat [x84|x64] [Debug|RelWithDebInfo]
echo "protobuf build"
cd %~dp0

if "%1" == "x86" (
  set BUILD_DIR=build32
) else if "%1" == "x64" (
  set BUILD_DIR=build64
)
if not exist %cd%\..\third_party\protobuf\%BUILD_DIR% (
  mkdir %cd%\..\third_party\protobuf\%BUILD_DIR%
)

echo BUILD_DIR: %cd%\..\third_party\protobuf\%BUILD_DIR%

set BUILD_CONFIG=%2
echo BUILD_CONFIG: %BUILD_CONFIG%

cd %cd%\..\third_party\protobuf\%BUILD_DIR%

echo "protobuf src: %cd%"

if "%1" == "x86" (
  echo "building x86"
  cmake -G "Visual Studio 16 2019" -A Win32 -DCMAKE_SYSTEM_VERSION=10.0.20348.0 -Dprotobuf_MSVC_STATIC_RUNTIME=true -Dprotobuf_INSTALL=false -Dprotobuf_WITH_ZLIB=false -Dprotobuf_BUILD_SHARED_LIBS=false -Dprotobuf_WITH_ZLIB_DEFAULT=false -Dprotobuf_BUILD_TESTS=false %cd%\..\cmake
) else if "%1" == "x64" (
  echo "building x64"
  cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_VERSION=10.0.20348.0 -Dprotobuf_MSVC_STATIC_RUNTIME=true -Dprotobuf_INSTALL=false -Dprotobuf_WITH_ZLIB=false -Dprotobuf_BUILD_SHARED_LIBS=false -Dprotobuf_WITH_ZLIB_DEFAULT=false -Dprotobuf_BUILD_TESTS=false %cd%\..\cmake
) else (
  echo "selecte platform"
  exit
)

cmake --build %cd% --config %BUILD_CONFIG%

if "%2" == "Debug" (
  set COPY_BUILD_CONFIG=debug
  set COPY_BUILD_NAME=libprotobuf-lited
) else if "%2" == "RelWithDebInfo" (
  set COPY_BUILD_CONFIG=release
  set COPY_BUILD_NAME=libprotobuf-lite
)

copy "%cd%\%BUILD_CONFIG%\%COPY_BUILD_NAME%.lib" "%cd%\..\..\..\..\out\lib\%COPY_BUILD_CONFIG%\%COPY_BUILD_NAME%.lib"
copy "%cd%\%BUILD_CONFIG%\%COPY_BUILD_NAME%.pdb" "%cd%\..\..\..\..\out\lib\%COPY_BUILD_CONFIG%\%COPY_BUILD_NAME%.pdb"

cd %cd%\..\..\..\tools