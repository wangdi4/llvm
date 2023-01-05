; RUN: SATest -BUILD --cpuarch=skx --tsize=16 --config=%s.cfg --dump-llvm-file - | FileCheck %s

; CHECK-DAG: define dso_local void @test{{.*}} !private_memory_size ![[PRIV_MEM_SIZE:[0-9]+]]
; CHECK-DAG: ![[PRIV_MEM_SIZE]] = !{i32 8192}

; CHECK-DAG: Test program was successfully built.
