; Test basic uniform if
; 
; opt < %s -analyze -slev | FileCheck -check-prefix=LLVM %s
; RUN: opt < %s -analyze -slev-hir | FileCheck -check-prefix=HIR %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; HIR: (4)VALUE{i32 %K} ===> {0|UNIFORM|AVR-4}
; HIR: (5)VALUE{i32 50} ===> {1|CONSTANT<50>|AVR-5}
; HIR: (3)EXPR{(4) icmp/slt (5)} ===> {2|UNIFORM|AVR-3|BC}
; HIR: (16)VALUE{i32* %b} ===> {3|UNIFORM|AVR-16}
; HIR: (17)VALUE{i64 i1} ===> {4|STRIDED<1>|AVR-17}
; HIR: (18)EXPR{(16) getelementptr (17)} ===> {5|STRIDED<1>|AVR-18}
; HIR: (11)(18)EXPR{i32* (16)VALUE{i32* %b} getelementptr (17)VALUE{i64 i1}}} ===> {6|STRIDED<1>|AVR-11}
; HIR: (10)EXPR{load (11)} ===> {7|RANDOM|AVR-10}
; HIR: (20)VALUE{i32 5} ===> {8|CONSTANT<5>|AVR-20}
; HIR: (19)VALUE{i32 %0} ===> {9|RANDOM|AVR-19}
; HIR: (21)EXPR{(20) mul (19)} ===> {10|RANDOM|AVR-21}
; HIR: (15)(21)EXPR{i32 (20)VALUE{i32 5} mul (19)VALUE{i32 %0}}} ===> {11|RANDOM|AVR-15}
; HIR: (14)EXPR{store (15)} ===> {12|RANDOM|AVR-14}
; HIR: (22)VALUE{i32* %b} ===> {13|UNIFORM|AVR-22}
; HIR: (23)VALUE{i64 i1} ===> {14|STRIDED<1>|AVR-23}
; HIR: (24)EXPR{(22) getelementptr (23)} ===> {15|STRIDED<1>|AVR-24}
; HIR: (13)(24)EXPR{i32* (22)VALUE{i32* %b} getelementptr (23)VALUE{i64 i1}}} ===> {16|STRIDED<1>|AVR-13}

; ModuleID = 'basic_uniform_if_noopt.ll'
source_filename = "basic_uniform_if.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %b, i32* noalias nocapture readnone %c, i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %cmp = icmp eq i32 %N, %K
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %cmp2 = icmp slt i32 %K, 50
  br label %for.body

for.body:                                         ; preds = %for.inc, %if.end
  %indvars.iv = phi i64 [ 0, %if.end ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  br i1 %cmp2, label %if.then3, label %for.inc

if.then3:                                         ; preds = %for.body
  %0 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %return

return:                                           ; preds = %entry, %for.end
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (branches/vpo 20412) (llvm/branches/vpo 20421)"}

