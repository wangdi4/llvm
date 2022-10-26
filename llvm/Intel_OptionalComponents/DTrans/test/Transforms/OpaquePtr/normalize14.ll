; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that DTransNormalizeOP pass doesn't crash when processing
; opaque structs.

; CHECK: define fastcc void @skin_root_clear
; CHECK:  call fastcc void @skin_root_clear(ptr %0)

%struct._ZTS4GSet.GSet = type opaque

define fastcc void @skin_root_clear(ptr "intel_dtrans_func_index"="1" %0) !intel.dtrans.func.type !1 {
  call fastcc void @skin_root_clear(ptr %0)
  ret void
}

!intel.dtrans.types = !{!0}

!0 = !{!"S", %struct._ZTS4GSet.GSet zeroinitializer, i32 -1}
!1 = distinct !{!2}
!2 = !{%struct._ZTS4GSet.GSet zeroinitializer, i32 1}
