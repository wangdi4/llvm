; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s -check-prefix=OPT
; REQUIRES: asserts

; Verify VPlan predicator: simple if-else statement.

; region1
; -------
; BB8
;  |
;  v
; loop12
;  |
;  v
; BB9


; loop12
; ------
; BB7
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region13 |
;  |       |
;  v       |
; BB11-----+
;  |
;  v
; BB6


; region13
; --------
; BB10
;  | \
;  v  v
; BB3 BB4
;  |  /
;  v v
; BB5



; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)

; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (a[i] == 0)
;       b[i] = b[i] * 5;
;     else
;       a[i] = a[i] + 5;
; 
;     c[i] = c[i] * N;
;   }
; }


; ModuleID = 'pred_if_else.ll'
source_filename = "pred_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N, i32 %M, i32 %K) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %add = add nsw i32 %0, 5
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx11, align 4
  %mul12 = mul nsw i32 %2, %N
  store i32 %mul12, i32* %arrayidx11, align 4
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


; NOOPT: [[loop_14:loop[0-9]+]]:
; NOOPT:   [[BB_9:BB[0-9]+]]:
; NOOPT:     [[BP_17:BP[0-9]+]] = [[AllOnes_16:AllOnes[0-9]+]]
; NOOPT:   [[BB_2:BB[0-9]+]]:
; NOOPT:     [[BP_18:BP[0-9]+]] = [[BP_17]]
; NOOPT:   [[region_15:region[0-9]+]]:
; NOOPT:     [[BP_18]] = [[BP_17]]
; NOOPT:   [[BB_13:BB[0-9]+]]:
; NOOPT:     [[BP_19:BP[0-9]+]] = [[BP_18]]
; NOOPT:   [[BB_8:BB[0-9]+]]:
; NOOPT:     [[BP_20:BP[0-9]+]] = [[BP_17]]

; NOOPT: [[region_15]]:
; NOOPT:   [[BB_12:BB[0-9]+]]:
; NOOPT:     [[BP_21:BP[0-9]+]] = [[BP_18]]
; NOOPT:     [[IfF_26:IfF[0-9]+]] = [[BP_21]] && ![[VBR_25:VBR[0-9]+]]
; NOOPT:     [[IfT_27:IfT[0-9]+]] = [[BP_21]] && [[VBR_25]]
; NOOPT:   [[BB_5:BB[0-9]+]]:
; NOOPT:     [[BP_24:BP[0-9]+]] = [[IfF_26]]
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_22:BP[0-9]+]] = [[IfT_27]]
; NOOPT:   [[BB_6:BB[0-9]+]]:
; NOOPT:     [[BP_23:BP[0-9]+]] = [[BP_24]] || [[BP_22]]



; OPT: [[region_15:region[0-9]+]]:
; OPT:   [[BB_12:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:     [[IfF_26:IfF[0-9]+]] = ![[VBR_25:VBR[0-9]+]]
; OPT:     [[IfT_27:IfT[0-9]+]] = [[VBR_25]]
; OPT:   [[BB_5:BB[0-9]+]]:
; OPT:     [[BP_24:BP[0-9]+]] = [[IfF_26]]
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT:     [[BP_22:BP[0-9]+]] = [[IfT_27]]
; OPT:   [[BB_6:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 

; OPT: [[loop_14:loop[0-9]+]]:
; OPT:   [[BB_9:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:   [[BB_2:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:   [[region_15]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:   [[BB_13:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:   [[BB_8:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 


