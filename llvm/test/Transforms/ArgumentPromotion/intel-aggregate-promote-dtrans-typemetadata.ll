; INTEL_FEATURE_SW_DTRANS

; This test is to verify that DTrans metadata gets updated on the @test
; function when argument promotion changes the signature to pass in pointers
; to structure field members instead of a pointer to the structure itself.

; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

%struct.test = type { ptr, ptr, ptr, ptr }
@G = global %struct.test zeroinitializer

declare i32 @foo(ptr, ptr)



define i32 @caller() {
entry:
  %v = call i32 @test(ptr @G)
  ret i32 %v
}

; Argument promotion hosts the loads to the caller, and updates the call arguments
; to the called function.
; CHECK: define i32 @caller() {
; CHECK:  %0 = getelementptr i8, ptr @G, i64 16
; CHECK:  %G.val = load ptr, ptr %0, align 8
; CHECK:  %1 = getelementptr i8, ptr @G, i64 24
; CHECK:  %G.val1 = load ptr, ptr %1, align 8
; CHECK:  %v = call i32 @test(ptr %G.val, ptr %G.val1)

define internal i32 @test(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !3 {
entry:
  %a.gep = getelementptr %struct.test, ptr %p, i64 0, i32 3
  %b.gep = getelementptr %struct.test, ptr %p, i64 0, i32 2
  %a = load ptr, ptr %a.gep
  %b = load ptr, ptr %b.gep
  %v = call i32 @foo(ptr %a, ptr %b)
  ret i32 %v
}

; The main check for this test is to verify that the DTrans attributes and
; metadata gets updated to relect the types of the fields passed in.
; CHECK: define internal i32 @test(ptr "intel_dtrans_func_index"="1" %p.16.val, ptr "intel_dtrans_func_index"="2" %p.24.val) !intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32*, i32*, i32*, i32* }

!intel.dtrans.types = !{!4}

; CHECK: ![[I32PTR:[0-9]+]] = !{i32 0, i32 1}
; CHECK: ![[FUNC_MD]] = distinct !{![[I32PTR]], ![[I32PTR]]}

; end INTEL_FEATURE_SW_DTRANS
