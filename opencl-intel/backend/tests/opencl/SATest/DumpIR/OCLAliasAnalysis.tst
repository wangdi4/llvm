; RUN: SATest -BUILD -enable-expensive-mem-opts=1 -tsize=1 --config=%s.cfg --dump-llvm-file=- -dump-time-passes=- | FileCheck %s

; The test checks that OCLAliasAnalysis takes effect and the LICM pass moves
; loop invariants outside the loop.

; CHECK-LABEL: define void @test_fn
; CHECK: wrapper_entry:
; CHECK:   [[IDX:%.*]] = getelementptr inbounds i64, i64 addrspace(1)* %explicit_0, i64 15
; CHECK:   [[LOAD:%.*]] = load i64, i64 addrspace(1)* [[IDX]], align 8
; CHECK:   br label %[[FOR_BODY:.*]]
; CHECK: [[FOR_BODY]]:
; CHECK:   store i64 [[LOAD]], i64 addrspace(3)* {{%.*}}, align 8
; CHECK: Pass execution timing report
; CHECK: External Alias Analysis

; RUN: SATest -BUILD -enable-expensive-mem-opts=0 -tsize=1 --config=%s.cfg -dump-time-passes=- | FileCheck -check-prefix=DISABLE %s
; DISABLE:     Pass execution timing report
; DISABLE-NOT: External Alias Analysis
