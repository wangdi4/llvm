; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

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
define "intel_dtrans_func_index"="1" %struct.test01* @test() !intel.dtrans.func.type !4 {
  %mem = tail call i8* @malloc(i64 16)
  %failed = icmp eq i8* %mem, null
  br i1 %failed, label %fail, label %success
fail:
  call void @free(i8* null)
  %bad = bitcast i8* null to %struct.test01*
; CHECK-NONOPAQUE: %bad = bitcast i8* null to %struct.test01*
; CHECK-OPAQUE:    %bad = bitcast ptr null to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01*
; CHECK-NEXT:      No element pointees.
  %other = bitcast i8* null to %struct.test02*

; CHECK-NONOPAQUE: %other = bitcast i8* null to %struct.test02*
; CHECK-OPAQUE:    %other = bitcast ptr null to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02*
; CHECK-NEXT:      No element pointees.

  %cmp = icmp eq %struct.test02* null, %other
  %g = getelementptr %struct.test02, %struct.test02* %other, i64 0, i32 0
  br label %done
success:
  %good = bitcast i8* %mem to %struct.test01*
; CHECK-NONOPAQUE: %good = bitcast i8* %mem to %struct.test01*
; CHECK-OPAQUE:    %good = bitcast ptr %mem to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.

  br label %done

done:
  %res = phi %struct.test01* [ %good, %success ], [ %bad, %fail ]
; CHECK-NONOPAQUE: %res = phi %struct.test01* [ %good, %success ], [ %bad, %fail ]
; CHECK-OPAQUE:    %res = phi ptr [ %good, %success ], [ %bad, %fail ]
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:         i8*
; CHECK-NEXT:       No element pointees.

  %phi2 = phi %struct.test01* [ %good, %success ], [ null, %fail ]
; CHECK-NONOPAQUE: %phi2 = phi %struct.test01* [ %good, %success ], [ null, %fail ]
; CHECK-OPAQUE:    %phi2 = phi ptr [ %good, %success ], [ null, %fail ]
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:         i8*
; CHECK-NEXT:       No element pointees.

  %gep = getelementptr i8, i8* null, i64 32
; CHECK-NONOPAQUE: %gep = getelementptr i8, i8* null, i64 32
; CHECK-OPAQUE:    %gep = getelementptr i8, ptr null, i64 32
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         i8*
; CHECK-NEXT:       No element pointees.

%gep2 = getelementptr %struct.test02, %struct.test02* null, i64 0
; CHECK-NONOPAQUE: %gep2 = getelementptr %struct.test02, %struct.test02* null, i64 0
; CHECK-OPAQUE:    %gep2 = getelementptr %struct.test02, ptr null, i64 0
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test02*
; CHECK-NEXT:       No element pointees.

  %itp = inttoptr i64 0 to %struct.test01*
; CHECK-NONOPAQUE: %itp = inttoptr i64 0 to %struct.test01*
; CHECK-OPAQUE:    %itp = inttoptr i64 0 to ptr
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:       No element pointees.

  %t1 = getelementptr %struct.test01, %struct.test01* %itp, i64 0
; CHECK-NONOPAQUE: %t1 = getelementptr %struct.test01, %struct.test01* %itp, i64 0
; CHECK-OPAQUE:    %t1 = getelementptr %struct.test01, ptr %itp, i64 0
; CHECK-NEXT:     LocalPointerInfo:
; CHECK-NEXT:       Aliased types:
; CHECK-NEXT:         %struct.test01*
; CHECK-NEXT:       No element pointees.

  ret %struct.test01* %res
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @malloc(i64)
declare !intel.dtrans.func.type !7 void @free(i8* "intel_dtrans_func_index"="1")

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
