@ECHO off

SETLOCAL ENABLEEXTENSIONS
IF ERRORLEVEL 1 (
    ECHO "ERROR: Unable to enable extensions"
    EXIT
)

IF NOT DEFINED BuildPlatform (
    ECHO "Build Platform not defined"
    SET BuildPlatform=windows
)

IF NOT DEFINED BuildArchitecture (
    ECHO "Build Architecture not defined"
    SET BuildPlatform=x64
)

SET ProjectRoot=%~dp0

SET BuildDirectory=%ProjectRoot%bin\%BuildPlatform%\%BuildArchitecture%
IF NOT EXIST %BuildDirectory% MKDIR %BuildDirectory%

SET /p VersionNumberOld=<version
SET /a VersionNumber=%VersionNumberOld% + 1
ECHO %VersionNumber%> ./version

REM -b makes sed treat file as binary, so that it doesn't replace \n with \r\n
SED -i -b "s/^[[:space:]]\* Version: .*$/ * Version: %VersionNumber%/" nlib.h

PUSHD %BuildDirectory%
ECHO Entering directory `%cd%'

REM .....................................................................
REM C
REM `````````````````````````````````````````````````````````````````````

SET CSource=%ProjectRoot%\test.c
SET CTarget=test.c.exe
SET CTargetPath=%BuildDirectory%\%CTarget%

REM LINK.EXE does some incremental linking thing-a-majig and deleting executable prevents it
REM IF EXIST %CTarget% del %CTarget%

REM SYNTAX CHECK ========================================================

ECHO C Syntax Check (Clang) ===========================================

SET CSynCompiler=clang
SET CSynCompilerFlags=-fsyntax-only
SET CSynLanguageFlags=--std=c18 -DBUILD_INTERNAL -DBUILD_DEBUG -DBUILD_SLOW
SET CSynWarningFlags=-Weverything -Wpedantic -pedantic-errors -Werror ^
                     -Wno-c++98-compat

WHERE %CSynCompiler% >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    %CSynCompiler% %CSynCompilerFlags% %CSynLanguageFlags% %CSynWarningFlags% %CSource%
)

REM COMPILING ===========================================================

ECHO C Compile (MSVC) =================================================

SET CCompiler=cl
SET CCompilerFlags=/nologo /Zi /FC /Oi
REM nologo - Don't print MSVC version info upon invocation
REM Zi - Create debug info
REM FC - Give full file paths in diagnostics
REM Oi - Use compiler intrinsics (built-ins) when possible
SET CLanguageFlags=/TC /std:c17 /DBUILD_INTERNAL /DBUILD_DEBUG /DBUILD_SLOW
REM TC - Treat source file as C code
SET CWarningFlags=/W4 /WX /wd4200 /wd4201
REM W4  - set warning level to max (equivalent to Clang's -Weverything)
REM WX - treat warnings as errors
REM Disabled Warnigs:
REM     wd4200 - nonstandard extension used : zero-sized array in struct/union
REM     wd4201 - nonstandard extension used : nameless struct/union
SET CLinkerFlags= /Fe%CTargetPath%

SET CCompilerExists=yes
WHERE %CCompiler% >nul 2>&1
IF %ERRORLEVEL% NEQ 0 SET CCompilerExists=no

IF %CCompilerExists%==yes (
    %CCompiler% %CCompilerFlags% %CLanguageFlags% %CWarningFlags% %CSource% %CLinkerFlags%
)

IF %CCompilerExists%==no (
    ECHO MSVC installation not found
)

REM .....................................................................
REM C++
REM `````````````````````````````````````````````````````````````````````

SET CPPSource=%ProjectRoot%\test.cpp
SET CPPTarget=test.cpp.exe
SET CPPTargetPath=%BuildDirectory%\%CPPTarget%

REM LINK.EXE does some incremental linking thing-a-majig and deleting executable prevents it
REM IF EXIST %CPPTarget% del %CPPTarget%

REM SYNTAX CHECK ========================================================

ECHO C++ Syntax Check (Clang) =========================================

SET CPPSynCompiler=clang++
SET CPPSynCompilerFlags=-fsyntax-only
SET CPPSynLanguageFlags=--std=c++14 -DBUILD_INTERNAL -DBUILD_DEBUG -DBUILD_SLOW
SET CPPSynWarningFlags=-Weverything -Wpedantic -pedantic-errors -Werror ^
                       -Wno-old-style-cast -Wno-c++98-compat-pedantic -Wno-writable-strings ^
                       -Wno-gnu-anonymous-struct

WHERE %CPPSynCompiler% >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    %CPPSynCompiler% %CPPSynCompilerFlags% %CPPSynLanguageFlags% %CPPSynWarningFlags% %CPPSource%
)

REM COMPILING ===========================================================

ECHO C++ Compile (MSVC) ===============================================

SET CPPCompiler=cl
SET CPPCompilerFlags=/nologo /Zi /FC /Oi
REM nologo - Don't print MSVC version info upon invocation
REM Zi - Create debug info
REM FC - Give full file paths in diagnostics
REM Oi - Use compiler intrinsics (built-ins) when possible
SET CPPLanguageFlags=/TP /std:c++14 /DBUILD_INTERNAL /DBUILD_DEBUG /DBUILD_SLOW
REM TP - Treat source file as C++ code
SET CPPWarningFlags=/W4 /WX /wd4201
REM W4  - set warning level to max (equivalent to Clang's -Weverything)
REM WX - treat warnings as errors
REM Disabled Warnigs:
REM     wd4201 - nonstandard extension used : nameless struct/union
SET CPPLinkerFlags= /Fe%CPPTargetPath%

SET CPPCompilerExists=yes
WHERE %CPPCompiler% >nul 2>&1
IF %ERRORLEVEL% NEQ 0 SET CPPCompilerExists=no

IF %CPPCompilerExists%==yes (
    %CPPCompiler% %CPPCompilerFlags% %CPPLanguageFlags% %CPPWarningFlags% %CPPSource% %CPPLinkerFlags%
)

IF %CPPCompilerExists%==no (
    ECHO MSVC installation not found
)

POPD
