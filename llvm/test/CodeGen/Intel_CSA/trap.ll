; RUN: llc -mtriple=csa < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @trap() {
; CHECK: # TRAP

  call void @llvm.trap()
  ret void
}

declare void @llvm.trap()