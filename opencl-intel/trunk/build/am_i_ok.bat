@echo off

set SO_FAR=YAY

::::::::::::::::: Start of tests ::::::::::::::::::::

set MOOD=YAY
if not exist "c:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\devenv.com" (
 echo Could not find VS2008
 set MOOD=BOO
)
if %SO_FAR%==YAY set SO_FAR=%MOOD%
 
set MOOD=YAY
grep --version > nul 2>&1 || set MOOD=BOO
if %MOOD%==BOO (
 echo grep not found in path. Cannot continue
 exit /b 1
)
if %SO_FAR%==YAY set SO_FAR=%MOOD%

set MOOD=YAY
egrep --version > nul 2>&1 || set MOOD=BOO
if %MOOD%==BOO (
 echo egrep not found in path
)
if %SO_FAR%==YAY set SO_FAR=%MOOD%
 
set MOOD=YAY
cmake --version | grep -q "2.8" || set MOOD=BOO
if %MOOD%==BOO (
 echo cmake 2.8 not found in path
)
if %SO_FAR%==YAY set SO_FAR=%MOOD%

set MOOD=YAY
python -c "import platform;print platform.system()" | grep -q Windows || set MOOD=BOO
if %MOOD%==BOO (
 echo ActiveState Python is not it the default path
)
if %SO_FAR%==YAY set SO_FAR=%MOOD%

set MOOD=YAY
if not exist c:\Windows\System32\msvcp100d.dll set MOOD=BOO
if %MOOD%==BOO (
 echo MSVCRT Debug Runtime dlls not installed
)
if %SO_FAR%==YAY set SO_FAR=%MOOD%

::::::::::::::::: End of tests ::::::::::::::::::::

if not %SO_FAR%==YAY exit /b 1

echo PASSED :)


