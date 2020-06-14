setlocal enableextensions
@echo off
set "MSVC_VER=14.26.28801"
set "THEVSPath=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional"
set "THEMSVC=%THEVSPath%\VC\Tools\MSVC"
set "THEROOT=%THEMSVC%\%MSVC_VER%"
set "THCL=%THEROOT%\bin\Hostx64\x64\cl.exe"
set "THINCLUDE=%THEROOT%\include"
set "THEINLUDE2=%THEVSPath%\Common7\IDE\VC\Linux\include\usr\include"
set "THEINLUDE3=%THEINLUDE2%\x86_64-linux-gnu"

set "T1=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\%MSVC_VER%\include"
set "T2=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\%MSVC_VER%\atlmfc\include"
set "T3=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\VS\include"
set "T4=C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\ucrt"
set "T5=C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\um"
set "T6=C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\shared"
set "T7=C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\winrt"
set "T8=C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\cppwinrt"
set "T9=C:\Program Files (x86)\Windows Kits\NETFXSDK\4.7.2\Include\um"

set "TINF=/I "%T1%" /I "%T2%" /I "%T3%" /I "%T4%" /I "%T5%" /I "%T6%" /I "%T7%" /I "%T8%" /I "%T9%""
@echo on

::"%THCL%" /FAcs mini.c

"%THCL%" /FAcs %TINF% ref.c
clang -cc1 ref.c -emit-llvm
endlocal