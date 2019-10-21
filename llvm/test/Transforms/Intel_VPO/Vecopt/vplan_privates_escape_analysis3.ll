; This test verifies that private-variables escaping into the unknown functions
; via an intermediate write is captured as 'unsafe'.

; RUN: opt %s -S -VPlanDriver -vplan-force-vf=4  -disable-vplan-codegen \
; RUN: -enable-vp-value-codegen=true -vplan-use-entity-instr -disable-output \
; RUN: -debug-only=vploop-analysis 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK-DAG: SOAUnsafe = [1024 x i32]* %arr_e.priv
; CHECK-DAG: SOAUnsafe = i32** %ptr.priv
; CHECK-DAG: SOASafe = i32* %index.lpriv


;Source Code: test2.c
;int arr_e[1024];
;extern int helper(int *elem);
;extern int helper2(void);
;int foo(int n1, int k) {
;  int index;
;#pragma omp simd private(arr_e)
;  for (index = 1; index < 1024; index++) {
;    int *ptr = &arr_e[index];
;    ptr[k] = helper2();
;    helper(&ptr[k]);
;  }
;  return arr_e[index-k];
;}

; Compile-command: icx test2.c -o out.ll -fiopenmp -O1 -S  \
; -mllvm -disable-vplan-codegen -mllvm -vplan-entities-dump \
; -mllvm -vplan-use-entity-instr -emit-llvm


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr_e = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo(i32 %n1, i32 %k) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %ptr.priv = alloca i32*, align 8
  %arr_e.priv = alloca [1024 x i32], align 4
  %index.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_e.priv), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %index.lpriv), "QUAL.OMP.PRIVATE"(i32** %ptr.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32** %ptr.priv to i8*
  %idxprom1 = sext i32 %k to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.2 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %2 = trunc i64 %indvars.iv.next to i32
  store i32 %2, i32* %index.lpriv, align 4
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %1)
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 %indvars.iv.next, !intel-tbaa !6
  store i32* %arrayidx, i32** %ptr.priv, align 8
  %call = call i32 @helper2()
  %3 = load i32*, i32** %ptr.priv, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %3, i64 %idxprom1
  store i32 %call, i32* %arrayidx2, align 4
  %call5 = call i32 @helper(i32* %arrayidx2)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %1)
  %exitcond = icmp eq i64 %indvars.iv.next, 1023
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %4 = load i32, i32* %index.lpriv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %sub = sub nsw i32 %4, %k
  %idxprom7 = sext i32 %sub to i64
  %arrayidx8 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr_e, i64 0, i64 %idxprom7, !intel-tbaa !6
  %5 = load i32, i32* %arrayidx8, align 4, !tbaa !6
  ret i32 %5
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare dso_local i32 @helper2() local_unnamed_addr

declare dso_local i32 @helper(i32*) local_unnamed_addr

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1024_i", !3, i64 0}
