:: A wicked little tool for stealing tests
:: TODO: Make it work recursively

@echo off
pushd .

set nodosfilewarning=1

set SOURCE_TREE_ROOT_DIR=..\..\llvm\CodeGen\X86

set DEST_TREE_ROOT_DIR=..\AVX

:: Traverse source directory tree
echo %SOURCE_TREE_ROOT_DIR%
for %%f in (%SOURCE_TREE_ROOT_DIR%\*.ll) do (
  call :gentest %%f %DEST_TREE_ROOT_DIR%
)

popd
goto :fin

:: arg1=source filename
:gentest
set LOCAL_FILENAME=%~nx1
set SRC_FILENAME=%1
set DST_FILENAME=%2\%LOCAL_FILENAME%

echo "; RUN: llc -mcpu=sandybridge < %%p/%SRC_FILENAME%" > %DST_FILENAME%

::Remove the "
sed -i 's/"//g' %DST_FILENAME%

::Replace \ with /
call gvim -c "%%s/\\/\//g|wq" %DST_FILENAME%
::sed -i 's/\\/\///g' %DST_FILENAME%

goto :fin


:fin
