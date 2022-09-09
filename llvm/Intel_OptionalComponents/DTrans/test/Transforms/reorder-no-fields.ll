; This test verifies dtrans field reordering transformation doesn't crash
; if a structure doesn't have fields.

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -whole-program-assume -dtrans-reorderfields  -disable-output  2>/dev/null
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-reorderfields  -disable-output  2>/dev/null


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.TCDef = type { i16, %struct.TCDef*, {}* }
declare void @f1(i8*);
define void @test1(%struct.TCDef* %s) {
  %p = bitcast %struct.TCDef* %s to i8*
  call void @f1(i8* %p)
  ret void
}
