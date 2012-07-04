; RUN: llvm-as %s -o %t.bc
; RUN: opt  -SimplifyGEP -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'

declare i32 @get_global_id(i32) nounwind readnone
declare i64 @get_local_id(i64) nounwind readnone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check no affect when all GEPs has single index
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main1
; CHECK-NEXT: entry:
; CHECK-NEXT: arrayidx
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main1(float * %memA, float * nocapture %memB, i32 %i1, i32 %i2) nounwind {
entry:
  %arrayidx = getelementptr float * %memA, i32 %i1
  %A = load float * %arrayidx, align 4
  %arrayidx1 = getelementptr float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}
