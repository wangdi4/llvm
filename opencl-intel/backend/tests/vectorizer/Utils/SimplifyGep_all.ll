; RUN: %oclopt -SimplifyGEP -verify %s -S | FileCheck %s
; RUN: %oclopt -opaque-pointers -SimplifyGEP -verify %s -S | FileCheck %s

; ModuleID = 'Program'

declare i32 @_Z13get_global_idj(i32) nounwind readnone
declare i64 @_Z12get_local_idj(i64) nounwind readnone

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
  %arrayidx = getelementptr float, float * %memA, i32 %i1
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP when GEP has two uniform i32 indices
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main2
; CHECK-NEXT: entry:
; CHECK-NEXT: arrayidx = getelementptr
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main2([4 x float] * %memA, float * nocapture %memB, i32 %i1, i32 %i2) nounwind {
entry:
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i32 %i1, i32 %i2
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP when GEP has two 32 indices,
;; first one is uniform and the second is not uniform
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main4
; CHECK-NEXT: entry:
; CHECK-NEXT: %i3 = call i32
; CHECK-NEXT: arrayidx = getelementptr
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main4([4 x float] * %memA, float * nocapture %memB, i32 %i1, i32 %i2) nounwind {
entry:
  %i3 = call i32 @_Z13get_global_idj(i32 0) 
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i32 %i1, i32 %i3
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP when GEP has two indices, first
;; one is uniform i32 and the second is not uniform i64
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main5
; CHECK-NEXT: entry:
; CHECK-NEXT: %i3 = call i64
; CHECK-NEXT: arrayidx = getelementptr
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main5([4 x float] * %memA, float * nocapture %memB, i32 %i1, i32 %i2) nounwind {
entry:
  %i3 = call i64 @_Z12get_local_idj(i64 0) 
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i32 %i1, i64 %i3
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP by calculating its indices into 
;; one single index, when GEP has two i32 indices, 
;; first one is not uniform and the second is uniform
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main6
; CHECK-NEXT: entry:
; CHECK-NEXT: %i3 = call i32
; CHECK-NEXT: mulIndex = mul
; CHECK-NEXT: addIndex = add
; CHECK-NEXT: ptrTypeCast = bitcast
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main6([4 x float] * %memA, float * nocapture %memB, i32 %i1, i32 %i2) nounwind {
entry:
  %i3 = call i32 @_Z13get_global_idj(i32 0) 
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i32 %i3, i32 %i1
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP by calculating its indices into 
;; one single index, when GEP has two i64 indices, 
;; first one is not uniform and the second is uniform
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main7
; CHECK-NEXT: entry:
; CHECK-NEXT: %i3 = call i64
; CHECK-NEXT: mulIndex = mul
; CHECK-NEXT: addIndex = add
; CHECK-NEXT: ptrTypeCast = bitcast
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main7([4 x float] * %memA, float * nocapture %memB, i64 %i1, i32 %i2) nounwind {
entry:
  %i3 = call i64 @_Z12get_local_idj(i64 0) 
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i64 %i3, i64 %i1
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i64 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check no affect when GEP has two indices, first
;; one is not uniform i64 and the second is uniform i32
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main8
; CHECK-NEXT: entry:
; CHECK-NEXT: %i3 = call i64
; CHECK-NEXT: arrayidx = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main8([4 x float] * %memA, float * nocapture %memB, i32 %i1, i32 %i2) nounwind {
entry:
  %i3 = call i64 @_Z12get_local_idj(i64 0) 
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i64 %i3, i32 %i1
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}
