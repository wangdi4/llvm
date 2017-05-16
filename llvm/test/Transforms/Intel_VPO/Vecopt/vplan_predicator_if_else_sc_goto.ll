; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s -check-prefix=OPT
; REQUIRES: asserts

; Verify the VPlan predicator: if with short-circuit condition (goto)

; region1
; -------
; BB9
;  |
;  v
; loop13
;  |
;  v
; BB10


; loop13
; ------
; BB8
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region14 |
;  |       |
;  v       |
; BB12-----+
;  |
;  v
; BB7



; region14
; --------
; BB11
;  |\
;  v \
; BB3 \
;  | \ \
;  v  v v
; BB5 BB4
;  |  /
;  v v
; BB6



; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)

; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (b[i] <= 100 && a[i] != 0) 
;       b[i] = b[i] * 5;
;     else
;       a[i] = a[i] + 5;
; 
;     c[i] = c[i] * N;
;   }
; }


; ModuleID = 'pred_if_else_sc_goto_noopt.ll'
source_filename = "pred_if_else_sc_goto.c"
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
  %cmp1 = icmp slt i32 %0, 101
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  br i1 %cmp1, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %for.body
  %cmp4 = icmp eq i32 %1, 0
  br i1 %cmp4, label %if.else, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body, %land.lhs.true
  %2 = phi i32 [ 0, %land.lhs.true ], [ %1, %for.body ]
  %add = add nsw i32 %2, 5
  store i32 %add, i32* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %arrayidx14 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx14, align 4
  %mul15 = mul nsw i32 %3, %N
  store i32 %mul15, i32* %arrayidx14, align 4
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

; NOOPT: [[loop_16:loop[0-9]+]]:
; NOOPT:   [[BB_11:BB[0-9]+]]:
; NOOPT:     [[BP_19:BP[0-9]+]] = 
; NOOPT:   [[BB_2:BB[0-9]+]]:
; NOOPT:     [[BP_20:BP[0-9]+]] = [[BP_19]]
; NOOPT:   [[region_17:region[0-9]+]]:
; NOOPT:     [[BP_20]] = [[BP_19]]
; NOOPT:   [[BB_15:BB[0-9]+]]:
; NOOPT:     [[BP_21:BP[0-9]+]] = [[BP_20]]
; NOOPT:   [[BB_10:BB[0-9]+]]:
; NOOPT:     [[BP_22:BP[0-9]+]] = [[BP_19]]

; NOOPT: [[region_17]]:
; NOOPT:   [[BB_14:BB[0-9]+]]:
; NOOPT:     [[BP_23:BP[0-9]+]] = [[BP_20]]
; NOOPT:     [[IfT_29:IfT[0-9]+]] = [[BP_23]] && [[VBR_28:VBR[0-9]+]]
; NOOPT:     [[IfF_32:IfF[0-9]+]] = [[BP_23]] && ![[VBR_28]]
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_24:BP[0-9]+]] = [[IfT_29]]
; NOOPT:     [[IfF_31:IfF[0-9]+]] = [[BP_24]] && ![[VBR_30:VBR[0-9]+]]
; NOOPT:     [[IfT_33:IfT[0-9]+]] = [[BP_24]] && [[VBR_30]]
; NOOPT:   [[BB_7:BB[0-9]+]]:
; NOOPT:     [[BP_27:BP[0-9]+]] = [[IfF_31]]
; NOOPT:   [[BB_5:BB[0-9]+]]:
; NOOPT:     [[BP_25:BP[0-9]+]] = [[IfF_32]] || [[IfT_33]]
; NOOPT:   [[BB_8:BB[0-9]+]]:
; NOOPT:     [[BP_26:BP[0-9]+]] = [[BP_27]] || [[BP_25]]


; OPT: [[region_17:region[0-9]+]]:
; OPT:   [[BB_14:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:     [[IfT_29:IfT[0-9]+]] = [[VBR_28:VBR[0-9]+]]
; OPT:     [[IfF_32:IfF[0-9]+]] = ![[VBR_28]]
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT:     [[BP_24:BP[0-9]+]] = [[IfT_29]]
; OPT:     [[IfF_31:IfF[0-9]+]] = [[BP_24]] && ![[VBR_30:VBR[0-9]+]]
; OPT:     [[IfT_33:IfT[0-9]+]] = [[BP_24]] && [[VBR_30]]
; OPT:   [[BB_7:BB[0-9]+]]:
; OPT:     [[BP_27:BP[0-9]+]] = [[IfF_31]]
; OPT:   [[BB_5:BB[0-9]+]]:
; OPT:     [[BP_25:BP[0-9]+]] = [[IfF_32]] || [[IfT_33]]
; OPT:   [[BB_8:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =

; OPT: [[loop_16:loop[0-9]+]]:
; OPT:   [[BB_11:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_2:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[region_17]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_15:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_10:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =


