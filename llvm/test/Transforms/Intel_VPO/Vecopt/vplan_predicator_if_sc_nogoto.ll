; RUN: opt %s -VPlanDriver -vplan-predicator-report -disable-vplan-codegen -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -disable-vplan-codegen -S -o /dev/null | FileCheck %s -check-prefix=OPT
; REQUIRES: asserts

; Verify the VPlan predicator: if with "short-circuit" condition, no goto.

; region1
; -------
; BB7
;  |
;  v
; loop11
;  |
;  v
; BB8


; loop11
; ------
; BB6
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region12 |
;  |       |
;  v       |
; BB10-----+
;  |
;  v
; BB5



; region12
; --------
; BB9
;  | \
;  |  v
;  |  BB4
;  |  /
;  | v
; BB3


; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)

; void foo(int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (b[i] <= 100 && b[i] != 0) 
;       b[i] = b[i] * 5;
; 
;     c[i] = c[i] * N;
;   }
; }


; ModuleID = 'pred_if_else_sc_nogoto_noopt.ll'
source_filename = "pred_if_else_sc_nogoto.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture readnone %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N, i32 %M, i32 %K) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 100
  %cmp4 = icmp eq i32 %0, 0
  %or.cond = or i1 %cmp1, %cmp4
  br i1 %or.cond, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %arrayidx10 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx10, align 4
  %mul11 = mul nsw i32 %1, %N
  store i32 %mul11, i32* %arrayidx10, align 4
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

; NOOPT: [[loop_13:loop[0-9]+]]:
; NOOPT:   [[BB_8:BB[0-9]+]]:
; NOOPT:     [[BP_16:BP[0-9]+]] = 
; NOOPT:   [[BB_2:BB[0-9]+]]:
; NOOPT:     [[BP_17:BP[0-9]+]] = [[BP_16]]
; NOOPT:   [[region_14:region[0-9]+]]:
; NOOPT:     [[BP_17]] = [[BP_16]]
; NOOPT:   [[BB_12:BB[0-9]+]]:
; NOOPT:     [[BP_18:BP[0-9]+]] = [[BP_17]]
; NOOPT:   [[BB_7:BB[0-9]+]]:
; NOOPT:     [[BP_19:BP[0-9]+]] = [[BP_16]]

; NOOPT: [[region_14]]:
; NOOPT:   [[BB_11:BB[0-9]+]]:
; NOOPT:     [[BP_20:BP[0-9]+]] = [[BP_17]]
; NOOPT:     [[IfF_24:IfF[0-9]+]] = [[BP_20]] && ![[VBR_23:VBR[0-9]+]]
; NOOPT:     [[IfT_25:IfT[0-9]+]] = [[BP_20]] && [[VBR_23]]
; NOOPT:   [[BB_5:BB[0-9]+]]:
; NOOPT:     [[BP_22:BP[0-9]+]] = [[IfF_24]]
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_21:BP[0-9]+]] = [[BP_22]] || [[IfT_25]] 


; OPT: [[region_14:region[0-9]+]]:
; OPT:   [[BB_11:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:     [[IfF_24:IfF[0-9]+]] = ![[VBR_23:VBR[0-9]+]]
; OPT:     [[IfT_25:IfT[0-9]+]] = [[VBR_23]]
; OPT:   [[BB_5:BB[0-9]+]]:
; OPT:     [[BP_22:BP[0-9]+]] = [[IfF_24]]
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 

; OPT: [[loop_13:loop[0-9]+]]:
; OPT:   [[BB_8:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:   [[BB_2:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[region_14]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_12:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_7:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =


