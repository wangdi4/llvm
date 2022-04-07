; RUN: %oclopt -SimplifyGEP -verify -S < %s | FileCheck %s
; RUN: %oclopt -opaque-pointers -SimplifyGEP -verify -S < %s | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32) nounwind readnone
declare i64 @_Z12get_local_idj(i64) nounwind readnone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP uses getTypeAllocSize function 
;; Checked types:
;;                <3 x float> --> 4 elements
;;                <2 x    i1> --> 2 elements
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


define void @main1([7 x <2 x i1>] * %memA, i1 * nocapture %memB, i32 %i1, i32 %i3) nounwind {
entry:
  %i2 = call i32 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr [7 x <2 x i1>], [7 x <2 x i1>] * %memA, i32 %i1, i32 %i2, i32 %i3
  %A = load i1, i1 * %arrayidx, align 4
  %arrayidx1 = getelementptr i1, i1 * %memB, i32 %i1
  store i1 %A, i1 * %arrayidx1, align 4
  ret void
; CHECK: define void @main1
; CHECK-NEXT: entry:
; CHECK-NEXT:   %i2 = call i32 @_Z13get_global_idj(i32 0)
; CHECK-NEXT:   [[mulIndex:%[a-zA-Z0-9]+]] = mul nuw i32 %i1, 7
; CHECK-NEXT:   [[addIndex:%[a-zA-Z0-9]+]] = add nuw i32 [[mulIndex]], %i2
; CHECK-NEXT:   [[mulIndex1:%[a-zA-Z0-9]+]] = mul nuw i32 [[addIndex]], 1
; CHECK-NEXT:   [[addIndex2:%[a-zA-Z0-9]+]] = add nuw i32 [[mulIndex1]], %i3
; CHECK-NEXT:   [[ptrTypeCast:%[a-zA-Z0-9]+]] = bitcast {{.*}} %memA to {{.*}}
; CHECK-NEXT:   [[simplifiedGEP:%[a-zA-Z0-9]+]] = getelementptr i1, {{.*}} [[ptrTypeCast]], i32 [[addIndex2]]
; CHECK-NEXT:   %A = load i1, {{.*}} [[simplifiedGEP]], align 4
; CHECK-NEXT:   %arrayidx1 = getelementptr i1, {{.*}} %memB, i32 %i1
; CHECK-NEXT:   store i1 %A, {{.*}} %arrayidx1, align 4
; CHECK-NEXT: ret void
}

define void @main2([5 x <3 x float>] * %memA, float * nocapture %memB, i32 %i1, i32 %i3) nounwind {
entry:
  %i2 = call i32 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr [5 x <3 x float>], [5 x <3 x float>] * %memA, i32 %i1, i32 %i2, i32 %i3
  %A = load float, float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
; CHECK: define void @main2
; CHECK-NEXT: entry:
; CHECK-NEXT:   %i2 = call i32 @_Z13get_global_idj(i32 0)
; CHECK-NEXT:   [[mulIndex:%[a-zA-Z0-9]+]] = mul nuw i32 %i1, 5
; CHECK-NEXT:   [[addIndex:%[a-zA-Z0-9]+]] = add nuw i32 [[mulIndex]], %i2
; CHECK-NEXT:   [[mulIndex1:%[a-zA-Z0-9]+]] = mul nuw i32 [[addIndex]], 4
; CHECK-NEXT:   [[addIndex2:%[a-zA-Z0-9]+]] = add nuw i32 [[mulIndex1]], %i3
; CHECK-NEXT:   [[ptrTypeCast:%[a-zA-Z0-9]+]] = bitcast {{.*}} %memA to {{.*}}
; CHECK-NEXT:   [[simplifiedGEP:%[a-zA-Z0-9]+]] = getelementptr float, {{.*}} [[ptrTypeCast]], i32 [[addIndex2]]
; CHECK-NEXT:   %A = load float, {{.*}} [[simplifiedGEP]], align 4
; CHECK-NEXT:   %arrayidx1 = getelementptr float, {{.*}} %memB, i32 %i1
; CHECK-NEXT:   store float %A, {{.*}} %arrayidx1, align 4
; CHECK-NEXT: ret void
}
