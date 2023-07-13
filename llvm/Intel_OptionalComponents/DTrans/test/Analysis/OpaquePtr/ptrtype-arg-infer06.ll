; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test for inferring the type of an input argument type on functions
; that lack metadata based on the usage types of the argument. In
; this case the argument is used as multiple types, so will not be
; inferred.

%struct.hsl = type { double, double, double }
%struct.rgb = type { i8, i8, i8 }

%struct.ident_t = type { i32, i32, i32, i32, ptr }
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0, i32 0, i32 0) }

define i32 @main() {
bb:
  %colors = alloca %struct.rgb
  br label %codeRepl

codeRepl:                                         ; preds = %bb
call void (ptr, i32, ptr, ...)
@__kmpc_fork_call(ptr nonnull @.kmpc_loc.0.0,
  i32 0,
  ptr bitcast (ptr @main.DIR.OMP.PARALLEL.LOOP.2.split20 to ptr),
  ptr nonnull %colors,
  i64 0,
  i64 1023)
br label %DIR.OMP.END.PARALLEL.AFTEROMP

DIR.OMP.END.PARALLEL.AFTEROMP:                    ; preds = %codeRepl
  ret i32 0
}

; This function is intentionally defined without DTrans metadata to force the
; the PtrTypeAnalyzer to infer the argument types based on usage.
; The first two input arguments are not used, so do not need to be marked as
; 'Unhandled' when there is no metadata describing them. The %colors argument
; gets used as multiple types. Currently, this will be marked as 'Unhandled',
; but in the future we may just want to mark the types the arguments is used
; as with a DTrans safety flag.
define internal void @main.DIR.OMP.PARALLEL.LOOP.2.split20(ptr %tid, ptr %bid, ptr %colors, i64 %.omp.lb.val.zext, i64 %.omp.ub.val) {
  %r = getelementptr %struct.rgb, ptr %colors, i64 0, i32 0
  %g = getelementptr %struct.rgb, ptr %colors, i64 0, i32 1
  %b = getelementptr %struct.rgb, ptr %colors, i64 0, i32 2
  %cast = bitcast ptr %colors to ptr
  %h = getelementptr %struct.hsl, ptr %cast, i64 0, i32 0
  %s = getelementptr %struct.hsl, ptr %cast, i64 0, i32 1
  %l = getelementptr %struct.hsl, ptr %cast, i64 0, i32 2
  ret void
}

; CHECK: Input Parameters: main.DIR.OMP.PARALLEL.LOOP.2.split20
; CHECK:    Arg 0: ptr %tid
; CHECK:   LocalPointerInfo:
; CHECK-NOT: <UNHANDLED>
; CHECK:    Arg 1: ptr %bid
; CHECK:   LocalPointerInfo:
; CHECK-NOT: <UNHANDLED>
; CHECK:    Arg 2: ptr %colors
; CHECK:   LocalPointerInfo:
; CHECK-SAME: <UNHANDLED>
; CHECK: define internal void @main.DIR.OMP.PARALLEL.LOOP.2.split20

; CHECK:    %r = getelementptr %struct.rgb, ptr %colors, i64 0, i32 0
; CHECK:   LocalPointerInfo:
; CHECK-SAME: <DEPENDS ON UNHANDLED>

; CHECK:    %g = getelementptr %struct.rgb, ptr %colors, i64 0, i32 1
; CHECK:   LocalPointerInfo:
; CHECK-SAME: <DEPENDS ON UNHANDLED>

; CHECK:    %b = getelementptr %struct.rgb, ptr %colors, i64 0, i32 2
; CHECK:   LocalPointerInfo:
; CHECK-SAME: <DEPENDS ON UNHANDLED>

; CHECK:    %h = getelementptr %struct.hsl, ptr %cast, i64 0, i32 0
; CHECK:   LocalPointerInfo:
; CHECK-SAME: <DEPENDS ON UNHANDLED>

; CHECK:    %s = getelementptr %struct.hsl, ptr %cast, i64 0, i32 1
; CHECK:   LocalPointerInfo:
; CHECK-SAME: <DEPENDS ON UNHANDLED>

; CHECK:    %l = getelementptr %struct.hsl, ptr %cast, i64 0, i32 2
; CHECK:   LocalPointerInfo:
; CHECK-SAME: <DEPENDS ON UNHANDLED>

declare !intel.dtrans.func.type !11 !callback !0 void @__kmpc_fork_call(ptr "intel_dtrans_func_index"="1", i32, ptr "intel_dtrans_func_index"="2", ...)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}

!2 = !{double 0.0e+00, i32 0}  ; double
!3 = !{i8 0, i32 0}  ; i8
!4 = !{i32 0, i32 0}  ; i32
!5 = !{i8 0, i32 1}  ; i8*
!6 = !{%struct.ident_t zeroinitializer, i32 1}  ; %struct.ident_t*
!7 = !{!"F", i1 true, i32 2, !8, !9, !9}  ; void (i32*, i32*, ...)
!8 = !{!"void", i32 0}  ; void
!9 = !{i32 0, i32 1}  ; i32*
!10 = !{!7, i32 1}  ; void (i32*, i32*, ...)*
!11 = distinct !{!6, !10}
!12 = !{!"S", %struct.hsl zeroinitializer, i32 3, !2, !2, !2} ; { double, double, double }
!13 = !{!"S", %struct.rgb zeroinitializer, i32 3, !3, !3, !3} ; { i8, i8, i8 }
!14 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !4, !4, !4, !4, !5} ; { i32, i32, i32, i32, i8* }

!intel.dtrans.types = !{!12, !13, !14}
