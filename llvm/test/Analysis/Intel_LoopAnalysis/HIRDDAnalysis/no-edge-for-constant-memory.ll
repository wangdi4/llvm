; RUN: opt < %s -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze | FileCheck %s 
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s

; Checks that there is no edge created for constant memory
; HIR-
; BEGIN REGION { }
;       + DO i1 = 0, 3, 1   <DO_LOOP>
;       |   if ((@convolutionalEncode.135.clone.0)[0][i1][%t139] != 0)
;       |   {
;       |      %t146 = %t146  ^  (%t4)[0][i1];
;       |      (%t140)[0] = %t146;
;       |   }
;       + END LOOP
; END REGION

; CHECK: %t146 --> %t146 FLOW
; CHECK-NOT: convolutionalEncode.135.clone.0

; ModuleID = 't_run_test.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@convolutionalEncode.135.clone.0 = external hidden unnamed_addr constant [4 x [2 x i8]]
; Function Attrs: nounwind uwtable

define hidden void @t_run_test(i64, i32, i8** nocapture readonly, i8* %t18, [8 x i8]* %t4, i64 %t139, i8* %t140) #0 {
header:
  br label %t145

t145:                                    ; preds = %t155, %header
  %t146 = phi i8 [ %t156, %t155 ], [ 0, %header ]
  %t147 = phi i64 [ %t157, %t155 ], [ 0, %header ]
  %t148 = getelementptr inbounds [4 x [2 x i8]], [4 x [2 x i8]]* @convolutionalEncode.135.clone.0, i64 0, i64 %t147, i64 %t139
  %t149 = load i8, i8* %t148, align 1, !tbaa !33
  %t150 = icmp eq i8 %t149, 0
  br i1 %t150, label %t155, label %t151

t151:                                    ; preds = %t145
  %t152 = getelementptr inbounds [8 x i8], [8 x i8]* %t4, i64 0, i64 %t147
  %t153 = load i8, i8* %t152, align 1, !tbaa !30
  %t154 = xor i8 %t146, %t153
  store i8 %t154, i8* %t140, align 1, !tbaa !32
  br label %t155

t155:                                    ; preds = %t151, %t145
  %t156 = phi i8 [ %t154, %t151 ], [ %t146, %t145 ]
  %t157 = add nuw nsw i64 %t147, 1
  %t158 = icmp eq i64 %t157, 4
  br i1 %t158, label %t159, label %t145

t159:                                    ; preds = %t155, %t151, %t145
  ret void
}

!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!30 = !{!31, !5, i64 0}
!31 = !{!"array@_ZTSA8_h", !5, i64 0}
!32 = !{!5, !5, i64 0}
!33 = !{!34, !5, i64 0}
!34 = !{!"array@_ZTSA2_h", !5, i64 0}

