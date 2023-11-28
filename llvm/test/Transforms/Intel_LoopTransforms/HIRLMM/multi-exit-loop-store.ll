; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s
;
; This test case checks that HIR LMM couldn't be applied since there is a ref
; to (@b[0]) before an early exit and doesn't dominates it.
;
; [source code]
;int a, c, b;
;int main(){
;  int e = 0;
;  for (; e <= 3; e++) {
;    switch (b) {
;    case 7:
;      b = e;
;      break;
;    case 6:
;      for (;;)
;        ;
;    default:
;      return 0;
;    }
;    if (c) {
;      b = a;
;      continue;
;    }
;    return 0;
;  }
;}
;

;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>          BEGIN REGION { }
;<22>               + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
;<2>                |   switch(%2)
;<2>                |   {
;<2>                |   case 7:
;<7>                |      (@b)[0] = i1;
;<8>                |      if (%0 == 0)
;<8>                |      {
;<9>                |         goto cleanup;
;<8>                |      }
;<2>                |      break;
;<2>                |   case 6:
;<5>                |      goto for.cond2.preheader;
;<2>                |   default:
;<3>                |      goto cleanup;
;<2>                |   }
;<12>               |   (@b)[0] = %1;
;<15>               |   %2 = %1;
;<16>               |   %2 = %1;
;<22>               + END LOOP
;<0>          END REGION


;*** IR Dump After HIR Loop Memory Motion ***
;
; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:       |   switch(%2)
; CHECK:       |   {
; CHECK:       |   case 7:
; CHECK:       |      (@b)[0] = i1;
; CHECK:       |      if (%0 == 0)
; CHECK:       |      {
; CHECK:       |         goto cleanup;
; CHECK:       |      }
; CHECK:       |      break;
; CHECK:       |   case 6:
; CHECK:       |      goto for.cond2.preheader;
; CHECK:       |   default:
; CHECK:       |      goto cleanup;
; CHECK:       |   }
; CHECK:       |   (@b)[0] = %1;
; CHECK:       |   %2 = %1;
; CHECK:       |   %2 = %1;
; CHECK:       + END LOOP
; CHECK: END REGION

;[Notes]
;
; Below is the produced HIR without the fix.
; Comparing with the properly generated code, this codegen is missing 2 conditions on line #109 and #112.
; This is incorrect because the control may exit the loop via any of early exits in the 1st iteration. In this case,
; a store may not have happened yet. Thus it is necessary to guard against the very 1st iteration.
;
;
;        BEGIN REGION { modified }
;             + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
;             |   switch(%2)
;             |   {
;             |   case 7:
;             |      %limm = i1;
;             |      if (%0 == 0)
;             |      {
;             |         (@b)[0] = %limm;
;             |         goto cleanup;
;             |      }
;             |      break;
;             |   case 6:
;             |      (@b)[0] = %limm;
;             |      goto for.cond2.preheader;
;             |   default:
;             |      (@b)[0] = %limm;
;             |      goto cleanup;
;             |   }
;             |   %limm = %1;
;             |   %2 = %1;
;             |   %2 = %1;
;             + END LOOP
;                (@b)[0] = %limm;
;       END REGION

; We can expand the analysis process to generate the following result.

; BEGIN REGION { modified }
;          %limm = 0;
;       + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
;       |   switch(%2)
;       |   {
;       |   case 7:
;       |      %limm = i1;
;       |      if (%0 == 0)
;       |      {
;       |         (@b)[0] = %limm;
;       |         goto cleanup;
;       |      }
;       |      break;
;       |   case 6:
;       |      if (i1 != 0)
;       |      {
;       |         (@b)[0] = %limm;
;       |      }
;       |      goto for.cond2.preheader;
;       |   default:
;       |      if (i1 != 0)
;       |      {
;       |         (@b)[0] = %limm;
;       |      }
;       |      goto cleanup;
;       |   }
;       |   %limm = %1;
;       |   %2 = %1;
;       |   %2 = %1;
;       + END LOOP
;          (@b)[0] = %limm;
; END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr @c, align 4
  %tobool = icmp eq i32 %0, 0
  %1 = load i32, ptr @a, align 4
  %.pre = load i32, ptr @b, align 4, !tbaa !2
  br label %for.body

for.body:                                         ; preds = %entry, %if.then
  %2 = phi i32 [ %.pre, %entry ], [ %1, %if.then ], !in.de.ssa !6
  %e.06 = phi i32 [ 0, %entry ], [ %inc, %if.then ], !in.de.ssa !7
  switch i32 %2, label %cleanup [
    i32 7, label %sw.bb
    i32 6, label %for.cond2.preheader
  ]

for.cond2.preheader:                              ; preds = %for.body
  br label %for.cond2

sw.bb:                                            ; preds = %for.body
  store i32 %e.06, ptr @b, align 4, !tbaa !2
  br i1 %tobool, label %cleanup, label %if.then

for.cond2:                                        ; preds = %for.cond2.preheader, %for.cond2
  br label %for.cond2

if.then:                                          ; preds = %sw.bb
  store i32 %1, ptr @b, align 4, !tbaa !2
  %inc = add nuw nsw i32 %e.06, 1
  %exitcond = icmp eq i32 %inc, 4
  %hir.de.ssa.copy0.in = call i32 @llvm.ssa.copy.i32(i32 %1), !in.de.ssa !6
  %e.06.in = call i32 @llvm.ssa.copy.i32(i32 %inc), !in.de.ssa !7
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          ; preds = %if.then, %sw.bb, %for.body
  ret i32 0
}

; Function Attrs: nounwind readnone
declare i32 @llvm.ssa.copy.i32(i32 returned %0) #1

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!"hir.de.ssa.copy0.de.ssa"}
!7 = !{!"e.06.de.ssa"}
