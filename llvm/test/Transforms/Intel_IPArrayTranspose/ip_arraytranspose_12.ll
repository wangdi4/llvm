; This test verifies that there are no legality issues to enable array
; transpose. Array (%p1) is allocated in main and referenced in OpenMP
; routine "foo" that is called indirectly from __kmpc_fork_call.

; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:    Passed: All validations

%struct.ident_t = type { i32, i32, i32, i32, i8* }
@.source.0.0.9 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.10 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.9, i32 0, i32 0) }

declare void @llvm.lifetime.start.p0i8(i64, i8*)
declare void @llvm.lifetime.end.p0i8(i64, i8*)

define i32 @main() #0 {
b0:
  %g = alloca double*, align 8
  %lb = alloca i32, align 4
  %ub = alloca i32, align 4
  %p1 = tail call i8* @malloc(i64 214400000)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 320000
  %bc0 = bitcast i8* %pinc to double*
  %bc1 = bitcast i32* %lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %bc1)
  store double* %bc0, double** %g, align 8
  call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(%struct.ident_t* @.kmpc_loc.0.0.10, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, double**, i32*, i32*)* @foo to void (i32*, i32*, ...)*), double** %g, i32* %lb, i32* %ub)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %bc1)
  br label %b1

b1:                                               ; preds = %b0
  tail call void @free(i8* nonnull %p1)
  ret i32 0
}

define void @foo(i32* %tid, i32* %bid, double** %grid.addr, i32* %.omp.lb, i32* %.omp.ub) {
b0:
  %ptr = load double*, double** %grid.addr
  br label %b1

b1:                                                ; preds = %b2, %b0
  %ph1 = phi i64 [ 0, %b0 ], [ %inc, %b2 ]
  %inc1 = add nuw nsw i64 %ph1, 19
  %gep1 = getelementptr inbounds double, double* %ptr, i64 %inc1
  %bc1 = bitcast double* %gep1 to i32*
  %ld = load i32, i32* %bc1, align 4
  %cmp0 = icmp eq i32 %ld, 0
  br i1 %cmp0, label %b2, label %b2

b2:                                               ; preds = %b1, %b1
  %inc = add nuw nsw i64 %ph1, 20
  %cmp = icmp ult i64 %ph1, 25999980
  br i1 %cmp, label %b1, label %b3

b3:
  ret void
}

declare noalias i8* @malloc(i64) #1
declare void @free(i8* nocapture) #2
declare void @__kmpc_fork_call(%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...)

attributes #0 = { norecurse }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
