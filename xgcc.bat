@echo off
rem batch file to set environment variables for GCC compiler.
rem Copyright ©1999 by Embedded Support Tools Corporation, All Rights Reserved.

rem CHANGES
rem 02-13-2001 gpc -	Define environment variable ESTDIR

rem This file assumes that the root directory for the compiler is set up in
rem an environment variable (XGCC) in your autoexec.bat file.
rem This is done at install time by the XGCC installation program.
rem Change the path below if you install the tools in a different directory.
path %xgcc%\bin;%PATH%

rem Define an environment variable ESTDIR to point to the ESTII directory.
rem Try drives d: and c:
set ESTDIR=
if EXIST d:\ESTII\NUL set ESTDIR=d:/ESTII/
if EXIST c:\ESTII\NUL set ESTDIR=c:/ESTII/

if "%ESTDIR%"=="" echo ***ERROR: CANNOT FIND DIRECTORY ESTII ON c: or d:

rem These environment variables are not needed for this version of the tools,
rem but they will be used if defined. This can cause lots of problems,
rem so we make sure they are blank.
set TMPDIR=
set GCC_EXEC_PREFIX=
set COMPILER_PATH=
set LIBRARY_PATH=
set C_INCLUDE_PATH=
set CPLUS_INCLUDE_PATH=
set OBJC_INCLUDE_PATH=
set DEPENDANCIES_OUTPUT=

rem End of xgcc.bat.
