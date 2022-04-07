; RUN: SATest -BUILD --cpuarch=skx --tsize=16 --config=%s.cfg --dump-llvm-file - | FileCheck %s --check-prefixes=SKX,CHECK
; RUN: SATest -BUILD --cpuarch=core-avx2 --tsize=8 --config=%s.cfg --dump-llvm-file - | FileCheck %s --check-prefixes=COREAVX2,CHECK
; RUN: SATest -BUILD --cpuarch=corei7 --tsize=4 --config=%s.cfg --dump-llvm-file - | FileCheck %s --check-prefixes=COREI7,CHECK

; CHECK: define void @test{{.*}} !private_memory_size ![[PRIV_MEM_SIZE:[0-9]+]]
; SKX: ![[PRIV_MEM_SIZE]] = !{i32 65600}
; COREAVX2: ![[PRIV_MEM_SIZE]] = !{i32 32800}
; COREI7: ![[PRIV_MEM_SIZE]] = !{i32 16400}


; CHECK: Test program was successfully built.
