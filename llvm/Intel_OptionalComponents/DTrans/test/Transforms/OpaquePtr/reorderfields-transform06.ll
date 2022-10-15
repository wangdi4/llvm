; This test verifies that the field reordering transformation is applied
; correctly to memmov/memcpy/memset with constant and non-constant
; sizes related to %struct.test.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -dtrans-reorderfieldsop | FileCheck %s
;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define void @foo(ptr "intel_dtrans_func_index"="1" %tp1, ptr "intel_dtrans_func_index"="2" %tp2, ptr "intel_dtrans_func_index"="3" %tp3) !intel.dtrans.func.type !5 {
entry:
  %p1 = bitcast ptr %tp1 to ptr
  %p2 = bitcast ptr %tp2 to ptr
  %call = bitcast ptr %tp2 to ptr
  %call1 = bitcast ptr %tp2 to ptr

  %ld2 = load i32, ptr @G, align 4
  %conv2 = sext i32 %ld2 to i64
  %mul = mul nsw i64 %conv2, 48
  call void @llvm.memset.p0.i64(ptr %call, i8 0, i64 %mul, i1 false)
; CHECK:  %[[SDIV:[0-9]+]] = sdiv exact i64 %mul, 48
; CHECK:  %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK:  void @llvm.memset.p0.i64(ptr %call, i8 0, i64 %[[MUL]], i1 false)

  call void @llvm.memset.p0.i64(ptr %call1, i8 0, i64 48, i1 false)
; CHECK:  void @llvm.memset.p0.i64(ptr %call1, i8 0, i64 40, i1 false)

  call void @llvm.memmove.p0.p0.i64(ptr %p2, ptr %p1, i64 48, i1 false)
; CHECK:  void @llvm.memmove.p0.p0.i64(ptr %p2, ptr %p1, i64 40, i1 false)

  call void @llvm.memcpy.p0.p0.i64(ptr %p1, ptr %p2, i64 48, i1 false)
; CHECK:  void @llvm.memcpy.p0.p0.i64(ptr %p1, ptr %p2, i64 40, i1 false)

  %p3 = bitcast ptr %tp3 to ptr
  %ld1 = load i32, ptr @G, align 4
  %conv1 = sext i32 %ld1 to i64
  %mul1 = mul nsw i64 %conv1, 48
  call void @llvm.memcpy.p0.p0.i64(ptr %p3, ptr %p2, i64 %mul1, i1 false)
; CHECK:  %[[SDIV:[0-9]+]] = sdiv exact i64 %mul1, 48
; CHECK:  %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK:  void @llvm.memcpy.p0.p0.i64(ptr %p3, ptr %p2, i64 %[[MUL]], i1 false)

  %mul2 = mul nsw i64 %conv1, 48
  call void @llvm.memmove.p0.p0.i64(ptr %p2, ptr %p3, i64 %mul2, i1 false)
; CHECK:  %[[SDIV:[0-9]+]] = sdiv exact i64 %mul2, 48
; CHECK:  %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK:  void @llvm.memmove.p0.p0.i64(ptr %p2, ptr %p3, i64 %[[MUL]], i1 false)

  ret void
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !7 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)
declare !intel.dtrans.func.type !8 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !9 void @llvm.memmove.p0.p0.i64(ptr "intel_dtrans_func_index"="1" , ptr "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4, !4, !4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = distinct !{!6, !6}
!9 = distinct !{!6, !6}
!10 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!10}
