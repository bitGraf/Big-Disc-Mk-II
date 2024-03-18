@echo off
SetLocal EnableDelayedExpansion

rem RUN WITH: Tools/shader_tool/unity_build/shader_tool.exe Game/run_tree/Data/Shaders/ Game/src/ShaderSrc/shaders_generated.cpp
rem echo Building!

echo =====================================
echo Building the ShaderTool...

rem Build shader_tool.exe
set ShaderToolCompileFlags=/MTd /nologo /Gm- /GR- /EHsc /Odi /std:c++14 /W4 /WX /wd4189 /wd4201 /wd4100 /FC /Fo../../bin/int/ /Fe: ../../bin/shader_tool.exe
cl shader_tool.cpp %ShaderToolCompileFlags% /link /SUBSYSTEM:CONSOLE /incremental:no /opt:ref user32.lib


rem rem Compile shader tool to and run it to generate some code
rem set ShaderToolCompileFlags=/MTd /nologo /Gm- /GR- /EHsc /Odi /std:c++14 /W4 /WX /wd4189 /wd4201 /wd4100 /FC
rem cl ..\Tools\shader_tool\shader_tool.cpp %ShaderToolCompileFlags% /link /SUBSYSTEM:CONSOLE /incremental:no /opt:ref user32.lib
rem shader_tool.exe ../Game/run_tree/Data/Shaders/ ../Game/src/ShaderSrc/shaders_generated.cpp

EXIT /B %ERRORLEVEL%