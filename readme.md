## Introduction
---
finderliveobs is the finderlive wrapper process for obs core.

## Build
---
finderliveobs only support `Windows`

### Prerequisites
---
- Windows version of CMake (3.20 or higher, latest preferred)
- Visual Studio 2019
- Windows 10 SDK 10.0.20348.0
- Git for Windows

### Build steps
---
- Clone `finderliveobs`
    ```
    git clone https://github.com/PCWeChat/FinderLiveOBS.git
    ```
- Run `init_env.bat` to initialize the environment
    ```shell
    init_env.bat [x86|x64] [Debug|RelWithDebInfo] [1|0]
    ```
    - platform: x86 | x64
    - config: Debug | RelWithDebInfo
    - keep obs project: 1 | 0

- Open `finderliveobs.sln` in the root path. Compile the configuration you have just initialized.

## Running
---
`finderliveobs` cannot be started independently.

## License
---
[GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)