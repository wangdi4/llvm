; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; Test partial write of an inner structure with memset. Ensure that the
; outer, other, and inner structures are marked 'Multi-struct memfunc' to
; inhibit optimizations that need to transform memfuncs. Also check that all
; of the fields of other are written.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS5inner.inner = type { i32, i32 }
; CHECK: Name: struct._ZTS5inner.inner
; CHECK: Number of fields: 2
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: Safety data: Global instance | Memfunc partial write | Nested structure | Multi-struct memfunc{{ *$}}
; CHECK: End LLVMType: %struct._ZTS5inner.inner = type { i32, i32 }

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS5other.other = type { i16, i16, i16, i16 }
; CHECK: Name: struct._ZTS5other.other
; CHECK: Number of fields: 4
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 2)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 3)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: Safety data: Global instance | Nested structure | Multi-struct memfunc{{ *$}}
; CHECK: End LLVMType: %struct._ZTS5other.other = type { i16, i16, i16, i16 }

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS5outer.outer = type { i32, i32, %struct._ZTS5other.other, i32, i32, %struct._ZTS5inner.inner, i32 }
; CHECK: Name: struct._ZTS5outer.outer
; CHECK: Number of fields: 7
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 2)Field LLVM Type: %struct._ZTS5other.other = type { i16, i16, i16, i16 }
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 5)Field LLVM Type: %struct._ZTS5inner.inner = type { i32, i32 }
; CHECK: Field info: Written
; CHECK: Readers: main{{ *$}}
; CHECK: Writers: main{{ *$}}
; CHECK: 6)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: Safety data: Global instance | Memfunc partial write | Contains nested structure | Multi-struct memfunc{{ *$}}
; CHECK: End LLVMType: %struct._ZTS5outer.outer = type { i32, i32, %struct._ZTS5other.other, i32, i32, %struct._ZTS5inner.inner, i32 }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS5outer.outer = type { i32, i32, %struct._ZTS5other.other, i32, i32, %struct._ZTS5inner.inner, i32 }
%struct._ZTS5inner.inner = type { i32, i32 }
%struct._ZTS5other.other = type { i16, i16, i16, i16 }

@mystruct = internal global %struct._ZTS5outer.outer zeroinitializer, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  call void @llvm.memcpy.p0.i64(ptr align 4 getelementptr inbounds (%struct._ZTS5outer.outer, ptr @mystruct, i32 0, i32 1), ptr align 4 getelementptr inbounds (%struct._ZTS5outer.outer, ptr @mystruct, i32 0, i32 1), i64 24, i1 false)
  ret i32 0
}

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn writeonly
declare void @llvm.memcpy.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nounwind willreturn writeonly }

!llvm.module.flags = !{!0, !1}
!intel.dtrans.types = !{!2, !5, !8}
!llvm.ident = !{!9}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"S", %struct._ZTS5outer.outer zeroinitializer, i32 7, !3, !3, !4, !3, !3, !7, !3}
!3 = !{i32 0, i32 0}
!4 = !{%struct._ZTS5other.other zeroinitializer, i32 0}
!5 = !{!"S", %struct._ZTS5other.other zeroinitializer, i32 4, !6, !6, !6, !6}
!6 = !{i16 0, i32 0}
!7 = !{%struct._ZTS5inner.inner zeroinitializer, i32 0}
!8 = !{!"S", %struct._ZTS5inner.inner zeroinitializer, i32 2, !3, !3}
!9 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
