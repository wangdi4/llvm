; This test verifies that array transpose is not enabled because
; constant multiplier (i.e 10) is not divisible by stride.
;
; SCEV that has problem:
; (8 + (8 * (sext i32 (18 + (10 * ({{(-80000 + (40000 * %tmp9)),+,40000}<%bb14>,+,200}<nw><%bb19> + %tmp53))) to i64))<nsw> + %arg2)

; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:    Failed: MemRefs validation failed

%struct.ident_t = type { i32, i32, i32, i32, i8* }
@.source.0.0.9 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.10 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.9, i32 0, i32 0) }

define i32 @main() #0 {
b0:
  %p1 = tail call i8* @malloc(i64 214400000)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 320000
  %bc0 = bitcast i8* %pinc to double*
  call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(%struct.ident_t* nonnull @.kmpc_loc.0.0.10, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, double*, i64, i64)* @foo to void (i32*, i32*, ...)*), double* %bc0, i64 0, i64 263)
  br label %b1

b1:                                               ; preds = %b0
  tail call void @free(i8* nonnull %p1)
  ret i32 0
}

define void @foo(i32* nocapture readonly %arg, i32* nocapture readnone %arg1, double* nocapture %arg2, i64 %arg3, i64 %arg4) {
bb:
  %tmp = alloca i32, align 4
  %tmp5 = alloca i32, align 4
  %tmp6 = alloca i32, align 4
  %tmp7 = alloca i32, align 4
  store i32 0, i32* %tmp, align 4
  %tmp8 = load i32, i32* %arg, align 4
  store i32 0, i32* %tmp5, align 4
  store i32 263, i32* %tmp6, align 4
  store i32 1, i32* %tmp7, align 4
  call void @__kmpc_for_static_init_4(%struct.ident_t* nonnull @.kmpc_loc.0.0.10, i32 %tmp8, i32 34, i32* nonnull %tmp, i32* nonnull %tmp5, i32* nonnull %tmp6, i32* nonnull %tmp7, i32 1, i32 1)
  %tmp9 = load i32, i32* %tmp5, align 4
  %tmp10 = load i32, i32* %tmp6, align 4
  %tmp11 = icmp sgt i32 %tmp9, %tmp10
  br i1 %tmp11, label %bb18, label %bb12

bb12:                                             ; preds = %bb
  %tmp13 = add i32 %tmp10, 1
  br label %bb14

bb14:                                             ; preds = %bb38, %bb12
  %tmp15 = phi i32 [ %tmp39, %bb38 ], [ %tmp9, %bb12 ]
  %tmp16 = mul i32 %tmp15, 40000
  %tmp17 = add i32 %tmp16, -80000
  br label %bb19

bb18:                                             ; preds = %bb38, %bb
  ret void

bb19:                                             ; preds = %bb46, %bb14
  %tmp20 = phi i32 [ 0, %bb14 ], [ %tmp47, %bb46 ]
  %tmp21 = mul nuw nsw i32 %tmp20, 200
  %tmp22 = add i32 %tmp17, %tmp21
  %tmp23 = icmp eq i32 %tmp20, 0
  %tmp24 = icmp eq i32 %tmp20, 199
  br i1 %tmp23, label %bb25, label %bb52
bb25:                                             ; preds = %bb25, %bb19
  %tmp26 = phi i64 [ %tmp37, %bb25 ], [ 0, %bb19 ]
  %tmp27 = trunc i64 %tmp26 to i32
  %tmp28 = add i32 %tmp22, %tmp27
  %tmp29 = mul nsw i32 %tmp28, 20
  %tmp30 = add nsw i32 %tmp29, 19
  %tmp31 = sext i32 %tmp30 to i64
  %tmp32 = getelementptr inbounds double, double* %arg2, i64 %tmp31
  %tmp34 = load double, double* %tmp32, align 4
  %tmp33 = bitcast double %tmp34 to i64
  %tmp36 = icmp eq i64 %tmp26, 199
  %tmp37 = add nuw nsw i64 %tmp26, 1
  br i1 %tmp36, label %bb46, label %bb25

bb38:                                             ; preds = %bb46
  %tmp39 = add nsw i32 %tmp15, 1
  %tmp40 = icmp eq i32 %tmp39, %tmp13
  br i1 %tmp40, label %bb18, label %bb14

bb41:                                             ; preds = %bb62, %bb49
  %tmp42 = phi i32 [ %tmp43, %bb49 ], [ %tmp53, %bb62 ]
  %tmp43 = add i32 %tmp42, 1
  %tmp44 = icmp eq i32 %tmp42, %tmp64
  br i1 %tmp44, label %bb46, label %bb45

bb45:                                             ; preds = %bb41
  switch i32 %tmp42, label %bb49 [
    i32 -1, label %bb50
    i32 198, label %bb50
  ]

bb46:                                             ; preds = %bb65, %bb41, %bb25
  %tmp47 = add nuw nsw i32 %tmp20, 1
  %tmp48 = icmp eq i32 %tmp47, 200
  br i1 %tmp48, label %bb38, label %bb19
bb49:                                             ; preds = %bb45
  switch i32 %tmp20, label %bb41 [
    i32 0, label %bb50
    i32 199, label %bb50
  ]

bb50:                                             ; preds = %bb65, %bb49, %bb49, %bb45, %bb45
  %tmp51 = phi i32 [ %tmp66, %bb65 ], [ %tmp43, %bb49 ], [ %tmp43, %bb49 ], [ %tmp43, %bb45 ], [ %tmp43, %bb45 ]
  br label %bb52

bb52:                                             ; preds = %bb50, %bb19
  %tmp53 = phi i32 [ %tmp51, %bb50 ], [ 0, %bb19 ]
  %tmp54 = add i32 %tmp22, %tmp53
  %tmp55 = mul nsw i32 %tmp54, 10
  %tmp56 = add nsw i32 %tmp55, 19
  %tmp57 = sext i32 %tmp56 to i64
  %tmp58 = getelementptr inbounds double, double* %arg2, i64 %tmp57
  %tmp59 = bitcast double* %tmp58 to i32*
  store i32 1, i32* %tmp59, align 4
  br i1 %tmp24, label %bb65, label %bb62

bb62:                                             ; preds = %bb52
  %tmp63 = icmp sgt i32 %tmp53, 199
  %tmp64 = select i1 %tmp63, i32 %tmp53, i32 199
  br label %bb41

bb65:                                             ; preds = %bb52
  %tmp66 = add nsw i32 %tmp53, 1
  %tmp67 = icmp slt i32 %tmp53, 199
  br i1 %tmp67, label %bb50, label %bb46
}

declare noalias i8* @malloc(i64) #1
declare void @free(i8* nocapture) #2
declare void @__kmpc_fork_call(%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...)
declare void @__kmpc_for_static_init_4(%struct.ident_t* nocapture readonly, i32, i32, i32* nocapture, i32* nocapture, i32* nocapture, i32* nocapture, i32, i32)

attributes #0 = { norecurse }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
