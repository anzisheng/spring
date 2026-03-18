@echo off
call "C:\PROGRA~1\MICROS~1\2022\PROFES~1\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%

cd /d "D:\workspace\FY25\QGLWater-Src-2026-01-16_SL40 - Clock\Src-2026-01-16\build\Desktop_Qt_6_8_3_MSVC2022_64bit-Release"
if errorlevel 1 exit /b %errorlevel%

"C:\Qt\Tools\QtCreator\bin\jom\jom.exe" -f Makefile.Release
exit /b %errorlevel%
