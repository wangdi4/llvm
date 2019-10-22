; This test verifies that private-variables escaping out through a write
; to an output argument are unsafe for data-layout transformation.

; RUN: opt %s -S -VPlanDriver -vplan-force-vf=4  -disable-vplan-codegen \
; RUN: -enable-vp-value-codegen=true -vplan-use-entity-instr  -disable-output \
; RUN: -debug-only=vploop-analysis 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK-DAG: SOAUnsafe = [1024 x i32]* %arr_e.priv
; CHECK-DAG: SOASafe = i32* %index.lpriv
; CHECK-DAG: SOASafe = [1024 x i32]* %arr_ne.priv

; Source-file: test.c
; int foo(int n1, int **out) {
;  int index;
;
; #pragma omp simd private(arr_ne, arr_e)
;  for (index = 0; index < 1024; index++) {
;    if (index % 2 == 0)
;      arr_ne[index] = index + n1;
;    else
;      arr_ne[index] = index - n1;
;
;    if ((index + n1) > 1024)
;      *out = &arr_e[n1];
;   }
;   return arr_e[n1];
; }


; Compile-command: icx test.c -o out.ll -fiopenmp -O1 -S  \
; -mllvm -disable-vplan-codegen -mllvm -vplan-entities-dump \
; -mllvm -vplan-use-entity-instr -emit-llvm

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr_ne = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr_e = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo(i32 %n1, i32** nocapture %out) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %arr_e.priv = alloca [1024 x i32], align 4
  %arr_ne.priv = alloca [1024 x i32], align 4
  %index.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_ne.priv), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_e.priv), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %index.lpriv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %idxprom8 = sext i32 %n1 to i64
  %arrayidx9 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 %idxprom8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.2 ]
  %rem1925 = and i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %rem1925, 0
  %1 = trunc i64 %indvars.iv to i32
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %omp.inner.for.body
  %add2 = add nsw i32 %1, %n1
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_ne.priv, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add2, i32* %arrayidx, align 4, !tbaa !2
  br label %if.end

if.else:                                          ; preds = %omp.inner.for.body
  %sub = sub nsw i32 %1, %n1
  %arrayidx4 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_ne.priv, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %sub, i32* %arrayidx4, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = trunc i64 %indvars.iv to i32
  %add5 = add nsw i32 %2, %n1
  %cmp6 = icmp sgt i32 %add5, 1024
  br i1 %cmp6, label %if.then7, label %omp.inner.for.inc

if.then7:                                         ; preds = %if.end
  store i32* %arrayidx9, i32** %out, align 8, !tbaa !7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then7, %if.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  store i32 1023, i32* %index.lpriv, align 4, !tbaa !9
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %idxprom12 = sext i32 %n1 to i64
  %arrayidx13 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr_e, i64 0, i64 %idxprom12, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx13, align 4, !tbaa !2
  ret i32 %3
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"pointer@_ZTSPi", !5, i64 0}
!9 = !{!4, !4, i64 0}
