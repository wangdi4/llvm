; RUN: SATest -BUILD --cpuarch=skx --tsize=16 --config=%s.cfg --dump-llvm-file - | FileCheck %s

; CHECK-DAG: define dso_local void @test_bar{{.*}} !private_memory_size ![[PRIV_MEM_SIZE_BAR:[0-9]+]]
; CHECK-DAG: define dso_local void @test_foo{{.*}} !private_memory_size ![[PRIV_MEM_SIZE_FOO:[0-9]+]]

; CHECK-DAG: ![[PRIV_MEM_SIZE_BAR]] = !{i32 8188}
; CHECK-DAG: ![[PRIV_MEM_SIZE_FOO]] = !{i32 8192}

; CHECK-DAG: Test program was successfully built.
