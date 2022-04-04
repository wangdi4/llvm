; RUN: %oclopt -SimplifyGEP -verify %s -S | FileCheck %s
; RUN: %oclopt -opaque-pointers -SimplifyGEP -verify %s -S | FileCheck %s

; ModuleID = 'Program'

declare i32 @_Z13get_global_idj(i32) nounwind readnone
declare i64 @_Z12get_local_idj(i64) nounwind readnone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check what sum of divergent and unifrom indices is split
;; in two GEPs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK:      @idxsum.basic
; CHECK-NOT:  %arrayidx = getelementptr float, float * %memA
; CHECK:      %uniformGEP = getelementptr float, {{.*}} %memA, i32 %uidx
; CHECK-NEXT: %divergentGEP = getelementptr float, {{.*}} %uniformGEP, i32 %didx
; CHECK-NEXT: %orig = load float, {{.*}} %divergentGEP

define void @idxsum.basic(float * %memA, float %factor, i32 %uidx) nounwind {
entry:
  %cidx = call i32 @_Z13get_global_idj(i32 0)
  %didx = mul i32 %cidx, %uidx
  %sum = add i32 %didx, %uidx
  %arrayidx = getelementptr float, float * %memA, i32 %sum
  %orig = load float , float * %arrayidx, align 4
  %mod  = fmul float %orig, %factor
  store float %mod, float * %arrayidx, align 4
  ret void
}


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check what a chain of sums of divergent and unifrom
;; 64 bit indices is split in two GEPs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK:      @idxsum.chain.0
; CHECK-NOT:  %arrayidx = getelementptr float, float * %memA
; CHECK:      %uniformIdx = add i64 %uidx{{[01]?}}, %uidx{{[01]?}}
; CHECK-NEXT: %divergentIdx = add i64 %didx{{[01]?}}, %didx{{[01]?}}
; CHECK-NEXT: %uniformGEP = getelementptr float, {{.*}} %memA, i64 %uniformIdx
; CHECK-NEXT: %divergentGEP = getelementptr float, {{.*}} %uniformGEP, i64 %divergentIdx
; CHECK-NEXT: %orig = load float, {{.*}} %divergentGEP

define void @idxsum.chain.0(float * %memA, float %factor, i64 %uidx0, i64 %uidx1) nounwind {
entry:
  %didx32 = call i32 @_Z13get_global_idj(i32 0)
  %didx0 = zext i32 %didx32 to i64
  %didx1 = call i64 @_Z12get_local_idj(i64 0)
  %sum0 = add i64 %didx0, %uidx0
  %sum1 = add i64 %didx1, %uidx1
  %sum = add i64 %sum0, %sum1
  %arrayidx = getelementptr float, float * %memA, i64 %sum
  %orig = load float , float * %arrayidx, align 4
  %mod  = fmul float %orig, %factor
  store float %mod, float * %arrayidx, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check what a chain of sums of divergent and unifrom
;; 32 bit indices is split in two GEPs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK:      @idxsum.chain.1
; CHECK-NOT:  %arrayidx = getelementptr float, float * %memA
; CHECK:      %uniformIdx = add i32 %uidx{{[01]?}}, %uidx{{[01]?}}
; CHECK-NEXT: %divergentIdx = add i32 %didx{{[01]?}}, %didx{{[01]?}}
; CHECK-NEXT: %uniformGEP = getelementptr float, {{.*}} %memA, i32 %uniformIdx
; CHECK-NEXT: %divergentGEP = getelementptr float, {{.*}} %uniformGEP, i32 %divergentIdx
; CHECK-NEXT: %orig = load float, {{.*}} %divergentGEP

define void @idxsum.chain.1(float * %memA, float %factor, i32 %uidx0, i32 %uidx1) nounwind {
entry:
  %didx0 = call i32 @_Z13get_global_idj(i32 0)
  %didx64 = call i64 @_Z12get_local_idj(i64 0)
  %didx1 = trunc i64 %didx64 to i32
  %sum0 = add i32 %didx0, %uidx0
  %sum1 = add i32 %didx1, %uidx1
  %sum = add i32 %sum0, %sum1
  %sext.sum = sext i32 %sum to i64
  %arrayidx = getelementptr float, float * %memA, i64 %sext.sum
  %orig = load float , float * %arrayidx, align 4
  %mod  = fmul float %orig, %factor
  store float %mod, float * %arrayidx, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check what a chain of sums of divergent and unifrom
;; indices of different sizes (32 and 64 bits) is split in two GEPs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK:      @idxsum.chain.2
; CHECK-NOT:  %arrayidx = getelementptr float, float * %memA
; CHECK:      %usum = add i64 %uidx{{[01]?}}, %uidx{{[01]?}}
; CHECK:      %divergentIdx = add i32 %didx{{[01]?}}, %didx{{[01]?}}
; CHECK-NEXT: %uniformGEP = getelementptr float, {{.*}} %memA, i64 %usum
; CHECK-NEXT: %divergentGEP = getelementptr float, {{.*}} %uniformGEP, i32 %divergentIdx
; CHECK-NEXT: %orig = load float, {{.*}} %divergentGEP

define void @idxsum.chain.2(float * %memA, float %factor, i64 %uidx0, i64 %uidx1) nounwind {
entry:
  %didx0 = call i32 @_Z13get_global_idj(i32 0)
  %didx64 = call i64 @_Z12get_local_idj(i64 0)
  %didx1 = trunc i64 %didx64 to i32
  %dsum = add i32 %didx0, %didx1
  %usum = add i64 %uidx0, %uidx1
  %sext.dsum = sext i32 %dsum to i64
  %sum = add i64 %usum, %sext.dsum
  %arrayidx = getelementptr float, float * %memA, i64 %sum
  %orig = load float , float * %arrayidx, align 4
  %mod  = fmul float %orig, %factor
  store float %mod, float * %arrayidx, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check what sum of consequtive and unifrom indices is
;; left as is and no new GEP is created.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK:      @idxsum.consecutive
; CHECK:      %sum = add i32 %cidx, %uidx
; CHECK-NEXT: %arrayidx = getelementptr float, {{.*}} %memA, i32 %sum
; CHECK-NOT:  uniformGEP

define void @idxsum.consecutive(float * %memA, float %factor, i32 %uidx) nounwind {
entry:
  %cidx = call i32 @_Z13get_global_idj(i32 0)
  %sum = add i32 %cidx, %uidx
  %arrayidx = getelementptr float, float * %memA, i32 %sum
  %orig = load float , float * %arrayidx, align 4
  %mod  = fmul float %orig, %factor
  store float %mod, float * %arrayidx, align 4
  ret void
}
