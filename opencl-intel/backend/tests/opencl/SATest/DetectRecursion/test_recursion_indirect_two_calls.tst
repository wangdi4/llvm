; RUN: SATest -BUILD --config=%s.cfg --dump-llvm-file - | FileCheck %s

; CHECK-DAG: define void @test_foo{{.*}} !private_memory_size ![[PRIV_MEM_SIZE_FOO:[0-9]+]]
; CHECK-DAG: define void @test_bar{{.*}} !private_memory_size ![[PRIV_MEM_SIZE_BAR:[0-9]+]]
; CHECK-DAG: ![[PRIV_MEM_SIZE_FOO]] = !{i32 131008}
; CHECK-DAG: ![[PRIV_MEM_SIZE_BAR]] = !{i32 131072}

; CHECK: Test program was successfully built.
