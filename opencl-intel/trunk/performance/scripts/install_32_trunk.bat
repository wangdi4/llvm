SET MODE=Release
SET LOCATION=C:\Users\nrotem\Desktop\CVCC\

@del /F runtime_tests32\OclCpuBackEnd.dll
@copy  %LOCATION%\build\Win32\bin\%MODE%\OclCpuBackEnd.dll runtime_tests32\OclCpuBackEnd.dll
@copy  %LOCATION%\build\Win32\bin\%MODE%\llc.exe           runtime_tests32\llc.exe
@dir   runtime_tests32\OclCpuBackEnd.dll
@pause

@copy  %LOCATION%\libraries\ocl_builtins\Win32\%MODE%\*.rtl runtime_tests32\
@copy  %LOCATION%\libraries\ocl_builtins\Win32\%MODE%\*.dll runtime_tests32\
@copy  %LOCATION%\libraries\ocl_builtins\Win32\%MODE%\*.pdb runtime_tests32\
@copy  %LOCATION%\libraries\ocl_builtins\bin\svml\Win32\*.dll  runtime_tests32\
@dir runtime_tests32\*.pdb
@dir runtime_tests32\*.rtl
@dir runtime_tests32\clb*.dll
@pause
