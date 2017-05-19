; RUN: opt %s -VPlanDriver -vplan-predicator-report -disable-vplan-codegen -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -disable-vplan-codegen -S -o /dev/null | FileCheck %s -check-prefix=OPT
; This is to test that the predicator can deal with a live-in condition bit recipe.
; The IR has been hand-modified to force the live-in.

; region1
; ----
;  BB7
;   |
;   v
; loop11
;   |
;   v
;  BB8
;

; loop11
; ------
;   BB6
;    |
;    v
;   BB2 <--+
;    |     |
; region12 |
;    |    F|
;   BB10 --+
;    |T
;    v
;   BB5
;
;

; region12
; --------
;   BB9
;  T| \F
;   |  \
;   |  BB4
;   |  /
;   | /
;   BB3
;



; ModuleID = 'inner_if_else.c'
source_filename = "inner_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %entry
  %arrayidx.hmm = getelementptr inbounds [1024 x i32], [1024 x i32]* @B, i64 0, i64 0
  %hmm = load i32, i32* %arrayidx.hmm, align 4, !tbaa !1
  ; Live-In
  %cmp1 = icmp sgt i32 %hmm, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.4

DIR.QUAL.LIST.END.4:                              ; preds = %for.cond.cleanup
  ret i32 0

for.body:                                         ; preds = %if.end, %DIR.QUAL.LIST.END.2
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx5 = getelementptr inbounds [1024 x i32], [1024 x i32]* @C, i64 0, i64 %indvars.iv
  br i1 %cmp1, label %if.end, label %if.else

if.else:                                          ; preds = %for.body
  %1 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  %add10 = add i32 %add, %1
  br label %if.end

if.end:                                           ; preds = %for.body, %if.else
  %storemerge = phi i32 [ %add10, %if.else ], [ %0, %for.body ]
  store i32 %storemerge, i32* %arrayidx5, align 4, !tbaa !1
  %add17 = add nsw i32 %storemerge, %0
  %arrayidx19 = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 %add17, i32* %arrayidx19, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20877)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1024_i", !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

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
; NOOPT:     [[IfF_24:IfF[0-9]+]] = [[BP_20]] && ![[UBR_23:UBR[0-9]+]]
; NOOPT:     [[IfT_25:IfT[0-9]+]] = [[BP_20]] && [[UBR_23]]
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_22:BP[0-9]+]] = [[IfF_24]]
; NOOPT:   [[BB_3:BB[0-9]+]]:
; NOOPT:     [[BP_21:BP[0-9]+]] = [[IfT_25]] || [[BP_22]]



; OPT: [[region_14:region[0-9]+]]:
; OPT:   [[BB_11:BB[0-9]+]]:
; OPT-NOT: BP[0-9]+ = 
; OPT:     [[IfF_24:IfF[0-9]+]] = ![[UBR_23:UBR[0-9]+]]
; OPT:     [[IfT_25:IfT[0-9]+]] = [[UBR_23]]
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT:     [[BP_22:BP[0-9]+]] = [[IfF_24]]
; OPT:   [[BB_3:BB[0-9]+]]:
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


