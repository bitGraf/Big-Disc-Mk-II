@echo off
set vc_build_path="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
call %vc_build_path%\vcvarsall.bat x64
rem set vc_build_tools_path="C:\BuildTools"
rem call %vc_build_tools_path%\devcmd.bat

if "%0" == "shell" (
    start "" "C:\Program Files\PureDevSoftware\10x\10x.exe" ..\rohin.10x
)

if NOT "%0" == "shell" (
    echo "clicked on this!"
    cmd /k
)
