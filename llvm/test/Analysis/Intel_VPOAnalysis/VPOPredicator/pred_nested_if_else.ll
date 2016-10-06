; REQUIRES: asserts
; RUN: opt < %s -hir-avr-generate -avr-predicate -debug -debug-only=avr-predicate -S -o /dev/null 2>&1| FileCheck %s

; Verify the AVR predicator: nested if-else statements.


; CHECK: Predicator finished:
; CHECK: LOOP
; CHECK: (2) ASSIGN{(23)EXPR{i32 (24)VALUE{i32 %0}} = (25)EXPR{i32 load (26)VALUE{i32* {al:4}(%a)[i1]}}}
; CHECK: (67) PREDICATE {P67 := }
; CHECK: (3) if /P67/  ((4)EXPR{i1 (5)VALUE{i32 %0} icmp/eq (6)VALUE{i32 0}})   {
; CHECK: }
; CHECK: (68) PREDICATE {P68 := (67) && (77)EXPR{i1 (75)VALUE{i1 &(4)} icmp/eq (76)VALUE{i1 true}}}
; CHECK: (7) ASSIGN{/P68/ (27)EXPR{i32 (28)VALUE{i32 %1}} = (29)EXPR{i32 load (30)VALUE{i32* {al:4}(%b)[i1]}}}
; CHECK: (8) if /P68/  ((9)EXPR{i1 (10)VALUE{i32 %1} icmp/sgt (11)VALUE{i32 5}})   {
; CHECK: }
; CHECK: (69) PREDICATE {P69 := (68) && (80)EXPR{i1 (78)VALUE{i1 &(9)} icmp/eq (79)VALUE{i1 true}}}
; CHECK: (12) ASSIGN{/P69/ (31)EXPR{i32 (32)VALUE{i32* {al:4}(%b)[i1]}} = (33)EXPR{i32 store (34)VALUE{i32 5 * %1}}}
; CHECK: (71) PREDICATE {P71 := (68) && (83)EXPR{i1 (81)VALUE{i1 &(9)} icmp/eq (82)VALUE{i1 false}}}
; CHECK: (13) ASSIGN{/P71/ (35)EXPR{i32 (36)VALUE{i32 %2}} = (37)EXPR{i32 load (38)VALUE{i32* {al:4}(%c)[i1]}}}
; CHECK: (14) ASSIGN{/P71/ (39)EXPR{i32 (40)VALUE{i32* {al:4}(%b)[i1]}} = (41)EXPR{i32 store (42)VALUE{i32 %1 + %2}}}
; CHECK: (72) PREDICATE {P72 := (67) && (86)EXPR{i1 (84)VALUE{i1 &(4)} icmp/eq (85)VALUE{i1 false}}}
; CHECK: (15) if /P72/  ((16)EXPR{i1 (17)VALUE{i32 %0} icmp/sgt (18)VALUE{i32 100}})   {
; CHECK: }
; CHECK: (73) PREDICATE {P73 := (72) && (89)EXPR{i1 (87)VALUE{i1 &(16)} icmp/eq (88)VALUE{i1 true}}}
; CHECK: (19) ASSIGN{/P73/ (43)EXPR{i32 (44)VALUE{i32 %3}} = (45)EXPR{i32 load (46)VALUE{i32* {al:4}(%b)[i1]}}}
; CHECK: (20) ASSIGN{/P73/ (47)EXPR{i32 (48)VALUE{i32* {al:4}(%a)[i1]}} = (49)EXPR{i32 store (50)VALUE{i32 %0 + -1 * %3}}}
; CHECK: (74) PREDICATE {P74 := (72) && (92)EXPR{i1 (90)VALUE{i1 &(16)} icmp/eq (91)VALUE{i1 false}}}
; CHECK: (21) ASSIGN{/P74/ (51)EXPR{i32 (52)VALUE{i32 %4}} = (53)EXPR{i32 load (54)VALUE{i32* {al:4}(%c)[i1]}}}
; CHECK: (22) ASSIGN{/P74/ (55)EXPR{i32 (56)VALUE{i32* {al:4}(%a)[i1]}} = (57)EXPR{i32 store (58)VALUE{i32 (%0 * %4)}}}
; CHECK: (70) PREDICATE {P70 := (74) || (73) || (71) || (69)}


; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)


; void foo(int * restrict a, int * restrict b, int * restrict c, int N, int M, int K)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (a[i] == 0) 
;       if (b[i] > 5)
;         b[i] = b[i] * 5;
;       else
;         b[i] = c[i] + b[i];
;     else
;       if (a[i] > 100)
;         a[i] = a[i] - b[i];
;       else
;         a[i] = a[i] * c[i];
;   }
; }


; ModuleID = 'pred_nested_if_else_noopt.ll'
source_filename = "pred_nested_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture readonly %c, i32 %N, i32 %M, i32 %K) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else16

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %1, 5
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %for.inc

if.else:                                          ; preds = %if.then
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx11, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %arrayidx3, align 4
  br label %for.inc

if.else16:                                        ; preds = %for.body
  %cmp19 = icmp sgt i32 %0, 100
  br i1 %cmp19, label %if.then20, label %if.else27

if.then20:                                        ; preds = %if.else16
  %arrayidx24 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx24, align 4
  %sub = sub nsw i32 %0, %3
  store i32 %sub, i32* %arrayidx, align 4
  br label %for.inc

if.else27:                                        ; preds = %if.else16
  %arrayidx31 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx31, align 4
  %mul32 = mul nsw i32 %4, %0
  store i32 %mul32, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then5, %if.else27, %if.then20
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (branches/vpo 20354) (llvm/branches/vpo 20401)"}
