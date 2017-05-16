; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s -check-prefix=OPT
; REQUIRES: asserts

; Verify VPlan predicator: if with two nested if-else statements.

; region1
; -------
; BB12
;  |
;  v
; loop17
;  |
;  v
; BB13



; loop17
; ------
; BB11
;  |
;  v
; BB2<------+
;  |        |
;  v        |
; region18  |
;  |        |
;  v        |
; BB15------+
;  |
;  v
; BB10




; region18
; --------
;    BB14
;     |   \
;     |    \
;     v     |
; region19  |
;     |     |
;     v     |
;   BB16    |
;    | \    |
;    v  v   |
;  BB8 BB9  |
;    \  |  /
;     v v v
;      BB4



; region19
; --------
; BB3
;  | \
;  v  v
; BB5 BB6
;  |  /
;  v v
; BB7



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

; NOOPT: [[loop_21:loop[0-9]+]]:
; NOOPT:   [[BB_15:BB[0-9]+]]:
; NOOPT:     [[BP_25:BP[0-9]+]] = 
; NOOPT:   [[BB_2:BB[0-9]+]]:
; NOOPT:     [[BP_26:BP[0-9]+]] = [[BP_25]]
; NOOPT:   [[region_22:region[0-9]+]]:
; NOOPT:     [[BP_26]] = [[BP_25]]
; NOOPT:   [[BB_19:BB[0-9]+]]:
; NOOPT:     [[BP_27:BP[0-9]+]] = [[BP_26]]
; NOOPT:   [[BB_14:BB[0-9]+]]:
; NOOPT:     [[BP_28:BP[0-9]+]] = [[BP_25]]

; NOOPT: [[region_22]]:
; NOOPT:   [[BB_18:BB[0-9]+]]:
; NOOPT:     [[BP_29:BP[0-9]+]] = [[BP_26]]
; NOOPT:     [[IfT_35:IfT[0-9]+]] = [[BP_29]] && [[VBR_34:VBR[0-9]+]]
; NOOPT:     [[IfF_39:IfF[0-9]+]] = [[BP_29]] && ![[VBR_34]]
; NOOPT:   [[region_23:region[0-9]+]]:
; NOOPT:     [[IfT_35]] = [[BP_29]] && [[VBR_34]]
; NOOPT:   [[BB_20:BB[0-9]+]]:
; NOOPT:     [[BP_30:BP[0-9]+]] = [[IfT_35]]
; NOOPT:     [[IfF_37:IfF[0-9]+]] = [[BP_30]] && ![[VBR_36:VBR[0-9]+]]
; NOOPT:     [[IfT_38:IfT[0-9]+]] = [[BP_30]] && [[VBR_36]]
; NOOPT:   [[BB_12:BB[0-9]+]]:
; NOOPT:     [[BP_33:BP[0-9]+]] = [[IfF_37]]
; NOOPT:   [[BB_11:BB[0-9]+]]:
; NOOPT:     [[BP_31:BP[0-9]+]] = [[IfT_38]]
; NOOPT:   [[BB_5:BB[0-9]+]]:
; NOOPT:     [[BP_32:BP[0-9]+]] = [[IfF_39]] || [[BP_33]] || [[BP_31]]

; NOOPT: [[region_23]]:
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_40:BP[0-9]+]] = [[IfT_35]]
; NOOPT:     [[IfF_45:IfF[0-9]+]] = [[BP_40]] && ![[VBR_44:VBR[0-9]+]]
; NOOPT:     [[IfT_46:IfT[0-9]+]] = [[BP_40]] && [[VBR_44]]
; NOOPT:   [[BB_8:BB[0-9]+]]:
; NOOPT:     [[BP_43:BP[0-9]+]] = [[IfF_45]]
; NOOPT:   [[BB_7:BB[0-9]+]]:
; NOOPT:     [[BP_41:BP[0-9]+]] = [[IfT_46]]
; NOOPT:   [[BB_9:BB[0-9]+]]:
; NOOPT:     [[BP_42:BP[0-9]+]] = [[BP_43]] || [[BP_41]]



; OPT: [[region_23:region[0-9]+]]:
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT:     [[BP_40:BP[0-9]+]] = [[IfT_35:IfT[0-9]+]]
; OPT:     [[IfF_45:IfF[0-9]+]] = [[BP_40]] && ![[VBR_44:VBR[0-9]+]]
; OPT:     [[IfT_46:IfT[0-9]+]] = [[BP_40]] && [[VBR_44]]
; OPT:   [[BB_8:BB[0-9]+]]:
; OPT:     [[BP_43:BP[0-9]+]] = [[IfF_45]]
; OPT:   [[BB_7:BB[0-9]+]]:
; OPT:     [[BP_41:BP[0-9]+]] = [[IfT_46]]
; OPT:   [[BB_9:BB[0-9]+]]:
; OPT:     [[BP_42:BP[0-9]+]] = [[BP_40]]

; OPT: [[region_22:region[0-9]+]]:
; OPT:   [[BB_18:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:     [[IfT_35]] = [[VBR_34:VBR[0-9]+]]
; OPT:     [[IfF_39:IfF[0-9]+]] = ![[VBR_34]]
; OPT:   [[region_23]]:
; OPT:     [[IfT_35]] = [[VBR_34]]
; OPT:   [[BB_20:BB[0-9]+]]:
; OPT:     [[BP_30:BP[0-9]+]] = [[IfT_35]]
; OPT:     [[IfF_37:IfF[0-9]+]] = [[BP_30]] && ![[VBR_36:VBR[0-9]+]]
; OPT:     [[IfT_38:IfT[0-9]+]] = [[BP_30]] && [[VBR_36]]
; OPT:   [[BB_12:BB[0-9]+]]:
; OPT:     [[BP_33:BP[0-9]+]] = [[IfF_37]]
; OPT:   [[BB_11:BB[0-9]+]]:
; OPT:     [[BP_31:BP[0-9]+]] = [[IfT_38]]
; OPT:   [[BB_5:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =

; OPT: [[loop_21:loop[0-9]+]]:
; OPT:   [[BB_15:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_2:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[region_22]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_19:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =
; OPT:   [[BB_14:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ =

