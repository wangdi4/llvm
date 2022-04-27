; INTEL_FEATURE_SW_DTRANS

; RUN: opt -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK

; Test that alloca instructions created by SROA extracting fields from a
; structure that has DTrans type metadata gets DTrans metadata associated with
; it. In this case, the type used in one of the alloca instructions is a single
; pointer type taken from a nested structure, so the DTrans metadata propagation
; needs to navigate to that structure to find the type to attach to alloca of the
; pointer type.

%struct.coder = type { %struct.filter, i8*, i64, i64, %struct.allocator*, %struct.filter** }
%struct.allocator = type { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
%struct.filter = type { i64** }

define void @test() {
  %coder = alloca %struct.coder, align 8
  %bc = bitcast %struct.coder* %coder to i8*
  call void @llvm.memset.p0i8.i64(i8* %bc, i8 0, i64 64, i1 false)
  %id = getelementptr inbounds %struct.coder, %struct.coder* %coder, i32 0, i32 1
  store i8* null, i8** %id, align 8
  ret void
}
; CHECK-TYPED: %coder.sroa.{{[0-9]+}} = alloca i64**, align 8, !intel_dtrans_type ![[MD_PP64:[0-9]+]]
; CHECK-TYPED: %coder.sroa.{{[0-9]+}} = alloca { i64, i64, %struct.allocator*, %struct.filter** }, align 8, !intel_dtrans_type ![[MD_L:[0-9]+]]
; CHECK-OPAQUE: %coder.sroa.{{[0-9]+}} = alloca ptr, align 8, !intel_dtrans_type ![[MD_PP64:[0-9]+]]
; CHECK-OPAQUE: %coder.sroa.{{[0-9]+}} = alloca { i64, i64, ptr, ptr }, align 8, !intel_dtrans_type ![[MD_L:[0-9]+]]

; CHECK: ![[MD_I64:[0-9]+]] = !{i64 0, i32 0}
; CHECK: ![[MD_PSA:[0-9]+]] = !{%struct.allocator zeroinitializer, i32 1}
; CHECK: ![[MD_PPSF:[0-9]+]] = !{%struct.filter zeroinitializer, i32 2}
; CHECK: ![[MD_PP64]] = !{i64 0, i32 2}
; CHECK: ![[MD_L]] = !{!"L", i32 4, ![[MD_I64]], ![[MD_I64]], ![[MD_PSA]], ![[MD_PPSF]]}

declare !intel.dtrans.func.type !12 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

!1 = !{%struct.filter zeroinitializer, i32 0}  ; %struct.filter
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.allocator zeroinitializer, i32 1}  ; %struct.allocator*
!5 = !{%struct.filter zeroinitializer, i32 2}  ; %struct.filter**
!6 = !{!"F", i1 false, i32 3, !2, !2, !3, !3}  ; i8* (i8*, i64, i64)
!7 = !{!6, i32 1}  ; i8* (i8*, i64, i64)*
!8 = !{!"F", i1 false, i32 2, !9, !2, !2}  ; void (i8*, i8*)
!9 = !{!"void", i32 0}  ; void
!10 = !{!8, i32 1}  ; void (i8*, i8*)*
!11 = !{i64 0, i32 2}  ; i64**
!12 = distinct !{!2}
!13 = !{!"S", %struct.coder zeroinitializer, i32 6, !1, !2, !3, !3, !4, !5} ; { %struct.filter, i8*, i64, i64, %struct.allocator*, %struct.filter** }
!14 = !{!"S", %struct.allocator zeroinitializer, i32 3, !7, !10, !2} ; { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
!15 = !{!"S", %struct.filter zeroinitializer, i32 1, !11} ; { i64** }

!intel.dtrans.types = !{!13, !14, !15}

; end INTEL_FEATURE_SW_DTRANS
