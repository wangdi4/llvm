; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -intel-ind-call-force-dtrans -passes=indirectcallconv -S < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that field single value indirect call specialization will specialize
; when the indirect call is done through a GEPOperator, even when memset
; is used to zero out the struct containing the function pointers.

; CHECK-NOT{{.*}}icmp{{.*}}
; CHECK:{{.*}}call i32 @foo()
; CHECK-NOT{{.*}}icmp{{.*}}
; CHECK:{{.*}}call i32 @bar()

; Needed to specify we are using 8 byte pointers (for memset)
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

define dso_local i32 @foo() {
  ret i32 5
}

define dso_local i32 @bar() {
  ret i32 5
}

%struct.MYSTRUCT = type { ptr, ptr }

@globstruct = internal global %struct.MYSTRUCT zeroinitializer, align 8

declare !intel.dtrans.func.type !5 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

define dso_local i32 @main() {
  %t2 = bitcast ptr @globstruct to ptr
  call void @llvm.memset.p0i8.i64(ptr %t2, i8 0, i64 16, i1 false)
  store ptr @foo, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct, i32 0, i32 0), align 8
  store ptr @bar, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct, i32 0, i32 1), align 8
  %t0 = load ptr, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct, i32 0, i32 0), align 8
  %call = call i32 %t0(), !intel_dtrans_type !1
  %t1 = load ptr, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct, i32 0, i32 1), align 8
  %call1 = call i32 %t1(), !intel_dtrans_type !1
  %add = add nsw i32 %call, %call1
  ret i32 %add
}
!1 = !{!"F", i1 false, i32 0, !2}  ; i32 ()
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 1}  ; i32 ()*
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !3, !3} ; { i32 ()*, i32 ()* }

!intel.dtrans.types = !{!6}

