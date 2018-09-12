; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that (B:0) and (B:2) are deleted and GEP is updated. Memset call size should be updated.

; CHECK-DAG: %__DFDT_struct.A = type { i32, %__DFT_struct.B, i32 }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

; CHECK: call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 12, i1 false)
; CHECK: getelementptr inbounds %__DFDT_struct.A, %__DFDT_struct.A* %a, i64 0, i32 0
; CHECK: getelementptr inbounds %__DFDT_struct.A, %__DFDT_struct.A* %a, i64 0, i32 2

; CHECK: getelementptr inbounds %__DFT_struct.B, %__DFT_struct.B* %b, i64 0, i32 0

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }

define i32 @foo(%struct.A* %a) {
entry:
  %p = bitcast %struct.A* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 16, i1 false)

  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2
  %0 = load i32, i32* %x, align 4
  %1 = load i32, i32* %y, align 4
  %2 = add i32 %0, %1

  ret i32 %2
}

define i16 @bar(%struct.B* %b) {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
