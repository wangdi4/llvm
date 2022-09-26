; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt < %s -S -o - -whole-program-assume -dtrans-resolvetypes 2>&1 | FileCheck %s
; RUN:  opt < %s -S -o - -whole-program-assume -passes=dtrans-resolvetypes 2>&1 | FileCheck %s

; This test checks that resolve types pass works correctly when a type is used with
; a llvm.intel.subscript function. The types must be merged and the function name
; must be renamed to use the new type's name.

%"QNCA_a0$double*$rank3$" = type { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank3$.7.39" = type { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

%"QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64 }] }
%"QNCA_a0$double*$rank2$.7.39" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64 }] }

%"OUTER_TYPE" = type {%"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank2$"}

%"QNCA_a0$%\22OUTER_TYPE\22*$rank1$" = type { %"OUTER_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@glob_dope_vector = internal global %"QNCA_a0$%\22OUTER_TYPE\22*$rank1$" { %"OUTER_TYPE"* null, i64 0, i64 0, i64 1342177408, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }

define internal void @bar(%"QNCA_a0$double*$rank3$.7.39"* %dv1, %"QNCA_a0$double*$rank2$.7.39"* %dv2) {
  %temp0 = getelementptr inbounds %"QNCA_a0$double*$rank3$.7.39", %"QNCA_a0$double*$rank3$.7.39"* %dv1, i64 0, i32 0
  %temp1 = load double*, double** %temp0
  %temp2 = getelementptr inbounds %"QNCA_a0$double*$rank2$.7.39", %"QNCA_a0$double*$rank2$.7.39"* %dv2, i64 0, i32 0
  %temp3 = load double*, double** %temp2
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  %temp0 = load %"OUTER_TYPE"*, %"OUTER_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22OUTER_TYPE\22*$rank1$", %"QNCA_a0$%\22OUTER_TYPE\22*$rank1$"* @glob_dope_vector, i64 0, i32 0)
  %temp1 = tail call %"OUTER_TYPE"* @"llvm.intel.subscript.p0s_OUTER_TYPEs.i64.i64.p0s_OUTER_TYPEs.i64"(i8 0, i64 0, i64 1200, %"OUTER_TYPE"* elementtype(%"OUTER_TYPE") %temp0, i64 0)
  %temp2 = getelementptr inbounds %"OUTER_TYPE", %"OUTER_TYPE"* %temp1, i64 0, i32 0
  %temp3 = getelementptr inbounds %"OUTER_TYPE", %"OUTER_TYPE"* %temp1, i64 0, i32 1
  %temp4 = bitcast %"QNCA_a0$double*$rank3$"* %temp2 to %"QNCA_a0$double*$rank3$.7.39"*
  %temp5 = bitcast %"QNCA_a0$double*$rank2$"* %temp3 to %"QNCA_a0$double*$rank2$.7.39"*
  call void @bar(%"QNCA_a0$double*$rank3$.7.39"* %temp4, %"QNCA_a0$double*$rank2$.7.39"* %temp5)
  ret i32 0
}

declare %"OUTER_TYPE"* @"llvm.intel.subscript.p0s_OUTER_TYPEs.i64.i64.p0s_OUTER_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"OUTER_TYPE"* elementtype(%"OUTER_TYPE") %3, i64 %4)

; Check that the types were merged and the new types were created
; CHECK-DAG: %"__DTRT_QNCA_a0$%\22OUTER_TYPE\22*$rank1$" = type { %__DTRT_OUTER_TYPE*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK-DAG: %__DTRT_OUTER_TYPE = type { %"__DTRT_QNCA_a0$double*$rank3$", %"__DTRT_QNCA_a0$double*$rank2$" }
; CHECK-DAG: %"__DTRT_QNCA_a0$double*$rank3$" = type { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
; CHECK-DAG: %"__DTRT_QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64 }] }

; Check that the global variable was updated
; CHECK-DAG: @glob_dope_vector = internal global %"__DTRT_QNCA_a0$%\22OUTER_TYPE\22*$rank1$" { %__DTRT_OUTER_TYPE* null, i64 0, i64 0, i64 1342177408, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }

; Check that the instructions in main were updated
; CHECK: define i32 @main
; CHECK:   %temp0 = load %__DTRT_OUTER_TYPE*, %__DTRT_OUTER_TYPE** getelementptr inbounds (%"__DTRT_QNCA_a0$%\22OUTER_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22OUTER_TYPE\22*$rank1$"* @glob_dope_vector, i64 0, i32 0), align 8
; CHECK:   %temp1 = tail call %__DTRT_OUTER_TYPE* @llvm.intel.subscript.p0s___DTRT_OUTER_TYPEs.i64.i64.p0s___DTRT_OUTER_TYPEs.i64(i8 0, i64 0, i64 1200, %__DTRT_OUTER_TYPE* elementtype(%__DTRT_OUTER_TYPE) %temp0, i64 0)
; CHECK:   %temp2 = getelementptr inbounds %__DTRT_OUTER_TYPE, %__DTRT_OUTER_TYPE* %temp1, i64 0, i32 0
; CHECK:   %temp3 = getelementptr inbounds %__DTRT_OUTER_TYPE, %__DTRT_OUTER_TYPE* %temp1, i64 0, i32 1
; CHECK:   call void @bar.1(%"__DTRT_QNCA_a0$double*$rank3$"* %temp2, %"__DTRT_QNCA_a0$double*$rank2$"* %temp3)

; Check that the clone for bar was generated and the instructions were updated
; CHECK: define internal void @bar.1(%"__DTRT_QNCA_a0$double*$rank3$"* %dv1, %"__DTRT_QNCA_a0$double*$rank2$"* %dv2) {
; CHECK:   %temp0 = getelementptr inbounds %"__DTRT_QNCA_a0$double*$rank3$", %"__DTRT_QNCA_a0$double*$rank3$"* %dv1, i64 0, i32 0
; CHECK:   %temp1 = load double*, double** %temp0, align 8
; CHECK:   %temp2 = getelementptr inbounds %"__DTRT_QNCA_a0$double*$rank2$", %"__DTRT_QNCA_a0$double*$rank2$"* %dv2, i64 0, i32 0
; CHECK:   %temp3 = load double*, double** %temp2, align 8
; CHECK:   ret void
; CHECK: }

; Check that the llvm.intel.subscript instrinsic was generated correctly
; CHECK: declare %__DTRT_OUTER_TYPE* @llvm.intel.subscript.p0s___DTRT_OUTER_TYPEs.i64.i64.p0s___DTRT_OUTER_TYPEs.i64(i8, i64, i64, %__DTRT_OUTER_TYPE*, i64)

