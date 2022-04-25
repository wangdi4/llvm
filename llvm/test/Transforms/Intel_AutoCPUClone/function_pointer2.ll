; RUN: opt -auto-cpu-clone < %s -S | FileCheck %s
; RUN: opt -passes=auto-cpu-clone < %s -S | FileCheck %s

; This lit test checks that function pointers to multiversioned
; functions are initialized to the correct versions.
;
; Source for this LLVM IR:
; static void empty(void) {}
;
; typedef void (*fptr)(void);
;
; typedef struct {
;   fptr fn;
; } MyStruct;
;
; void foo(MyStruct *S) {
;   S->fn = empty;
; }

; CHECK: define dso_local void @foo.A
; CHECK: store void ()* @empty

; CHECK: define dso_local void @foo.V
; CHECK: store void ()* @empty

; CHECK: define dso_local void @foo.a
; CHECK: store void ()* @empty

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MyStruct = type { void ()* }

; Function Attrs: nounwind uwtable
define dso_local void @foo(%struct.MyStruct* noundef %S) #0 !llvm.auto.cpu.dispatch !3 {
entry:
  %S.addr = alloca %struct.MyStruct*, align 8
  store %struct.MyStruct* %S, %struct.MyStruct** %S.addr, align 8, !tbaa !6
  %0 = load %struct.MyStruct*, %struct.MyStruct** %S.addr, align 8, !tbaa !6
  %fn = getelementptr inbounds %struct.MyStruct, %struct.MyStruct* %0, i32 0, i32 0, !intel-tbaa !10
  store void ()* @empty, void ()** %fn, align 8, !tbaa !10
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @empty() #0 !llvm.auto.cpu.dispatch !3 {
entry:
  ret void
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5}
!4 = !{!"auto-cpu-dispatch-target", !"core-avx2"}
!5 = !{!"auto-cpu-dispatch-target", !"skylake-avx512"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSP8MyStruct", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !12, i64 0}
!11 = !{!"struct@", !12, i64 0}
!12 = !{!"pointer@_ZTSPFvvE", !8, i64 0}
