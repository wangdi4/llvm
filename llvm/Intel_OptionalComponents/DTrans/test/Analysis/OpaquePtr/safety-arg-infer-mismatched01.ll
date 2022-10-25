; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that when the type of an argument that is inferred from usage within
; the function as one type which is different than the type the calling
; parameter was used as in the calling function, the paramter is marked as
; "Mismatched argument use"

%struct.hsl = type { double, double, double }
%struct.rgb = type { i8, i8, i8 }

%struct.ident_t = type { i32, i32, i32, i32, i8* }
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }

define i32 @main() {
bb:
  %colors1 = alloca %struct.rgb
  %colors2 = alloca %struct.rgb
  br label %codeRepl

codeRepl:                                         ; preds = %bb
call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...)
@__kmpc_fork_call(%struct.ident_t* nonnull @.kmpc_loc.0.0,
  i32 0,
  void (i32*, i32*, ...)* bitcast (void (i32*, i32*, %struct.rgb*, %struct.rgb*, i64, i64)* @main.DIR.OMP.PARALLEL.LOOP.2.split20 to void (i32*, i32*, ...)*),
  %struct.rgb* nonnull %colors1,
  %struct.rgb* nonnull %colors2,
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
define internal void @main.DIR.OMP.PARALLEL.LOOP.2.split20(i32* %tid, i32* %bid, %struct.rgb* %colors1, %struct.rgb* %colors2, i64 %.omp.lb.val.zext, i64 %.omp.ub.val) {
  %p8a = bitcast %struct.rgb* %colors1 to i8*
  %p8b = bitcast %struct.rgb* %colors2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p8a, i8* %p8b, i64 12, i1 false)
  ret void
}

declare !intel.dtrans.func.type !11 !callback !0 void @__kmpc_fork_call(%struct.ident_t* "intel_dtrans_func_index"="1", i32, void (i32*, i32*, ...)* "intel_dtrans_func_index"="2", ...)
declare !intel.dtrans.func.type !12 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; CHECK: DTRANS_StructInfo:
; CHECK:   LLVMType: %struct.rgb
; CHECK:   Safety data: Local instance | Mismatched argument use
; CHECK:   End LLVMType: %struct.rgb

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
!12 = distinct !{!5, !5}
!13 = !{!"S", %struct.hsl zeroinitializer, i32 3, !2, !2, !2} ; { double, double, double }
!14 = !{!"S", %struct.rgb zeroinitializer, i32 3, !3, !3, !3} ; { i8, i8, i8 }
!15 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !4, !4, !4, !4, !5} ; { i32, i32, i32, i32, i8* }

!intel.dtrans.types = !{!13, !14, !15}
