REM BUILD.bat script to compile msvc project
REM written 5/17/2021 JH Strom
REM ========================================

break>build.log

REM call msvc x64 .obj compiler
SET CL=/Zp2 /Ox /EHsc /std:c++17 /MD /Verbose /DNOMINMAX /D_WIN32 /DRELEASE /DWINDOWS_IGNORE_PACKING_MISMATCH /I"C:\vcpkg\installed\x64-windows\include" /I"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\shared" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\um" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\ucrt"

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.25.28610\bin\Hostx64\x64\cl.exe" /c CLBVHnode.cpp CLEngineBase.cpp CLmathlib.cpp CLOBJloader.cpp CLRaytracer.cpp CLutils.cpp main.cpp >> build.log 2>&1

REM call msvc x64 linker
set LINK=/OUT:"OCLRTX.exe" /NXCOMPAT /DYNAMICBASE "glfw3dll.lib" "glew32.lib" "OpenCL.lib" "Cfgmgr32.lib" "OpenGL32.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /LTCG:STATUS /MACHINE:X64 /INCREMENTAL /SUBSYSTEM:WINDOWS /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ERRORREPORT:PROMPT /NOLOGO /VERBOSE /LIBPATH:"C:\vcpkg\installed\x64-windows\lib" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64" /LIBPATH:"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.25.28610\lib\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt_enclave\x64" /TLBID:1 

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.25.28610\bin\Hostx64\x64\link.exe" CLBVHnode.obj CLEngineBase.obj CLmathlib.obj CLOBJloader.obj CLRaytracer.obj CLutils.obj main.obj >> build.log 2>&1

type build.log
PAUSE