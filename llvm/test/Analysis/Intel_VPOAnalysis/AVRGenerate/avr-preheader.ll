; RUN: opt < %s -hir-ssa-deconstruction -hir-framework -hir-dd-analysis -hir-avr-generate -analyze | FileCheck %s

; Check the printing of preheader node for inner loop.

;CHECK: LOOP
;CHECK: bitcast 333 * %a
;CHECK: LOOP


; Source
;__declspec(vector(nomask, uniform(a), uniform(b), uniform(c)))
;void bottom_test(int a, int b, int* c) {
;  int k = 333 * a;
;  for (int i = 0; i < a; ++i) {
;    c[b - i] = a * k + c[b];
;    k += 66666 / k;
;  }
;}
; To Build this LL
;clang -S -O0 -fcilkplus -fms-compatibility -emit-llvm -o avr-preheader.ll test.c
;opt -mem2reg -loop-rotate -loop-simplify -S -o avr-preheader.ll avr-preheader.ll
;opt -vec-clone -S -o bottom_loops-opt-cloned.ll avr-preheader.ll

source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @bottom_test(i32 %a, i32 %b, i32* %c) #0 {
entry:
  %mul = mul nsw i32 333, %a
  %cmp1 = icmp slt i32 0, %a
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %i.03 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %k.02 = phi i32 [ %mul, %for.body.lr.ph ], [ %add4, %for.inc ]
  %mul1 = mul nsw i32 %a, %k.02
  %idxprom = sext i32 %b to i64
  %arrayidx = getelementptr inbounds i32, i32* %c, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %mul1, %0
  %sub = sub nsw i32 %b, %i.03
  %idxprom2 = sext i32 %sub to i64
  %arrayidx3 = getelementptr inbounds i32, i32* %c, i64 %idxprom2
  store i32 %add, i32* %arrayidx3, align 4
  %div = sdiv i32 66666, %k.02
  %add4 = add nsw i32 %k.02, %div
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.03, 1
  %cmp = icmp slt i32 %inc, %a
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define x86_regcallcc void @_ZGVxN4uuu_bottom_test(i32 %a, i32 %b, i32* %c) #0 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  call void @llvm.intel.directive(metadata !8)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !9, i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !10, i32 %a, i32 %b, i32* %c)
  call void @llvm.intel.directive(metadata !11)
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %mul = mul nsw i32 333, %a
  %cmp1 = icmp slt i32 0, %a
  br i1 %cmp1, label %for.body.lr.ph, label %simd.loop.exit

for.body.lr.ph:                                   ; preds = %simd.loop
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %i.03 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %k.02 = phi i32 [ %mul, %for.body.lr.ph ], [ %add4, %for.inc ]
  %mul1 = mul nsw i32 %a, %k.02
  %idxprom = sext i32 %b to i64
  %arrayidx = getelementptr inbounds i32, i32* %c, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %mul1, %0
  %sub = sub nsw i32 %b, %i.03
  %idxprom2 = sext i32 %sub to i64
  %arrayidx3 = getelementptr inbounds i32, i32* %c, i64 %idxprom2
  store i32 %add, i32* %arrayidx3, align 4
  %div = sdiv i32 66666, %k.02
  %add4 = add nsw i32 %k.02, %div
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.03, 1
  %cmp = icmp slt i32 %inc, %a
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %for.cond.for.end_crit_edge, %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !12

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.intel.directive(metadata !14)
  call void @llvm.intel.directive(metadata !11)
  br label %for.end

for.end:                                          ; preds = %simd.end.region
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

attributes #0 = { nounwind uwtable "_ZGVxN4uuu_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!cilk.functions = !{!0}
!llvm.ident = !{!7}

!0 = !{void (i32, i32, i32*)* @bottom_test, !1, !2, !3, !4, !5, !6}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"a", !"b", !"c"}
!3 = !{!"arg_step", i32 0, i32 0, i32 0}
!4 = !{!"arg_alig", i32 undef, i32 undef, i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"mask", i1 false}
!7 = !{!"clang version 3.9.0 (branches/vpo 12286)"}
!8 = !{!"DIR.OMP.SIMD"}
!9 = !{!"QUAL.OMP.SIMDLEN"}
!10 = !{!"QUAL.OMP.UNIFORM"}
!11 = !{!"DIR.QUAL.LIST.END"}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.unroll.disable"}
!14 = !{!"DIR.OMP.END.SIMD"}
