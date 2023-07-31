; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking" -aa-pipeline="basic-aa" -debug-only=hir-inter-loop-blocking-profit 2>&1 < %s | FileCheck %s
; REQUIRES: asserts

; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-inter-loop-blocking" -print-changed -disable-output 2>&1 < %s | FileCheck %s --check-prefix=CHECK-CHANGED

; memrefs are all in the shape of array[0][i2+1]. All dimension 2s do not have IV. Thus, no spatial loop corresponds to the dimension 2. We bail out without transformation.

; Function: main

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 91, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 20, 1   <DO_LOOP>
; CHECK:              |   |   (%mw4)[0][i2 + 1] = 62;
; CHECK:              |   |   %3 = (%oe)[0][i2 + 1];
; CHECK:              |   |   %4 = (%nz9)[0][i2 + 1];
; CHECK:              |   |   (%nz9)[0][i2 + 1] = %3 + %4;
; CHECK:              |   + END LOOP
; CHECK:              |
; CHECK:              |
; CHECK:              |   + DO i2 = 0, 44, 1   <DO_LOOP>
; CHECK:              |   |   %5 = (%mw4)[0][i2 + 1];
; CHECK:              |   |   (%oe)[0][i2 + 1] = -1 * %5;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK: No transformation: Some dimensions have no matching loop level

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRInterLoopBlocking

;Module Before HIR
; ModuleID = 'atg_CMPLRLLVM-25162.c'
source_filename = "atg_CMPLRLLVM-25162.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %nz9 = alloca [100 x i32], align 16
  %oe = alloca [100 x i32], align 16
  %mw4 = alloca [100 x i32], align 16
  %0 = bitcast ptr %nz9 to ptr
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %0) #3
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(400) %0, i8 0, i64 400, i1 false)
  %1 = bitcast ptr %oe to ptr
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %1) #3
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(400) %1, i8 0, i64 400, i1 false)
  %2 = bitcast ptr %mw4 to ptr
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %2) #3
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(400) %2, i8 0, i64 400, i1 false)
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc18
  %j.035 = phi i32 [ 2, %entry ], [ %inc19, %for.inc18 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr %mw4, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 62, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr %oe, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %3 = load i32, ptr %arrayidx5, align 4, !tbaa !2
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr %nz9, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %4 = load i32, ptr %arrayidx7, align 4, !tbaa !2
  %add = add i32 %4, %3
  store i32 %add, ptr %arrayidx7, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 22
  br i1 %exitcond, label %for.body10.preheader, label %for.body3

for.body10.preheader:                             ; preds = %for.body3
  br label %for.body10

for.body10:                                       ; preds = %for.body10.preheader, %for.body10
  %indvars.iv36 = phi i64 [ %indvars.iv.next37, %for.body10 ], [ 1, %for.body10.preheader ]
  %arrayidx12 = getelementptr inbounds [100 x i32], ptr %mw4, i64 0, i64 %indvars.iv36, !intel-tbaa !2
  %5 = load i32, ptr %arrayidx12, align 4, !tbaa !2
  %sub = sub i32 0, %5
  %arrayidx14 = getelementptr inbounds [100 x i32], ptr %oe, i64 0, i64 %indvars.iv36, !intel-tbaa !2
  store i32 %sub, ptr %arrayidx14, align 4, !tbaa !2
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %exitcond38 = icmp eq i64 %indvars.iv.next37, 46
  br i1 %exitcond38, label %for.inc18, label %for.body10

for.inc18:                                        ; preds = %for.body10
  %inc19 = add nuw nsw i32 %j.035, 1
  %exitcond39 = icmp eq i32 %inc19, 94
  br i1 %exitcond39, label %for.end20, label %for.cond1.preheader

for.end20:                                        ; preds = %for.inc18
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %2) #3
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %1) #3
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %0) #3
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind readnone uwtable "intel-lang"="fortran" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_j", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
