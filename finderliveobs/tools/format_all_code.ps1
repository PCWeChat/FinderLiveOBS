$currentDirectory = Get-Location

function FormatDirectory {
    param (
        [string]$DirectoryPath
    )
    Set-Location $DirectoryPath
    $DirectoryPath = Get-Location
    Write-Output ('format directory: ' + $DirectoryPath + ' ...')

    &"$PSScriptRoot\clang-format.exe" --verbose -i -style=file *.c *.cc *.cpp *.h *.hpp
    Get-ChildItem -Path ./ -Directory -Recurse |
    ForEach-Object {
        Write-Output ('format directory: ' + $_.FullName + ' ...')
        Set-Location $_.FullName
        &"$PSScriptRoot\clang-format.exe" --verbose -i -style=file *.c *.cc *.cpp *.h *.hpp
    }

    Set-Location $currentDirectory.Path
}

Write-Output ($PSScriptRoot)

# 格式化 finderliveobs
FormatDirectory -DirectoryPath "$PSScriptRoot\..\flobs"

FormatDirectory -DirectoryPath "$PSScriptRoot\..\flui"
