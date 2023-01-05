; This test verifies that array transpose is not enabled because
; there are no SCEVAddRec expressions in SCEV.
;
; SCEV that has problem:
; ((8 * (sext i32 %tmp26 to i64))<nsw> + %arg2)<nsw>

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
  br i1 %tmp11, label %bb19, label %bb12

bb12:                                             ; preds = %bb
  %tmp13 = add i32 %tmp10, 1
  br label %bb14

bb14:                                             ; preds = %bb35, %bb12
  %tmp15 = phi i32 [ %tmp36, %bb35 ], [ %tmp9, %bb12 ]
  %tmp16 = mul i32 %tmp15, 40002
  %tmp17 = add i32 %tmp16, -80000
  %tmp18 = icmp eq i32 %tmp15, 2
  br label %bb20

bb19:                                             ; preds = %bb35, %bb
  ret void

bb20:                                             ; preds = %bb42, %bb14
  %tmp21 = phi i64 [ 0, %bb14 ], [ %tmp43, %bb42 ]
  %tmp22 = trunc i64 %tmp21 to i32
  %tmp23 = mul i32 %tmp22, 200
  %tmp24 = add i32 %tmp23, %tmp17
  %tmp25 = mul nsw i32 %tmp24, 20
  %tmp26 = or i32 %tmp25, 19
  %tmp27 = sext i32 %tmp26 to i64
  %tmp28 = getelementptr inbounds double, double* %arg2, i64 %tmp27
  %tmp29 = bitcast double* %tmp28 to i32*
  %tmp30 = load i32, i32* %tmp29, align 4
  %tmp31 = or i32 %tmp30, 1
  store i32 %tmp31, i32* %tmp29, align 4
  %tmp32 = add i32 %tmp22, -2
  %tmp33 = icmp ult i32 %tmp32, 196
  %tmp34 = icmp eq i32 %tmp22, 0
  br label %bb58

bb35:                                             ; preds = %bb42
  %tmp36 = add nsw i32 %tmp15, 1
  %tmp37 = icmp eq i32 %tmp36, %tmp13
  br i1 %tmp37, label %bb19, label %bb14

bb38:                                             ; preds = %bb68, %bb62
  %tmp39 = phi i64 [ %tmp63, %bb62 ], [ %tmp69, %bb68 ]
  %tmp40 = phi i32 [ %tmp60, %bb62 ], [ %tmp41, %bb68 ]
  %tmp41 = trunc i64 %tmp39 to i32
  switch i32 %tmp40, label %bb45 [
    i32 -1, label %bb46
    i32 198, label %bb46
  ]

bb42:                                             ; preds = %bb68, %bb46
  %tmp43 = add nuw nsw i64 %tmp21, 1
  %tmp44 = icmp eq i64 %tmp43, 200
  br i1 %tmp44, label %bb35, label %bb20

bb45:                                             ; preds = %bb38
  br i1 %tmp34, label %bb46, label %bb67

bb46:                                             ; preds = %bb67, %bb67, %bb61, %bb58, %bb58, %bb45, %bb38, %bb38
  %tmp47 = phi i32 [ %tmp59, %bb58 ], [ %tmp59, %bb58 ], [ %tmp59, %bb61 ], [ %tmp41, %bb67 ], [ %tmp41, %bb67 ], [ %tmp41, %bb38 ], [ %tmp41, %bb38 ], [ %tmp41, %bb45 ]
  %tmp48 = add i32 %tmp24, %tmp47
  %tmp49 = mul nsw i32 %tmp48, 20
  %tmp50 = add nsw i32 %tmp49, 19
  %tmp51 = sext i32 %tmp50 to i64
  %tmp52 = getelementptr inbounds double, double* %arg2, i64 %tmp51
  %tmp54 = load double, double* %tmp52, align 4
  %tmp56 = add nsw i32 %tmp47, 1
  %tmp57 = icmp slt i32 %tmp47, 199
  br i1 %tmp57, label %bb58, label %bb42

bb58:                                             ; preds = %bb46, %bb20
  %tmp59 = phi i32 [ 1, %bb20 ], [ %tmp56, %bb46 ]
  %tmp60 = phi i32 [ 0, %bb20 ], [ %tmp47, %bb46 ]
  switch i32 %tmp22, label %bb61 [
    i32 0, label %bb46
    i32 199, label %bb46
  ]

bb61:                                             ; preds = %bb58
  br i1 %tmp18, label %bb46, label %bb62

bb62:                                             ; preds = %bb61
  %tmp63 = sext i32 %tmp59 to i64
  %tmp64 = icmp sgt i64 %tmp63, 199
  %tmp65 = select i1 %tmp64, i64 %tmp63, i64 199
  %tmp66 = add nuw nsw i64 %tmp65, 1
  br label %bb38

bb67:                                             ; preds = %bb45
  switch i32 %tmp15, label %bb68 [
    i32 2, label %bb46
    i32 261, label %bb46
    i32 3, label %bb71
    i32 260, label %bb71
  ]

bb68:                                             ; preds = %bb75, %bb71, %bb67
  %tmp69 = add nsw i64 %tmp39, 1
  %tmp70 = icmp eq i64 %tmp69, %tmp66
  br i1 %tmp70, label %bb42, label %bb38

bb71:                                             ; preds = %bb67, %bb67
  %tmp72 = add i32 %tmp40, -1
  %tmp73 = icmp ult i32 %tmp72, 196
  %tmp74 = and i1 %tmp33, %tmp73
  br i1 %tmp74, label %bb75, label %bb68

bb75:                                             ; preds = %bb71
  %tmp76 = add i32 %tmp24, %tmp41
  %tmp77 = mul nuw nsw i32 %tmp76, 20
  %tmp78 = add nuw nsw i32 %tmp77, 19
  %tmp79 = sext i32 %tmp78 to i64
  %tmp80 = getelementptr inbounds double, double* %arg2, i64 %tmp79
  %tmp81 = bitcast double* %tmp80 to i32*
  store i32 1, i32* %tmp81, align 4
  br label %bb68
}

declare noalias i8* @malloc(i64) #1
declare void @free(i8* nocapture) #2
declare void @__kmpc_fork_call(%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...)
declare void @__kmpc_for_static_init_4(%struct.ident_t* nocapture readonly, i32, i32, i32* nocapture, i32* nocapture, i32* nocapture, i32* nocapture, i32, i32)

attributes #0 = { norecurse }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
