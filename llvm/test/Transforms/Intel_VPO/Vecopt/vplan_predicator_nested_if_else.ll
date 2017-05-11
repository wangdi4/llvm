; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s -check-prefix=OPT
; REQUIRES: asserts

; Verify the VPlan predicator: nested if-else statements.

; region1
; -------
; BB12
;  |
;  v
; loop16
;  |
;  v
; BB13



; loop16
; ------
; BB11
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region17 |
;  |       |
;  v       |
; BB15-----+
;  |
;  v
; BB10


; region17
; --------
;       BB14
;      /    \
;     v      v
;   BB3      BB4
;   / \     /  \
;  v   v   v    v
; BB8 BB9 BB5  BB6
;   \   | |   /
;    \  | |  /
;     v v v v
;       BB7


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

; NOOPT: [[loop_20:loop[0-9]+]]:
; NOOPT:   [[BB_15:BB[0-9]+]]:
; NOOPT:     [[BP_23:BP[0-9]+]] = [[AllOnes_22:AllOnes[0-9]+]]
; NOOPT:   [[BB_2:BB[0-9]+]]:
; NOOPT:     [[BP_24:BP[0-9]+]] = [[BP_23]]
; NOOPT:   [[region_21:region[0-9]+]]:
; NOOPT:     [[BP_24]] = [[BP_23]]
; NOOPT:   [[BB_19:BB[0-9]+]]:
; NOOPT:     [[BP_25:BP[0-9]+]] = [[BP_24]]
; NOOPT:   [[BB_14:BB[0-9]+]]:
; NOOPT:     [[BP_26:BP[0-9]+]] = [[BP_23]]

; NOOPT: [[region_21]]:
; NOOPT:   [[BB_18:BB[0-9]+]]:
; NOOPT:     [[BP_27:BP[0-9]+]] = [[BP_24]]
; NOOPT:     [[IfF_36:IfF[0-9]+]] = [[BP_27]] && ![[VBR_35:VBR[0-9]+]]
; NOOPT:     [[IfT_40:IfT[0-9]+]] = [[BP_27]] && [[VBR_35]]
; NOOPT:   [[BB_5:BB[0-9]+]]:
; NOOPT:     [[BP_32:BP[0-9]+]] = [[IfF_36]]
; NOOPT:     [[IfF_38:IfF[0-9]+]] = [[BP_32]] && ![[VBR_37:VBR[0-9]+]]
; NOOPT:     [[IfT_39:IfT[0-9]+]] = [[BP_32]] && [[VBR_37]]
; NOOPT:   [[BB_8:BB[0-9]+]]:
; NOOPT:     [[BP_34:BP[0-9]+]] = [[IfF_38]]
; NOOPT:   [[BB_7:BB[0-9]+]]:
; NOOPT:     [[BP_33:BP[0-9]+]] = [[IfT_39]]
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_28:BP[0-9]+]] = [[IfT_40]]
; NOOPT:     [[IfF_42:IfF[0-9]+]] = [[BP_28]] && ![[VBR_41:VBR[0-9]+]]
; NOOPT:     [[IfT_43:IfT[0-9]+]] = [[BP_28]] && [[VBR_41]]
; NOOPT:   [[BB_12:BB[0-9]+]]:
; NOOPT:     [[BP_31:BP[0-9]+]] = [[IfF_42]]
; NOOPT:   [[BB_11:BB[0-9]+]]:
; NOOPT:     [[BP_29:BP[0-9]+]] = [[IfT_43]]
; NOOPT:   [[BB_9:BB[0-9]+]]:
; NOOPT:     [[BP_30:BP[0-9]+]] = [[BP_34]] || [[BP_33]] || [[BP_31]] || [[BP_29]]



; OPT: [[region_21:region[0-9]+]]:
; OPT:   [[BB_18:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+
; OPT:     [[IfF_36:IfF[0-9]+]] = ![[VBR_35:VBR[0-9]+]]
; OPT:     [[IfT_40:IfT[0-9]+]] = [[VBR_35]]
; OPT:   [[BB_5:BB[0-9]+]]:
; OPT:     [[BP_32:BP[0-9]+]] = [[IfF_36]]
; OPT:     [[IfF_38:IfF[0-9]+]] = [[BP_32]] && ![[VBR_37:VBR[0-9]+]]
; OPT:     [[IfT_39:IfT[0-9]+]] = [[BP_32]] && [[VBR_37]]
; OPT:   [[BB_8:BB[0-9]+]]:
; OPT:     [[BP_34:BP[0-9]+]] = [[IfF_38]]
; OPT:   [[BB_7:BB[0-9]+]]:
; OPT:     [[BP_33:BP[0-9]+]] = [[IfT_39]]
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT:     [[BP_28:BP[0-9]+]] = [[IfT_40]]
; OPT:     [[IfF_42:IfF[0-9]+]] = [[BP_28]] && ![[VBR_41:VBR[0-9]+]]
; OPT:     [[IfT_43:IfT[0-9]+]] = [[BP_28]] && [[VBR_41]]
; OPT:   [[BB_12:BB[0-9]+]]:
; OPT:     [[BP_31:BP[0-9]+]] = [[IfF_42]]
; OPT:   [[BB_11:BB[0-9]+]]:
; OPT:     [[BP_29:BP[0-9]+]] = [[IfT_43]]
; OPT:   [[BB_9:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+

; OPT: [[loop_20:loop[0-9]+]]:
; OPT:   [[BB_15:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+
; OPT:   [[BB_2:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+
; OPT:   [[region_21]]:
; OPT-NOT: BP[0-9]+
; OPT:   [[BB_19:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+
; OPT:   [[BB_14:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+

