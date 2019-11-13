; Test to check that DA ignores integer overflow clamping pattern, and propagates shape of operand being checked for overflow.

; REQUIRES: asserts
; RUN: opt %s -vplan-da-ignore-integer-overflow=true -debug-only=vplan-divergence-analysis -VPlanDriver -S -vplan-force-vf=4 2>&1 | FileCheck %s

; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i32 1] i32 [[IV_PHI:%vp.*]] = phi  [ i32 0, BB2 ],  [ i32 [[IV_PHI_ADD:%vp.*]], BB5 ]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i32 1] i64 [[IV_SEXT:%vp.*]] = sext i32 [[IV_PHI]] to i64
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[UNIT_ADD:%vp.*]] = add i64 [[IV_SEXT]] i64 %uni.call
; CHECK-NEXT: Uniform: [Shape: Uniform] i64* [[UNI_GEP:%vp.*]] = getelementptr inbounds i64* %arr i64 42
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[UNI_LOAD:%vp.*]] = load i64* [[UNI_GEP]]
; CHECK-NEXT: Uniform: [Shape: Uniform] float* [[UNI_GEP2:%vp.*]] = getelementptr inbounds float* %arr1 i64 [[UNI_LOAD]]

; CHECK-LABEL: Basic Block: BB4
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[INNER_IV:%vp.*]] = phi  [ i64 0, BB3 ],  [ i64 [[INNER_IV_ADD:%vp.*]], BB4 ]
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[UNI_MUL:%vp.*]] = mul i64 [[INNER_IV]] i64 %uni.call2
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[UNIT_ADD2:%vp.*]] = add i64 [[UNI_MUL]] i64 [[UNIT_ADD]]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[OVERFLOW_CHK:%vp.*]] = and i64 [[UNIT_ADD2]] i64 4294967295
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] float* [[UNIT_GEP:%vp.*]] = getelementptr inbounds float* [[UNI_GEP2]] i64 [[OVERFLOW_CHK]]
; CHECK-NEXT: Divergent: [Shape: Random] float [[UNIT_LOAD:%vp.*]] = load float* [[UNIT_GEP]]
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[INNER_IV_ADD]] = add i64 [[INNER_IV]] i64 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[INNER_IV_CMP:%vp.*]] = icmp i64 [[INNER_IV_ADD]] i64 %n

; CHECK-LABEL: Basic Block: BB5
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i32 [[IV_PHI_ADD]] = add i32 [[IV_PHI]] i32 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[IV_PHI_CMP:%vp.*]] = icmp i32 [[IV_PHI_ADD]] i32 1024


; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32) local_unnamed_addr #1

; Function Attrs: nounwind readnone
declare i64 @_Z14get_local_sizej(i32) local_unnamed_addr #1

define void @test1(i64 %n, i64* %arr, float* %arr1) {
  entry:
  %cmp = icmp sgt i64 %n, 0
  %uni.call = call i64 @_Z12get_local_idj(i32 0) #1
  %uni.call2 = call i64 @_Z14get_local_sizej(i32 0) #1
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(float* %arr1, i64* %arr) ]
  br label %for.body

for.body:
  %indvars.iv = phi i32 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %outer.iv = sext i32 %indvars.iv to i64
  %unit.add = add i64 %outer.iv, %uni.call
  %outer.gep = getelementptr inbounds i64, i64* %arr, i64 42
  %outer.load = load i64, i64* %outer.gep
  %outer.gep2 = getelementptr inbounds float, float* %arr1, i64 %outer.load
  br label %for.body2

for.body2:
  %indvars.iv2 = phi i64 [ 0, %for.body ], [ %indvars.iv.next2, %for.body2 ]
  %mul = mul i64 %indvars.iv2, %uni.call2
  %add = add i64 %mul, %unit.add
  %overflow.clamp = and i64 %add, 4294967295
  %unit.stride.gep = getelementptr inbounds float, float* %outer.gep2, i64 %overflow.clamp
  %unit.stride.load = load float, float* %unit.stride.gep
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond2, label %for.latch, label %for.body2

for.latch:
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}
