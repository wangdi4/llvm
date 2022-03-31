; RUN: %oclopt -SimplifyGEP -verify %s -S | FileCheck %s
; RUN: %oclopt -opaque-pointers -SimplifyGEP -verify %s -S | FileCheck %s

; ModuleID = 'Program'

declare i32 @_Z13get_global_idj(i32) nounwind readnone
declare i64 @_Z12get_local_idj(i64) nounwind readnone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP when GEP has two unifrom indices,
;; One is i32 and second is i64
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main3
; CHECK-NEXT: entry:
; CHECK-NEXT: arrayidx = getelementptr
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main3([4 x float] * %memA, float * nocapture %memB, i32 %i1, i64 %i2) nounwind {
entry:
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i32 %i1, i64 %i2
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}
