; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that (B:0) and (B:2) are deleted and GEP is updated. Memset call size should be updated.

; CHECK-DAG: %__DFDT_struct.A = type { i32, %__DFT_struct.B, i32 }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

; CHECK: @foo
; CHECK: call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 12, i1 false)
; CHECK: getelementptr inbounds i8, i8* %p, i64 0
; CHECK: getelementptr inbounds i8, i8* %p, i64 8

; CHECK: @bar
; CHECK: getelementptr inbounds i8, i8* %p, i64 0

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }

define i32 @foo(%struct.A* %a) {
entry:
  %p = bitcast %struct.A* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 16, i1 false)

  %px = getelementptr inbounds i8, i8* %p, i64 0 ; (A:0)
  %py = getelementptr inbounds i8, i8* %p, i64 12 ; (A:2)
  %x = bitcast i8* %px to i32*
  %y = bitcast i8* %py to i32*
  %0 = load i32, i32* %x, align 4
  %1 = load i32, i32* %y, align 4
  %2 = add i32 %0, %1

  ret i32 %2
}

define i16 @bar(%struct.A* %a) {
entry:
  %p = bitcast %struct.A* %a to i8*
  %py = getelementptr inbounds i8, i8* %p, i64 6 ; (A:1:1)
  %y = bitcast i8* %py to i16*
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
