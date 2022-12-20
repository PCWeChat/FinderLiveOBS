@echo off
@REM usage: init_env.bat [x86|x64] [Debug|RelWithDebInfo] [1|0]

echo "platform: %1"
echo "config: %2"
echo "keep obs project: %3"

set ENV_ROOT_PATH=%~dp0
echo %ENV_ROOT_PATH%

echo "start obs build"
call %ENV_ROOT_PATH%/finderliveobs/tools/obs_build.bat %1 %2 %3

echo "start protobuf build"
call %ENV_ROOT_PATH%/finderliveobs/tools/protobuf_build.bat %1 %2