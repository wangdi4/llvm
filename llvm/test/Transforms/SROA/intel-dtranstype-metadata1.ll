; INTEL_FEATURE_SW_DTRANS

; RUN: opt -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK

; Test that a literal structure created by SROA extracting fields from a
; structure that has DTrans type metadata gets DTrans metadata associated with
; it when the literal structure created contains a pointer type.

%struct.tm = type { i32, i32, i32, i32, i32, i32, i64, %struct.test* }
%struct.test = type { i64, i64 }

define fastcc void @test() {
  ; This alloca gets split into two separate allocas. One contained the fields
  ; prior to the field addressed by the getelementptr, and one with fields
  ; after.
  %i3 = alloca %struct.tm, align 8
  %i5 = bitcast %struct.tm* %i3 to i8*
  call void @llvm.memset.p0i8.i64(i8* %i5, i8 0, i64 48, i1 false)
  %i30 = getelementptr inbounds %struct.tm, %struct.tm* %i3, i64 0, i32 5
  %i31 = load i32, i32* %i30, align 4
  ret void
}
; CHECK-TYPED: %i3.sroa.{{[0-9]+}} = alloca { i64, %struct.test* }, align 8, !intel_dtrans_type ![[MD:[0-9]+]]
; CHECK-OPAQUE: %i3.sroa.{{[0-9]+}} = alloca { i64, ptr }, align 8, !intel_dtrans_type ![[MD:[0-9]+]]

; CHECK: ![[MD_I64:[0-9]+]] = !{i64 0, i32 0}
; CHECK: ![[MD_PSTRUCT_TEST:[0-9]+]] = !{%struct.test zeroinitializer, i32 1}
; CHECK: ![[MD]] = !{!"L", i32 2, ![[MD_I64]], ![[MD_PSTRUCT_TEST]]}

declare !intel.dtrans.func.type !5 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.tm zeroinitializer, i32 8, !1, !1, !1, !1, !1, !1, !2, !3} ; { i32, i32, i32, i32, i32, i32, i64, %struct.test* }
!7 = !{!"S", %struct.test zeroinitializer, i32 2, !2, !2} ; { i64, i64 }

!intel.dtrans.types = !{!6, !7}

; end INTEL_FEATURE_SW_DTRANS
