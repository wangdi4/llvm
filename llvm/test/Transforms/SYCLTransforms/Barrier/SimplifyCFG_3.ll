; RUN: opt -passes=simplifycfg -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=simplifycfg -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the the LLVM pass SimplifyCFG does not duplicate function
;; with noduplicate attribute.
;; The case: Function @bar with call to function @foo with barrier instructions
;;           (just before if()/if()-else basic blocks)
;;           Function @foo has a noduplicate attribute
;; The expected result:
;;      1. same number of calls to @foo
;;*****************************************************************************

; CHECK: @bar
; CHECK-NOT: call void @foo
; CHECK: call void @foo(i64 1)
; CHECK: call void @foo(i64 1)
; CHECK-NOT: call void @foo
; CHECK: declare void @foo(i64) #1

@Buff = internal addrspace(3) global [128 x float] zeroinitializer, align 16

define void @bar(i1 %x, i1 %y, ptr addrspace(1) noalias nocapture %out) #0 {
BB_0:
  store volatile float 0.000000e+000, ptr addrspace(3) @Buff, align 4
  br label %BB_70

BB_70:                                      ; preds = %BB_0
  tail call void @foo(i64 1) nounwind
  br i1 %y, label %BB_72, label %BB_79

BB_72:                                      ; preds = %BB_70
  store volatile float 1.0, ptr addrspace(3) @Buff, align 4
  br label %BB_79

BB_79:                                      ; preds = %BB_72, %BB_70
  %.pr = phi i1 [ %x, %BB_72 ], [ false, %BB_70 ]
  tail call void @foo(i64 1) nounwind
  br i1 %.pr, label %BB_80, label %BB_83

BB_80:                                      ; preds = %BB_79
  %load1 = load volatile float, ptr addrspace(3) @Buff, align 4
  %ptr1 = getelementptr inbounds float, ptr addrspace(1) %out, i64 4
  store float %load1, ptr addrspace(1) %ptr1, align 4
  ret void

BB_83:                                      ; preds = %BB_79, %BB_0
  ret void
}

declare void @foo(i64) #1 

attributes #0 = { nounwind }
attributes #1 = { noduplicate }

; TODO: SimplifyCFG changes led to missing debug info. Will remove after this
; issue is fixed.
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function bar --  br label %common.ret
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 13
; DEBUGIFY: WARNING: Missing line 14

; DEBUGIFY-NOT: WARNING
