SET MODE=Release
SET LOCATION=C:\Users\nrotem\Desktop\CVCC\

del /F runtime_tests64\OclCpuBackEnd.dll
copy %LOCATION%\build\Win64\bin\%MODE%\OclCpuBackEnd.dll runtime_tests64\OclCpuBackEnd.dll
REM @copy %LOCATION%\build\Win64\bin\%MODE%\llc.exe           runtime_tests64\llc.exe
dir runtime_tests64\OclCpuBackEnd.dll
@pause

@copy %LOCATION%\libraries\ocl_builtins\x64\%MODE%\*.rtl runtime_tests64\
@copy %LOCATION%\libraries\ocl_builtins\x64\%MODE%\*.dll runtime_tests64\
@copy %LOCATION%\libraries\ocl_builtins\x64\%MODE%\*.pdb runtime_tests64\
@copy %LOCATION%\libraries\ocl_builtins\bin\svml\x64\*.dll  runtime_tests64\
dir runtime_tests64\*.pdb
dir runtime_tests64\*.rtl
dir runtime_tests64\clb*.dll
@pause