; RUN: opt %s -VPlanDriver -vplan-predicator-report -S -o /dev/null 2>&1 | FileCheck %s -check-prefix=REPORT
; RUN: opt %s -VPlanDriver -S 2>&1 | FileCheck %s -check-prefix=CG
; REQUIRES: asserts

; This is to test that the predicator preserves uniform control flow.


; Predicator report output
; REPORT: [[region_14:region[0-9]+]]:
; REPORT-NEXT:   [[BB_11:BB[0-9]+]]:
; REPORT-NOT:     [[BP_20:BP[0-9]+]] =
; REPORT-NEXT:     [[IfF_24:IfF[0-9]+]] = {{!UBR[0-9]+}}
; REPORT-NEXT:     [[IfT_25:IfT[0-9]+]] = {{UBR[0-9]+}}
; REPORT-NEXT:   [[BB_4:BB[0-9]+]]:
; REPORT-NOT:     [[BP_22:BP[0-9]+]] = 
; REPORT-NEXT:   [[BB_3:BB[0-9]+]]:
; REPORT-NOT:     [[BP_21:BP[0-9]+]] = 

; REPORT: [[loop_13:loop[0-9]+]]:
; REPORT-NEXT:   [[BB_8:BB[0-9]+]]:
; REPORT-NOT:     [[BP_16:BP[0-9]+]] = 
; REPORT-NEXT:   [[BB_2:BB[0-9]+]]:
; REPORT-NOT:     [[BP_17:BP[0-9]+]] = 
; REPORT-NEXT:   [[region_14]]:
; REPORT-NOT:     [[BP_17]] = 
; REPORT-NEXT:   [[BB_12:BB[0-9]+]]:
; REPORT-NOT:     [[BP_18:BP[0-9]+]] =
; REPORT-NEXT:   [[BB_7:BB[0-9]+]]:
; REPORT-NOT:     [[BP_19:BP[0-9]+]] =

; Codegen

; CG: vector.body:
; CG: br i1 {{%cmp[0-9]+}}, label %[[VPBB2:VPlannedBB[0-9]*]], label %[[VPBB1:VPlannedBB[0-9]*]]
; CG: [[VPBB1]]: 
; CG: br label %[[VPBB2]]
; CG: [[VPBB2]]: 
; CG: br i1 {{%[0-9]+}}, label %middle.block, label %vector.body


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



