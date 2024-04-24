@echo off
SetLocal EnableDelayedExpansion

REM Get a list of all the .cpp files.
pushd src
SET SrcFiles=
FOR /R %%f in (*.cpp *.c) do (
    SET SrcFiles=!SrcFiles! %%f
)
popd


set Name=rohin
set WarningFlags=/W4 /WX /wd4189 /wd4201 /wd4100 /wd4505
set Definitions=/D_CRT_SECURE_NO_WARNINGS /D_DEBUG /DRH_EXPORT /DRH_INTERNAL /DRH_PLATFORM_WINDOWS /DRH_COMPILE_OPENGL_4_4
set CompilerFlags=/MTd /nologo /Gm- /GR- /EHsc /Odi /std:c++14 %WarningFlags% /FC /Z7 %Definitions% /LD
set IncludeDirs=/Isrc /Ideps/math_lib/include /Ideps/stb /Ideps/imgui/imgui /Ideps/DirectX12-Headers/include
set LinkerFlags=/incremental:no /opt:ref /SUBSYSTEM:CONSOLE %CommonLinkerFlags% user32.lib Gdi32.lib Winmm.lib opengl32.lib Shlwapi.lib D3D12.lib DXGI.lib dxguid.lib d3dcompiler.lib
set ObjectFlags=/Fo../bin/int/

echo =====================================
echo Building the Engine...

cl %IncludeDirs% %CompilerFlags% %SrcFiles% /Fe: ../bin/%Name%.dll %ObjectFlags% /link %LinkerFlags%

if %ERRORLEVEL% NEQ 0 (
    echo ERRORLEVEL:%ERRORLEVEL%
) else (
    echo Succesfully built Rohin.dll!
)

EXIT /B %ERRORLEVEL%