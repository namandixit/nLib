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
REM BUILD
REM `````````````````````````````````````````````````````````````````````

SET Source=%ProjectRoot%\test.c
SET Target=test.exe
SET TargetPath=%BuildDirectory%\%Target%

REM LINK.EXE does some incremental linking thing-a-majig and deleting executable prevents it
REM IF EXIST %Target% del %Target%

REM SYNTAX CHECK ========================================================

SET SynCompiler=clang
SET SynCompilerFlags=-fsyntax-only
SET SynLanguageFlags=--std=c18 -DBUILD_INTERNAL -DBUILD_DEBUG -DBUILD_SLOW
SET SynWarningFlags=-Weverything -Wpedantic -pedantic-errors -Werror ^
                    -Wno-c++98-compat

WHERE %SynCompiler% >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    %SynCompiler% %SynCompilerFlags% %SynLanguageFlags% %SynWarningFlags% %Source%
)

REM COMPILING ===========================================================

SET Compiler=cl
SET CompilerFlags=/nologo /Zi /FC /Oi /I .
REM nologo - Don't print MSVC version info upon invocation
REM Zi - Create debug info
REM FC - Give full file paths in diagnostics
REM Oi - Use compiler intrinsics (built-ins) when possible
SET LanguageFlags= /std:c17 /DBUILD_INTERNAL /DBUILD_DEBUG /DBUILD_SLOW
SET WarningFlags=/W4 /WX /wd4200
REM W4  - set warning level to max (equivalent to Clang's -Weverything)
REM WX - treat warnings as errors
REM Disabled Warnigs:
REM     wd4200 - nonstandard extension used : zero-sized array in struct/union
SET LinkerFlags= /Fe%TargetPath%

SET CompilerExists=yes
WHERE %Compiler% >nul 2>&1
IF %ERRORLEVEL% NEQ 0 SET CompilerExists=no

IF %CompilerExists%==yes (
    %Compiler% %CompilerFlags% %LanguageFlags% %WarningFlags% %Source% %LinkerFlags%
)

IF %CompilerExists%==no (
    ECHO "MSVC installation not found"
)

POPD
