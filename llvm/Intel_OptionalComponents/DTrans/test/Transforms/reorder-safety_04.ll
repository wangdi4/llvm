; UNSUPPORTED: enable-opaque-pointers

;  RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

; This test verifies that the reordering transformation is NOT applied to
; struct.test because a memory write is done to the first field of the structure
; using a bitcast of a pointer to the structure. The transformation does not
; currently support rewriting the store instruction if the structure fields were
; reordered.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }
; CHECK-NOT: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
  %call = call i8* @calloc(i64 10, i64 48)
  %s = bitcast i8* %call to %struct.test*

  ; The following instructions write the structure field, but reorder fields
  ; does not support transforming this write to the location of a reordered
  ; structure.
  %i = bitcast i8* %call to i32*
  store i32 10, i32* %i
  ret i32 0
}

declare i8* @calloc(i64, i64)
