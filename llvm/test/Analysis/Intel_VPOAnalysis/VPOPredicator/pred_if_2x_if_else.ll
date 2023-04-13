; RUN: opt -passes="hir-ssa-deconstruction" < %s -hir-avr-generate -hir-avr-predicate -debug -debug-only=avr-predicate -S -o /dev/null 2>&1| FileCheck %s
; REQUIRES: asserts

; XFAIL: *
; This test did not add -hir-ssa-deconstruction to the command line and hence was constructing incorrect HIR. It triggers  assrtion in the HIR verifier.
; The CHECK directives need to be adjusted to make it work again.

; Verify AVR predicator: if with two nested if-else statements.

; CHECK: Predicator finished:
; CHECK: LOOP
; CHECK: (62) PREDICATE {P62 := }
; CHECK: (3) if /P62/  ((4)EXPR{i1 (5)VALUE{i32 %0} icmp/sgt (6)VALUE{i32 0}})   {
; CHECK: (63) PREDICATE {P63 := (62) && (72)EXPR{i1 (70)VALUE{i1 &(4)} icmp/eq (71)VALUE{i1 true}}}
; CHECK: (7) ASSIGN{/P63/ (26)EXPR{i32 (27)VALUE{i32 %1}} = (28)EXPR{i32 load (29)VALUE{i32* (%a)[i1]}}}
; CHECK: (8) if /P63/  ((9)EXPR{i1 (10)VALUE{i32 %1} icmp/sgt (11)VALUE{i32 0}})   {
; CHECK: (64) PREDICATE {P64 := (63) && (75)EXPR{i1 (73)VALUE{i1 &(9)} icmp/eq (74)VALUE{i1 true}}}
; CHECK: (12) ASSIGN{/P64/ (30)EXPR{i32 (31)VALUE{i32* (%b)[i1]}} = (32)EXPR{i32 store (33)VALUE{i32 5 * %0}}}
; CHECK: (69) PREDICATE {P69 := (63) && (87)EXPR{i1 (85)VALUE{i1 &(9)} icmp/eq (86)VALUE{i1 false}}}
; CHECK: (13) ASSIGN{/P69/ (34)EXPR{i32 (35)VALUE{i32* (%a)[i1]}} = (36)EXPR{i32 store (37)VALUE{i32 %1 + 5}}}
; CHECK: (65) PREDICATE {P65 := (69) || (64)}
; CHECK: (14) ASSIGN{/P65/ (38)EXPR{i32 (39)VALUE{i32 %3}} = (40)EXPR{i32 load (41)VALUE{i32* (%c)[i1]}}}
; CHECK: (15) ASSIGN{/P65/ (42)EXPR{i32 (43)VALUE{i32* (%c)[i1]}} = (44)EXPR{i32 store (45)VALUE{i32 (%N * %3)}}}
; CHECK: (16) if /P65/  ((17)EXPR{i1 (18)VALUE{i32 (%N * %3)} icmp/sgt (19)VALUE{i32 0}})   {
; CHECK: (66) PREDICATE {P66 := (65) && (78)EXPR{i1 (76)VALUE{i1 &(17)} icmp/eq (77)VALUE{i1 true}}}
; CHECK: (20) ASSIGN{/P66/ (46)EXPR{i32 (47)VALUE{i32* (%a)[i1]}} = (48)EXPR{i32 store (49)VALUE{i32 (%N * %3) + -1 * %2}}}
; CHECK: (68) PREDICATE {P68 := (65) && (84)EXPR{i1 (82)VALUE{i1 &(17)} icmp/eq (83)VALUE{i1 false}}}
; CHECK: (21) ASSIGN{/P68/ (50)EXPR{i32 (51)VALUE{i32* (%b)[i1]}} = (52)EXPR{i32 store (53)VALUE{i32 (%N * %3 * %2)}}}
; CHECK: (67) PREDICATE {P67 := (62) && (81)EXPR{i1 (79)VALUE{i1 &(4)} icmp/eq (80)VALUE{i1 false}} || (68) || (66)}


; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)


; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
; 
;     if (b[i] > 0) {
;       if (a[i] > 0) 
;         b[i] = b[i] * 5;
;       else
;         a[i] = a[i] + 5;
; 
;       c[i] = c[i] * N;
; 
;       if (c[i] > 0) 
;         a[i] = c[i] - a[i];
;       else
;         b[i] = a[i] * c[i];
;     }
;   }
; }


; ModuleID = 'pred_if_2x_if_else_noopt.ll'
source_filename = "pred_if_2x_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %1, 0
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %if.then
  %add = add nsw i32 %1, 5
  store i32 %add, i32* %arrayidx3, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then5
  %2 = phi i32 [ %add, %if.else ], [ %1, %if.then5 ]
  %arrayidx15 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx15, align 4
  %mul16 = mul nsw i32 %3, %N
  store i32 %mul16, i32* %arrayidx15, align 4
  %cmp21 = icmp sgt i32 %mul16, 0
  br i1 %cmp21, label %if.then22, label %if.else29

if.then22:                                        ; preds = %if.end
  %sub = sub nsw i32 %mul16, %2
  store i32 %sub, i32* %arrayidx3, align 4
  br label %for.inc

if.else29:                                        ; preds = %if.end
  %mul34 = mul nsw i32 %2, %mul16
  store i32 %mul34, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.else29, %if.then22
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
