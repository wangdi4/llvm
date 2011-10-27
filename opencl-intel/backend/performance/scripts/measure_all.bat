SET REV=4240
SET LOCATION=C:\Users\nrotem\Desktop\CVCC\


cd runtime_tests32

%LOCATION%\performance\scripts\bench_wolf.py corei7 > ../%REV%_corei7_32_0
%LOCATION%\performance\scripts\bench_wolf.py corei7 > ../%REV%_corei7_32_1
%LOCATION%\performance\scripts\bench_wolf.py corei7 > ../%REV%_corei7_32_2

%LOCATION%\performance\scripts\bench_wolf.py sandybridge > ../%REV%_sandybridge_32_0
%LOCATION%\performance\scripts\bench_wolf.py sandybridge > ../%REV%_sandybridge_32_1
%LOCATION%\performance\scripts\bench_wolf.py sandybridge > ../%REV%_sandybridge_32_2

cd ..

cd runtime_tests64

%LOCATION%\performance\scripts\bench_wolf.py corei7 > ../%REV%_corei7_64_0
%LOCATION%\performance\scripts\bench_wolf.py corei7 > ../%REV%_corei7_64_1
%LOCATION%\performance\scripts\bench_wolf.py corei7 > ../%REV%_corei7_64_2

%LOCATION%\performance\scripts\bench_wolf.py sandybridge > ../%REV%_sandybridge_64_0
%LOCATION%\performance\scripts\bench_wolf.py sandybridge > ../%REV%_sandybridge_64_1
%LOCATION%\performance\scripts\bench_wolf.py sandybridge > ../%REV%_sandybridge_64_2

cd ..

aggregate.py %REV%_corei7_32 %REV%_corei7_64 %REV%_sandybridge_32 %REV%_sandybridge_64 > %REV%_all.csv