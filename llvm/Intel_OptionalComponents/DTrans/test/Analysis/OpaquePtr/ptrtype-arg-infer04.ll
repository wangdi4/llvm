; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test for inferring the type of an input argument type on functions
; that lack metadata based on the usage types of the argument. In
; this case the argument is not used, so will not have an inferred
; type, but should not cause an 'Unhandled' safety condition.

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }


define internal void @_Z9doNothingv() {
  ret void
}

define i32 @main() {
bb:
  br label %codeRepl

codeRepl:                                         ; preds = %bb
  call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(%struct.ident_t* @.kmpc_loc.0.0, i32 0, void (i32*, i32*, ...)* bitcast (void (i32*, i32*)* @main.bb.split to void (i32*, i32*, ...)*))
  br label %DIR.OMP.END.PARALLEL.AFTEROMP

DIR.OMP.END.PARALLEL.AFTEROMP:                    ; preds = %codeRepl
  ret i32 0
}

; This function is intentionally defined without DTrans metadata to force the
; the PtrTypeAnalyzer to infer the argument types based on usage.
; In this case the input arguments are not used, so do not need to be marked as
; 'Unhandled' when there is no metadata describing them.
define internal void @main.bb.split(i32* %tid, i32* %bid) {
newFuncRoot:
  br label %bb.split

DIR.OMP.PARALLEL.START:                           ; preds = %bb.split
  call void @_Z9doNothingv()
  br label %DIR.OMP.END.PARALLEL.EXIT

DIR.OMP.END.PARALLEL.EXIT:                        ; preds = %DIR.OMP.PARALLEL.START
  br label %DIR.OMP.END.PARALLEL.AFTEROMP.exitStub

bb.split:                                         ; preds = %newFuncRoot
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.PARALLEL.START

DIR.OMP.END.PARALLEL.AFTEROMP.exitStub:           ; preds = %DIR.OMP.END.PARALLEL.EXIT
  ret void
}

; CHECK: Input Parameters: main.bb.split
; CHECK-NONOPAQUE: Arg 0: i32* %tid
; CHECK-OPAQUE:    Arg 0: ptr %tid
; CHECK:   LocalPointerInfo:
; CHECK-NOT: <UNHANDLED>
; CHECK-NONOPAQUE: Arg 1: i32* %bid
; CHECK-OPAQUE:    Arg 1: ptr %bid
; CHECK:   LocalPointerInfo:
; CHECK-NOT: <UNHANDLED>
; CHECK: define internal void @main.bb.split

declare !intel.dtrans.func.type !9 !callback !0 void @__kmpc_fork_call(%struct.ident_t* "intel_dtrans_func_index"="1", i32, void (i32*, i32*, ...)* "intel_dtrans_func_index"="2", ...)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{%struct.ident_t zeroinitializer, i32 1}  ; %struct.ident_t*
!5 = !{!"F", i1 true, i32 2, !6, !7, !7}  ; void (i32*, i32*, ...)
!6 = !{!"void", i32 0}  ; void
!7 = !{i32 0, i32 1}  ; i32*
!8 = !{!5, i32 1}  ; void (i32*, i32*, ...)*
!9 = distinct !{!4, !8}
!10 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !2, !2, !2, !2, !3} ; { i32, i32, i32, i32, i8* }

!intel.dtrans.types = !{!10}
