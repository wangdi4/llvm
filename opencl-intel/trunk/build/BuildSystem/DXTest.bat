set BUILD_TARGET=%1
set MODE=%2
set VECTORIZE=%3

python.exe ..\..\gfx\SACompilerTest\scripts\runtest.py --bin=..\..\build\win%BUILD_TARGET%\bin\%MODE%\SACompilerTest.exe --root=\\nntavc101xwb1.ccr.corp.intel.com\AVC.QA_OpenCL_resources\Volcano_Testing\PerformanceCriticalShaders --vectorize=%VECTORIZE% --baseline=..\..\gfx\SACompilerTest\scripts\baseline_perfcrit_%VECTORIZE%.txt --csv=logs\add\DXTest_V%VECTORIZE%_%MODE%_x%BUILD_TARGET%.csv  > logs\add\DXTest_V%VECTORIZE%_%MODE%_x%BUILD_TARGET%.log 2>&1

set path >>logs\add\path_env.log
cd >>logs\add\path_env.log
