; Verify that we have correct types for 'sext' instruction and the next one. 
; Input LLVM IR is generated for below code with command:  icx -O2 -mllvm -print-module-before-loopopt
;


;long long a[100000];
;
;int foo(int N) {
;  int t = 2;
;  int i;
;  long y = 10;
;#pragma omp simd
;  for (i = 0; i < N; i+=y) {
;    if (a[i] < i)
;      t = a[i];
;    else
;      t = a[i + 1];
;  }
;  return t;
;}


;=================================Print after simplify plain CFG=================================
;  REGION: region1
;  BB2:
;   <Empty Block>
;  SUCCESSORS(1):BB3
;
;  BB3:
;   i64 %vp48864 = add i64 %vp48736 i64 -1
;  SUCCESSORS(1):BB4
;
;  BB4:
;   i64 %vp40448 = phi [ i64 0, BB3 ], [ i64 %vp48432, BB10 ]
;   i64 %vp40672 = load i64 %vp40608
;   i64 %vp41872 = bitcast i64 %vp40672
;   i32 %vp42128 = trunc i64 %vp40448
;   i32 %vp42288 = mul i32 10 i32 %vp42128
;   i64 %vp42448 = sext i32 %vp42288
;   i1 %vp42608 = icmp i64 %vp40672 i64 %vp42448
;  SUCCESSORS(1):BB9
;
;  BB9:
;   <Empty Block>
;   Condition(BB4): i1 %vp42608 = icmp i64 %vp40672 i64 %vp42448
;  SUCCESSORS(2):BB5(i1 %vp42608), BB6(!i1 %vp42608)
;
;    BB5:
;     i64 %vp47760 = load i64 %vp47696
;     i64 %vp47952 = bitcast i64 %vp47760
;    SUCCESSORS(1):BB6
;
;  BB6:
;   i64 %vp48432 = add i64 %vp40448 i64 1
;   i1 %vp49024 = icmp i64 %vp48432 i64 %vp48864
;  SUCCESSORS(1):BB10
;
;  BB10:
;   <Empty Block>
;   Condition(BB6): i1 %vp49024 = icmp i64 %vp48432 i64 %vp48864
;  SUCCESSORS(2):BB4(i1 %vp49024), BB7(!i1 %vp49024)
;
;  BB7:
;   <Empty Block>
;  SUCCESSORS(1):BB8
;
;  BB8:
;   <Empty Block>
;  END Block - no SUCCESSORS
;  END Region(region1)
;=================================================================================================

; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -S -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir" -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; Check decomposed VPInstructions
; CHECK: load
; CHECK: i64 [[SEXT:%vp.*]] = sext i32 [[OP:%vp.*]]
; CHECK-NEXT: i1 [[CMP:%vp.*]] = icmp i64 [[OP1:%vp.*]] i64 [[SEXT]]

;Module Before HIR; ModuleID = 't4.c'
source_filename = "t4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [100000 x i64] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.134, label %omp.precond.end

DIR.OMP.SIMD.134:                                 ; preds = %entry
  %sub3 = add nsw i32 %N, -1
  %conv = sext i32 %sub3 to i64
  %add = add nsw i64 %conv, 10
  %div = sdiv i64 %add, 10
  %conv4 = trunc i64 %div to i32
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %cmp735 = icmp slt i32 %conv4, 1
  br i1 %cmp735, label %omp.loop.exit, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %DIR.OMP.SIMD.134
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.preheader, %omp.inner.for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.preheader ]
  %1 = trunc i64 %indvars.iv to i32
  %mul = mul i32 %1, 10
  %idxprom = sext i32 %mul to i64
  %arrayidx = getelementptr inbounds [100000 x i64], [100000 x i64]* @a, i64 0, i64 %idxprom, !intel-tbaa !2
  %2 = load i64, i64* %arrayidx, align 16, !tbaa !2
  %cmp13 = icmp slt i64 %2, %idxprom
  br i1 %cmp13, label %omp.inner.for.inc, label %if.else

if.else:                                          ; preds = %omp.inner.for.body
  %add18 = or i32 %mul, 1
  %idxprom19 = sext i32 %add18 to i64
  %arrayidx20 = getelementptr inbounds [100000 x i64], [100000 x i64]* @a, i64 0, i64 %idxprom19, !intel-tbaa !2
  %3 = load i64, i64* %arrayidx20, align 8, !tbaa !2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.else
  %t.1.in = phi i64 [ %3, %if.else ], [ %2, %omp.inner.for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %div
  br i1 %exitcond, label %omp.loop.exit.loopexit, label %omp.inner.for.body

omp.loop.exit.loopexit:                           ; preds = %omp.inner.for.inc
  %t.1.in.lcssa = phi i64 [ %t.1.in, %omp.inner.for.inc ]
  %t.1.le = trunc i64 %t.1.in.lcssa to i32
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.loop.exit.loopexit, %DIR.OMP.SIMD.134
  %t.036 = phi i32 [ 2, %DIR.OMP.SIMD.134 ], [ %t.1.le, %omp.loop.exit.loopexit ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %t.2 = phi i32 [ %t.036, %omp.loop.exit ], [ 2, %entry ]
  ret i32 %t.2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1



!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100000_x", !4, i64 0}
!4 = !{!"long long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

