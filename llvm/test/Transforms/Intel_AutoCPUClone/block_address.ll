; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; The test checks that functions that have indirect goto's are not
; multiversioned
; Source for this LLVM IR:
; void foo () {
;   mark:
;     void *LabelPtr = &&mark;
; }

; CHECK: define dso_local void @_Z3foov()
; CHECK-NOT: @_Z3foov.A()
; CHECK-NOT: @_Z3foov.X()
; CHECK-NOT: @_Z3foov.resolver()


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3foov() #0 !llvm.auto.cpu.dispatch !3 {
entry:
  %LabelPtr = alloca ptr, align 8
  br label %mark

mark:                                             ; preds = %indirectgoto, %entry
  %0 = bitcast ptr %LabelPtr to ptr
  store ptr blockaddress(@_Z3foov, %mark), ptr %LabelPtr, align 8, !tbaa !5
  %1 = bitcast ptr %LabelPtr to ptr
  ret void

indirectgoto:                                     ; No predecessors!
  indirectbr ptr undef, [label %mark]
}

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!""}
!3 = !{!4}
!4 = !{!"auto-cpu-dispatch-target", !"broadwell"}
!5 = !{!6, !6, i64 0}
!6 = !{!"pointer@_ZTSPv", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
