; RUN: opt -auto-cpu-clone < %s -S | FileCheck %s
; RUN: opt -passes=auto-cpu-clone < %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Trying to add byval to the ifunc would result in
;
;   error: argument attributes invalid in function type
;
; So that *seems* to be fine.

; CHECK-DAG: @foo = ifunc i32 (%struct*), i32 (%struct*)* ()* @foo.resolver
; CHECK-DAG: define i32 @foo.A(%struct* byval(%struct) %s) #{{[0-9]*}} !llvm.acd.clone !0 {
; CHECK-DAG: define i32 @foo.b(%struct* byval(%struct) %s) #{{[0-9]*}} !llvm.acd.clone !0 {
; CHECK-DAG: define i32 (%struct*)* @foo.resolver()
; CHECK-DAG: ret i32 (%struct*)* @foo.b
; CHECK-DAG: ret i32 (%struct*)* @foo.A

%struct = type { i32 }

define i32 @foo(%struct* byval(%struct) %s) !llvm.auto.cpu.dispatch !1 {
  %gep = getelementptr %struct, %struct *%s, i32 0, i32 0
  %f = load i32, i32 *%gep
  %add = add i32 %f, 42
  ret i32 %add
}

define i32 @bar(i32 %a) {
  %s = alloca %struct
  %gep = getelementptr %struct, %struct *%s, i32 0, i32 0
  store i32 %a, i32 *%gep
  %ret = call i32 @foo(%struct* byval(%struct) %s)
  ret i32 %ret
}


attributes #0 = { "target-features"="+sse4.2" }

!0 = !{!"auto-cpu-dispatch-target", !"skylake"}

!1 = !{!0}
