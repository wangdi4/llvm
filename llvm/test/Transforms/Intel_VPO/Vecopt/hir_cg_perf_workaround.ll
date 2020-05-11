; LLVM IR generated from the following testcase using icx -O1 -S -emit-llvm
; struct S1 {
;   unsigned short a1;
;   unsigned short a2;
;   unsigned short a3;
;   unsigned short a4;
; } *s1p;
; 
; double foo(unsigned n, double d)
; {
;   int i1;
; 
;   double sum = 0;
;   for (i1 = 0; i1 < n; i1++) {
;     sum +=    s1p[-1 * i1].a3 * d;
;     sum +=    s1p[-1 * i1].a1 * d;
;   }
;   
;   return sum;
; }
; ModuleID = 'cg_perf_workaround.c'
; RUN: opt -vplan-force-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -disable-hir-loop-reversal -VPlanDriverHIR -hir-cg -print-after-all -S -enable-blob-coeff-vec -enable-nested-blob-vec  < %s 2>&1 | FileCheck %s
; CHECK: IR Dump After HIR Vec Directive Insertion Pass
; CHECK: llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK: IR Dump After VPlan Vectorization Driver HIR
; CHECK:      DO i1 = 0, {{.*}}, 1   <DO_LOOP>
; CHECK:      END LOOP
; ModuleID = 'cg_perf_workaround.c'
source_filename = "cg_perf_workaround.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S1 = type { i16, i16, i16, i16 }

@s1p = common local_unnamed_addr global %struct.S1* null, align 8

; Function Attrs: noinline norecurse nounwind readonly uwtable
define double @foo(i32 %n, double %d) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %n, 0
  br i1 %cmp18, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %0 = load %struct.S1*, %struct.S1** @s1p, align 8, !tbaa !2
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %sum.020 = phi double [ 0.000000e+00, %for.body.lr.ph ], [ %add9, %for.body ]
  %1 = sub nsw i64 0, %indvars.iv
  %a3 = getelementptr inbounds %struct.S1, %struct.S1* %0, i64 %1, i32 2
  %2 = load i16, i16* %a3, align 2, !tbaa !6
  %conv1 = uitofp i16 %2 to double
  %mul2 = fmul fast double %conv1, %d
  %add = fadd fast double %sum.020, %mul2
  %a1 = getelementptr inbounds %struct.S1, %struct.S1* %0, i64 %1, i32 0
  %3 = load i16, i16* %a1, align 2, !tbaa !9
  %conv7 = uitofp i16 %3 to double
  %mul8 = fmul fast double %conv7, %d
  %add9 = fadd fast double %add, %mul8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %add9.lcssa = phi double [ %add9, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.body, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add9.lcssa, %for.end.loopexit ]
  ret double %sum.0.lcssa
}

attributes #0 = { noinline norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 0b4a517eb9d8148972ed8a9ef18c3616c05891fc) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 394335808fb2249a44d0c32f2e0724d070071064)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"unspecified pointer", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !8, i64 4}
!7 = !{!"struct@S1", !8, i64 0, !8, i64 2, !8, i64 4, !8, i64 6}
!8 = !{!"short", !4, i64 0}
!9 = !{!7, !8, i64 0}
