; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s -check-prefix=OPT
; Verify VPlan predicator: two nested if-else statements.

; region1
; -------
; BB11
;  |
;  v
; loop16
;  |
;  v
; BB12


; loop16
; ------
; BB10
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region17 |
;  |       |
;  v       |
; region18 |
;  |       |
;  v       |
; BB15-----+
;  |
;  v
; BB9


; region18
; --------
; BB14
;  | \
;  v  v
; BB6 BB7
;  |  /
;  v v
; BB8


; region17
; --------
; BB13
;  |  \
;  v   v
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
;     if (a[i] > 0) 
;       b[i] = b[i] * 5;
;     else
;       a[i] = a[i] + 5;
; 
;     c[i] = c[i] * N;
; 
;     if (c[i] > 0) 
;       a[i] = c[i] - a[i];
;     else
;       b[i] = a[i] * c[i];
;   }
; }

; ModuleID = 'pred_2x_if_else_noopt.ll'
source_filename = "pred_2x_if_else.c"
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
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add = add nsw i32 %0, 5
  store i32 %add, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = phi i32 [ %add, %if.else ], [ %0, %if.then ]
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx11, align 4
  %mul12 = mul nsw i32 %3, %N
  store i32 %mul12, i32* %arrayidx11, align 4
  %cmp17 = icmp sgt i32 %mul12, 0
  br i1 %cmp17, label %if.then18, label %if.else25

if.then18:                                        ; preds = %if.end
  %sub = sub nsw i32 %mul12, %2
  store i32 %sub, i32* %arrayidx, align 4
  br label %for.inc

if.else25:                                        ; preds = %if.end
  %mul30 = mul nsw i32 %2, %mul12
  %arrayidx32 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %mul30, i32* %arrayidx32, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then18, %if.else25
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











; NOOPT: [[loop_19:loop[0-9]+]]:
; NOOPT:   [[BB_13:BB[0-9]+]]:
; NOOPT:     [[BP_23:BP[0-9]+]] = [[AllOnes_22:AllOnes[0-9]+]]
; NOOPT:   [[BB_2:BB[0-9]+]]:
; NOOPT:     [[BP_24:BP[0-9]+]] = [[BP_23]]
; NOOPT:   [[region_20:region[0-9]+]]:
; NOOPT:     [[BP_24]] = [[BP_23]]
; NOOPT:   [[region_21:region[0-9]+]]:
; NOOPT:     [[BP_24]] = [[BP_23]]
; NOOPT:   [[BB_18:BB[0-9]+]]:
; NOOPT:     [[BP_25:BP[0-9]+]] = [[BP_24]]
; NOOPT:   [[BB_12:BB[0-9]+]]:
; NOOPT:     [[BP_26:BP[0-9]+]] = [[BP_23]]

; NOOPT: [[region_20]]:
; NOOPT:   [[BB_16:BB[0-9]+]]:
; NOOPT:     [[BP_34:BP[0-9]+]] = [[BP_24]]
; NOOPT:     [[IfF_39:IfF[0-9]+]] = [[BP_34]] && ![[VBR_38:VBR[0-9]+]]
; NOOPT:     [[IfT_40:IfT[0-9]+]] = [[BP_34]] && [[VBR_38]]
; NOOPT:   [[BB_5:BB[0-9]+]]:
; NOOPT:     [[BP_37:BP[0-9]+]] = [[IfF_39]]
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_35:BP[0-9]+]] = [[IfT_40]]
; NOOPT:   [[BB_6:BB[0-9]+]]:
; NOOPT:     [[BP_36:BP[0-9]+]] = [[BP_37]] || [[BP_35]]

; NOOPT: [[region_21]]:
; NOOPT:   [[BB_17:BB[0-9]+]]:
; NOOPT:     [[BP_27:BP[0-9]+]] = [[BP_24]]
; NOOPT:     [[IfF_32:IfF[0-9]+]] = [[BP_27]] && ![[VBR_31:VBR[0-9]+]]
; NOOPT:     [[IfT_33:IfT[0-9]+]] = [[BP_27]] && [[VBR_31]]
; NOOPT:   [[BB_9:BB[0-9]+]]:
; NOOPT:     [[BP_30:BP[0-9]+]] = [[IfF_32]]
; NOOPT:   [[BB_8:BB[0-9]+]]:
; NOOPT:     [[BP_28:BP[0-9]+]] = [[IfT_33]]
; NOOPT:   [[BB_10:BB[0-9]+]]:
; NOOPT:     [[BP_29:BP[0-9]+]] = [[BP_30]] || [[BP_28]]



; OPT: [[region_20:region[0-9]+]]:
; OPT:   [[BB_16:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:     [[IfF_39:IfF[0-9]+]] = ![[VBR_38:VBR[0-9]+]]
; OPT:     [[IfT_40:IfT[0-9]+]] = [[VBR_38]]
; OPT:   [[BB_5:BB[0-9]+]]:
; OPT:     [[BP_37:BP[0-9]+]] = [[IfF_39]]
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT:     [[BP_35:BP[0-9]+]] = [[IfT_40]]
; OPT:   [[BB_6:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =

; OPT: [[region_21:region[0-9]+]]:
; OPT:   [[BB_17:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:     [[IfF_32:IfF[0-9]+]] = ![[VBR_31:VBR[0-9]+]]
; OPT:     [[IfT_33:IfT[0-9]+]] = [[VBR_31]]
; OPT:   [[BB_9:BB[0-9]+]]:
; OPT:     [[BP_30:BP[0-9]+]] = [[IfF_32]]
; OPT:   [[BB_8:BB[0-9]+]]:
; OPT:     [[BP_28:BP[0-9]+]] = [[IfT_33]]
; OPT:   [[BB_10:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT: [[loop_19:loop[0-9]+]]:
; OPT:   [[BB_13:BB[0-9]+]]:
; OPT-NOT:  BP[0-9]+ =
; OPT:   [[BB_2:BB[0-9]+]]:
; OPT-NOT:  BP[0-9]+ =
; OPT:   [[region_20]]:
; OPT-NOT:  BP[0-9]+ =
; OPT:   [[region_21]]:
; OPT-NOT:  BP[0-9]+ =
; OPT:   [[BB_18:BB[0-9]+]]:
; OPT-NOT:  BP[0-9]+ =
; OPT:   [[BB_12:BB[0-9]+]]:
; OPT-NOT:  BP[0-9]+ =

