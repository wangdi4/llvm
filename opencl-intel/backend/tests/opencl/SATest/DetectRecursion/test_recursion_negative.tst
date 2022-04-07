; RUN: SATest -BUILD --cpuarch=skx --tsize=16 --config=%s.cfg --dump-llvm-file - | FileCheck %s --check-prefixes=SKX,CHECK
; RUN: SATest -BUILD --cpuarch=core-avx2 --tsize=8 --config=%s.cfg --dump-llvm-file - | FileCheck %s --check-prefixes=COREAVX2,CHECK
; RUN: SATest -BUILD --cpuarch=corei7 --tsize=4 --config=%s.cfg --dump-llvm-file - | FileCheck %s --check-prefixes=COREI7,CHECK

; CHECK-DAG: define void @test_foo{{.*}} !private_memory_size ![[PRIV_MEM_SIZE_FOO:[0-9]+]]
; CHECK-DAG: define void @test_bar{{.*}} !private_memory_size ![[PRIV_MEM_SIZE_BAR:[0-9]+]]

; SKX-DAG: ![[PRIV_MEM_SIZE_FOO]] = !{i32 64}
; SKX-DAG: ![[PRIV_MEM_SIZE_BAR]] = !{i32 0}

; COREAVX2-DAG: ![[PRIV_MEM_SIZE_FOO]] = !{i32 32}
; COREAVX2-DAG: ![[PRIV_MEM_SIZE_BAR]] = !{i32 0}

; COREI7-DAG: ![[PRIV_MEM_SIZE_FOO]] = !{i32 16}
; COREI7-DAG: ![[PRIV_MEM_SIZE_BAR]] = !{i32 0}

; CHECK: Test program was successfully built.
