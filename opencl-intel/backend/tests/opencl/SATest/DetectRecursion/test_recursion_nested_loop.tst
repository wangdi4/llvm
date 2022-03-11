; RUN: SATest -BUILD --config=%s.cfg --dump-llvm-file - | FileCheck %s

; CHECK: define void @test{{.*}} !private_memory_size ![[PRIV_MEM_SIZE:[0-9]+]]
; CHECK: ![[PRIV_MEM_SIZE]] = !{i32 65600}


; CHECK: Test program was successfully built.
