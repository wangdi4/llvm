; RUN: opt -basic-aa -tbaa -gvn -S < %s | FileCheck %s

; GVN should be able to eliminate the load out of the loop
; by using both BasicAA and TBAA information.

; CHECK: @foo
; CHECK:        entry:
; CHECK-NEXT:   %m_logcount = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 0
; CHECK-NEXT:   %0 = load i32, i32* %m_logcount, align 4, !tbaa !1
; CHECK-NEXT:   %idxprom = sext i32 %0 to i64
; CHECK-NEXT:   %id = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom, i32 0
; CHECK-NEXT:   store i32 %0, i32* %id, align 4, !tbaa !6
; CHECK-NEXT:   %logitem1 = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom, i32 1
; CHECK-NEXT:   store i32 %arg1, i32* %logitem1, align 4, !tbaa !8
; CHECK-NEXT:   %logitem2 = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom, i32 2
; CHECK-NEXT:   store i32 %arg2, i32* %logitem2, align 4, !tbaa !9
; CHECK-NEXT:   %add = add nsw i32 %0, 111
; CHECK-NEXT:   store i32 %add, i32* %m_logcount, align 4, !tbaa !1
; CHECK-NEXT:   %idxprom10 = sext i32 %add to i64
; CHECK-NEXT:   %id13 = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom10, i32 0
; CHECK-NEXT:   store i32 0, i32* %id13, align 4, !tbaa !6
; CHECK-NEXT:   ret void


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, [100 x %struct.test2] }
%struct.test2 = type { i32, i32, i32 }

; Function Attrs: nounwind uwtable
define void @foo(%struct.test* nocapture %t, i32 %arg1, i32 %arg2) #0 {
entry:
  %m_logcount = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 0
  %0 = load i32, i32* %m_logcount, align 4, !tbaa !1
  %idxprom = sext i32 %0 to i64
  %id = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom, i32 0
  store i32 %0, i32* %id, align 4, !tbaa !6
  %logitem1 = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom, i32 1
  store i32 %arg1, i32* %logitem1, align 4, !tbaa !8
  %logitem2 = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom, i32 2
  store i32 %arg2, i32* %logitem2, align 4, !tbaa !9
  %1 = load i32, i32* %m_logcount, align 4, !tbaa !1
  %add = add nsw i32 %1, 111
  store i32 %add, i32* %m_logcount, align 4, !tbaa !1
  %idxprom10 = sext i32 %add to i64
  %id13 = getelementptr inbounds %struct.test, %struct.test* %t, i64 0, i32 1, i64 %idxprom10, i32 0
  store i32 0, i32* %id13, align 4, !tbaa !6
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1275)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"test", !3, i64 0, !4, i64 4}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"test2", !3, i64 0, !3, i64 4, !3, i64 8}
!8 = !{!7, !3, i64 4}
!9 = !{!7, !3, i64 8}
