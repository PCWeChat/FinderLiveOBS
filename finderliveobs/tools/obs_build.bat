@echo off
@REM usage: obs_build.bat [x86|x64] [Debug|RelWithDebInfo] [1|0]

: 0x0. cmd line parse
set BUILD_PLATFORM=%1
echo BUILD_PLATFORM: %BUILD_PLATFORM%

if "%1" == "x86" (
  set BUILD_SUFFIX=32
  set BUILD_GRAPHICE_SUFFIX=64
  set GRAPHICE_BUILD=x64
) else if "%1" == "x64" (
  set BUILD_SUFFIX=64
  set BUILD_GRAPHICE_SUFFIX=32
  set GRAPHICE_BUILD=x86
)

echo BUILD_SUFFIX: %BUILD_SUFFIX%

set BUILD_CONFIG=%2
echo BUILD_CONFIG: %BUILD_CONFIG%

set ROOT_PATH=%~dp0
echo current path: %ROOT_PATH%

: change RelWithDebInfo/Release -> Release
if /I "%BUILD_CONFIG%"=="Debug" (
  set COPY_BUILD_CONFIG=debug
) else (
  set COPY_BUILD_CONFIG=release
)
echo copy config name: %COPY_BUILD_CONFIG%

: 0x1. clean old file and path
@REM del cmd path must use \
if not exist "%ROOT_PATH%..\..\out\lib\%COPY_BUILD_CONFIG%" (
  mkdir "%ROOT_PATH%..\..\out\lib\%COPY_BUILD_CONFIG%"
)

if exist "%ROOT_PATH%\..\..\out\lib\%COPY_BUILD_CONFIG%\obs.lib" (
  del /f "%ROOT_PATH%\..\..\out\lib\%COPY_BUILD_CONFIG%\obs.lib"
)

if exist "%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%\data" (
  rmdir "%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%\data" /s /q
)

if exist "%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%\obs-plugins" (
  rmdir "%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%\obs-plugins" /s /q
)

: 0x2. do build
call "%ROOT_PATH%\..\third_party\obs-studio\make_obs.bat" %BUILD_PLATFORM% %BUILD_CONFIG%
call "%ROOT_PATH%\..\third_party\obs-studio\make_graphics_hook.bat" %GRAPHICE_BUILD% %BUILD_CONFIG%

: 0x3. copy/mklink build result
@REM copy cmd path must use \
copy "%ROOT_PATH%\..\third_party\obs-studio\build%BUILD_SUFFIX%\libobs\%BUILD_CONFIG%\obs.lib" "%ROOT_PATH%..\..\out\lib\%COPY_BUILD_CONFIG%\obs.lib"

xcopy "%ROOT_PATH%\..\third_party\obs-studio\build%BUILD_SUFFIX%\rundir\%BUILD_CONFIG%\data" "%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%\data\" /f /e /c /y
xcopy "%ROOT_PATH%\..\third_party\obs-studio\build%BUILD_SUFFIX%\rundir\%BUILD_CONFIG%\obs-plugins\%BUILD_SUFFIX%bit" "%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%\obs-plugins\" /f /e /c /y
  
@REM copy obs dll/pdb
set OBS_BIN_SRC_DIR=%ROOT_PATH%\..\third_party\obs-studio\build%BUILD_SUFFIX%\rundir\%BUILD_CONFIG%\bin\%BUILD_SUFFIX%bit
set OBS_BIN_DEST_DIR=%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%
set files=^
avcodec-58.dll avdevice-58.dll avfilter-7.dll avformat-58.dll avutil-56.dll libcurl.dll libmbedcrypto.dll libobs-d3d11.dll libobs-d3d11.pdb ^
libobs-opengl.dll libobs-opengl.pdb libobs-winrt.dll libobs-winrt.pdb libogg-0.dll libopus-0.dll libsrt.dll libvorbis-0.dll libvorbisenc-2.dll libvorbisfile-3.dll ^
libvpx-1.dll libx264-161.dll obs.dll obs.pdb obsglad.dll obsglad.pdb swresample-3.dll swscale-5.dll w32-pthreads.dll w32-pthreads.pdb zlib.dll ^
airmediaserve.dll usbscreen.dll usbscreendriver.exe
@REM private airmediaserve.dll usbscreen.dll usbscreendriver.exe

for %%i in (%files%) do ( 
  echo "%OBS_BIN_DEST_DIR%/%%i"
  if exist %OBS_BIN_DEST_DIR%/%%i (
    del /f "%OBS_BIN_DEST_DIR%\%%i"
  )
  copy "%OBS_BIN_SRC_DIR%\%%i" "%OBS_BIN_DEST_DIR%\%%i"
)

@REM keep obs cmake vs file
if "%3" NEQ "1" (
  rmdir "%ROOT_PATH%\..\third_party\obs-studio\build%BUILD_SUFFIX%" /s /q
  rmdir "%ROOT_PATH%\..\third_party\obs-studio\build%BUILD_GRAPHICE_SUFFIX%" /s /q
  del "%ROOT_PATH%\..\..\out\bin\%COPY_BUILD_CONFIG%\data\.gitignore"
)