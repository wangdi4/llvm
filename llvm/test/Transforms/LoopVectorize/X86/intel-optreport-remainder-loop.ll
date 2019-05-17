; RUN: opt < %s -loop-vectorize -S 2>&1 | FileCheck %s
; Check that optreport metadata node is not propagated to remainder loop

; CHECK: vector.body:
; CHECK: br {{.*}} label %vector.body, !llvm.loop [[vect:![0-9]+]]
; CHECK: for.body:
; CHECK: br {{.*}} label %for.body, !llvm.loop [[scalar:![0-9]+]]

; CHECK: [[vect]] = distinct !{[[vect]], [[dummy1:![0-9]+]], [[dummy2:![0-9]+]], [[optreport:![0-9]+]], [[width:![0-9]+]]}
; CHECK: [[dummy1]] = !{!"DummyMetadata1"}
; CHECK: [[dummy2]] = !{!"DummyMetadata2"}
; CHECK  [[optreport]] = distinct !{!"llvm.loop.optreport", {{.*}}}
; CHECK: [[width]] = !{!"llvm.loop.isvectorized", i32 1}
; CHECK: [[scalar]] = distinct !{[[scalar]], [[dummy1]], [[dummy2]], [[runtime_unroll:![0-9]+]], [[width]]}
; CHECK: [[runtime_unroll]] = !{!"llvm.loop.unroll.runtime.disable"}

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = external global [255 x i32]

; Function Attrs: nounwind readonly uwtable
define i32 @vect() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %red.05 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [255 x i32], [255 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %red.05
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 255
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !1

for.end:                                          ; preds = %for.body
  ret i32 %add
}

!1 = distinct !{!1, !2, !3, !4}
!2 = !{!"DummyMetadata1"}
!3 = !{!"DummyMetadata2"}
!4 = distinct !{!"llvm.loop.optreport", !5}
!5 = distinct !{!"intel.loop.optreport", !6}
!6 = !{!"intel.optreport.remarks", !7}
!7 = !{!"intel.optreport.remark", !"LLorg: Loop has been completely unrolled"}

