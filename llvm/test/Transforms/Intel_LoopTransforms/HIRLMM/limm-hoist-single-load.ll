; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -hir-lmm -scoped-noalias -print-after=hir-lmm < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,hir-lmm,print<hir>" -aa-pipeline="scoped-noalias-aa" < %s 2>&1 | FileCheck %s
;
; Source code
;int a, b;
;short c;
;int *d;
;void e() {
;  unsigned f = 0;
;  for (; f < 51; f += 3) {
;    a = b;
;    c = *d * *d;
;  }
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>          BEGIN REGION { }
;<14>               %mv.test = &((@a)[0]) >=u &((%0)[0]);
;<15>               %mv.test2 = &((%0)[0]) >=u &((@a)[0]);
;<16>               %mv.and = %mv.test  &  %mv.test2;
;<17>               if (%mv.and == 0)
;<17>               {
;<10>                  + DO i1 = 0, 16, 1   <DO_LOOP>  <MVTag: 10>
;<2>                   |   (@a)[0] = %.pre;
;<3>                   |   %1 = (%0)[0];
;<10>                  + END LOOP
;<17>               }
;<17>               else
;<17>               {
;<11>                  + DO i1 = 0, 16, 1   <DO_LOOP>  <MVTag: 10> <nounroll> <novectorize>
;<12>                  |   (@a)[0] = %.pre;
;<13>                  |   %1 = (%0)[0];
;<11>                  + END LOOP
;<17>               }
;<0>          END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
;CHECK:       BEGIN REGION { modified }
;CHECK:            %mv.test = &((@a)[0]) >=u &((%0)[0]);
;CHECK:            %mv.test2 = &((%0)[0]) >=u &((@a)[0]);
;CHECK:            %mv.and = %mv.test  &  %mv.test2;
;CHECK:            if (%mv.and == 0)
;CHECK:            {
;CHECK:               %1 = (%0)[0];
;CHECK:               (@a)[0] = %.pre;
;CHECK:            }
;CHECK:            else
;CHECK:            {
;CHECK:               + DO i1 = 0, 16, 1   <DO_LOOP>  <MVTag: 10> <nounroll> <novectorize>
;CHECK:               |   (@a)[0] = %.pre;
;CHECK:               |   %1 = (%0)[0];
;CHECK:               + END LOOP
;CHECK:            }
;CHECK:       END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common dso_local local_unnamed_addr global i32 0, align 4
@a = common dso_local local_unnamed_addr global i32 0, align 4
@d = common dso_local local_unnamed_addr global i32* null, align 8
@c = common dso_local local_unnamed_addr global i16 0, align 2

; Function Attrs: norecurse nounwind uwtable
define dso_local void @e() local_unnamed_addr #0 {
entry:
  %0 = load i32*, i32** @d, align 8, !tbaa !2
  %.pre = load i32, i32* @b, align 4, !tbaa !6
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %f.03 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  store i32 %.pre, i32* @a, align 4, !tbaa !6
  %1 = load i32, i32* %0, align 4, !tbaa !6
  %add = add nuw nsw i32 %f.03, 3
  %cmp = icmp ult i32 %add, 51
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  %.lcssa = phi i32 [ %1, %for.body ]
  %mul = mul nsw i32 %.lcssa, %.lcssa
  %conv = trunc i32 %mul to i16
  store i16 %conv, i16* @c, align 2, !tbaa !8
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 59f0a4f3baa133224b131c19ed0a4a18f74d9a89) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 9458e8aef0fe73b527a3daa0b2479284b106e878)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"short", !4, i64 0}
