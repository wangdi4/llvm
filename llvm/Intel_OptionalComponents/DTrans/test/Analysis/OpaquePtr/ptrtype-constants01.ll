; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on instruction using compiler constants
; where the pointer type analyzer is attempting to resolve pointer types.
;
; The pointer type analyzer cannot use a single ValueTypeInfo object for
; compiler constants because the same Value object for the constant can be
; used with multiple instructions and represent different types. Instead,
; the pointer type analyzer associates the ValueTypeInfo with a
; { instruction, operand number } pair.
;
; This test is to verify the pointer type analyzer does not crash when
; evaluating these null constants. (CMPLRLLVM-31691)

%struct.test01 = type { i64, i64 }
%struct.test02 = type { i32, i32 }
define "intel_dtrans_func_index"="1" ptr @test() !intel.dtrans.func.type !4 {
  %mem = tail call ptr @malloc(i64 16)
  %failed = icmp eq ptr %mem, null
  br i1 %failed, label %fail, label %success
fail:
  call void @free(ptr null)
  %bad = bitcast ptr null to ptr
; CHECK:    %bad = bitcast ptr null to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01*
; CHECK-NEXT:      No element pointees.
  %other = bitcast ptr null to ptr

; CHECK:    %other = bitcast ptr null to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02*
; CHECK-NEXT:      No element pointees.

  %cmp = icmp eq ptr null, %other
  %g = getelementptr %struct.test02, ptr %other, i64 0, i32 0
  br label %done
success:
  %good = bitcast ptr %mem to ptr
; CHECK:    %good = bitcast ptr %mem to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.

  br label %done

done:
  %res = phi ptr [ %good, %success ], [ %bad, %fail ]
; CHECK:    %res = phi ptr [ %good, %success ], [ %bad, %fail ]
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:         i8*
; CHECK-NEXT:       No element pointees.

  %phi2 = phi ptr [ %good, %success ], [ null, %fail ]
; CHECK:    %phi2 = phi ptr [ %good, %success ], [ null, %fail ]
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:         i8*
; CHECK-NEXT:       No element pointees.

  %gep = getelementptr i8, ptr null, i64 32
; CHECK:    %gep = getelementptr i8, ptr null, i64 32
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         i8*
; CHECK-NEXT:       No element pointees.

%gep2 = getelementptr %struct.test02, ptr null, i64 0
; CHECK:    %gep2 = getelementptr %struct.test02, ptr null, i64 0
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test02*
; CHECK-NEXT:       No element pointees.

  %itp = inttoptr i64 0 to ptr
; CHECK:    %itp = inttoptr i64 0 to ptr
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:       No element pointees.

  %t1 = getelementptr %struct.test01, ptr %itp, i64 0
; CHECK:    %t1 = getelementptr %struct.test01, ptr %itp, i64 0
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:       No element pointees.

  ret ptr %res
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @malloc(i64)
declare !intel.dtrans.func.type !7 void @free(ptr "intel_dtrans_func_index"="1")

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!intel.dtrans.types = !{!8, !9}
