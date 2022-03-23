; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -S %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -S %s | FileCheck %s

; WARNING!!!
; WARNING!!!      ** CONTAINS INTEL IP **
; WARNING!!!      DO NOT SHARE EXTERNALLY
; WARNING!!!
;
; The original Fortran test from CMPLRLLVM_7431:
;
; integer i
; integer a(100), b(100)
; do i = 1, 100
;     a(i) = i;
; enddo
; !$omp parallel do default(none) shared(a,b)
; do i = 1, 100
;     b(i) = a(i);
; enddo
; !$omp end parallel do
;     print *, b
; end
;
; Compile with ifx -qopenmp -S -emit-llvm.
; Before the fix, it would fail (asert) in VPlanDriverHIR when the
; WRN graph construction tries to process the PARALLEL.LOOP construct
; in HIR representation.

; Verify that HIR emits the directive for the PARALLEL.LOOP constuct
; CHECK: %{{.*}} = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

source_filename = "parallel.f90"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @MAIN__() local_unnamed_addr #0 {
alloca:
  %ARGBLOCK_0 = alloca { i64, i8* }, align 8
  %addressof = alloca [4 x i8], align 1
  %litaddr = alloca i32, align 4
  %"_unnamed_main$$_$I" = alloca i32, align 8
  %"_unnamed_main$$_$A" = alloca [100 x i32], align 16
  %"_unnamed_main$$_$B" = alloca [100 x i32], align 16
  %"var$1" = alloca [8 x i64], align 16
  store i32 2, i32* %litaddr, align 4
  %0 = call i32 @for_set_reentrancy(i32* nonnull %litaddr)
  store i32 1, i32* %"_unnamed_main$$_$I", align 8
  %ptr_cast = getelementptr inbounds [100 x i32], [100 x i32]* %"_unnamed_main$$_$A", i64 0, i64 0
  br label %bb4

bb4:                                              ; preds = %bb4, %alloca
  %indvars.iv27 = phi i64 [ %indvars.iv.next28, %bb4 ], [ 1, %alloca ]
  %1 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %ptr_cast, i64 %indvars.iv27)
  %2 = trunc i64 %indvars.iv27 to i32
  store i32 %2, i32* %1, align 4
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond29 = icmp eq i64 %indvars.iv.next28, 101
  br i1 %exitcond29, label %bb5, label %bb4

bb5:                                              ; preds = %bb4
  store i32 101, i32* %"_unnamed_main$$_$I", align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE"(i32* %"_unnamed_main$$_$I"), "QUAL.OMP.SHARED"([100 x i32]* %"_unnamed_main$$_$B", [100 x i32]* %"_unnamed_main$$_$A"), "QUAL.OMP.DEFAULT.NONE"() ]
  store i32 1, i32* %"_unnamed_main$$_$I", align 8
  %ptr_cast14 = getelementptr inbounds [100 x i32], [100 x i32]* %"_unnamed_main$$_$B", i64 0, i64 0
  br label %bb12

bb12:                                             ; preds = %bb12, %bb5
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb12 ], [ 1, %bb5 ]
  %4 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %ptr_cast, i64 %indvars.iv)
  %5 = load i32, i32* %4, align 4
  %6 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %ptr_cast14, i64 %indvars.iv)
  store i32 %5, i32* %6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %bb13, label %bb12

bb13:                                             ; preds = %bb12
  store i32 101, i32* %"_unnamed_main$$_$I", align 8
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %.fca.0.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 0
  store i8 9, i8* %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 1
  store i8 5, i8* %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 2
  store i8 1, i8* %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 3
  store i8 0, i8* %.fca.3.gep, align 1
  %BLKFIELD_ = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_0, i64 0, i32 0
  store i64 400, i64* %BLKFIELD_, align 8
  %BLKFIELD_21 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_0, i64 0, i32 1
  %7 = bitcast i8** %BLKFIELD_21 to [100 x i32]**
  store [100 x i32]* %"_unnamed_main$$_$B", [100 x i32]** %7, align 8
  %arraydecay = bitcast [8 x i64]* %"var$1" to i8*
  %bit_cast23 = bitcast { i64, i8* }* %ARGBLOCK_0 to i8*
  %8 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %arraydecay, i32 -1, i64 1239157112576, i8* nonnull %.fca.0.gep, i8* nonnull %bit_cast23)
  ret void
}

declare i32 @for_set_reentrancy(i32*) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

attributes #0 = { "pre_loopopt" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }
