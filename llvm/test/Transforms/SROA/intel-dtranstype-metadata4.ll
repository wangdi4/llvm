; INTEL_FEATURE_SW_DTRANS

; RUN: opt -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK

; Test that alloca instructions created by SROA extracting fields from a
; structure that has DTrans type metadata gets DTrans metadata associated with
; it. In this case, the type used in one of the alloca instructions is a subset
; of fields taken from a nested structure, so the DTrans metadata propagation
; needs to navigate to that structure to form the metadata for a new literal structure.

%struct.coder = type { %struct.filter, i64, %struct.allocator*, %struct.allocator*, i32*, i32*, %struct.filter* }
%struct.allocator = type { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
%struct.filter = type { i64*, i8*, i64 }

define void @test() {
  %coder = alloca %struct.coder, align 8
  %bc = bitcast %struct.coder* %coder to i8*
  %gep = getelementptr i8, i8* %bc, i64 8
  call void @llvm.memset.p0i8.i64(i8* %gep, i8 0, i64 64, i1 false)
  %id = getelementptr inbounds %struct.coder, %struct.coder* %coder, i32 0, i32 1
  store i64 0, i64* %id, align 8
  ret void
}

; CHECK-TYPED: %coder.sroa.{{[0-9]+}} = alloca { i8*, i64 }, align 8, !intel_dtrans_type ![[MD_L1:[0-9]+]]
; CHECK-TYPED: %coder.sroa.{{[0-9]+}} = alloca { %struct.allocator*, %struct.allocator*, i32*, i32*, %struct.filter* }, align 8, !intel_dtrans_type ![[MD_L2:[0-9]+]]
; CHECK-OPAQUE: %coder.sroa.{{[0-9]+}} = alloca { ptr, i64 }, align 8, !intel_dtrans_type ![[MD_L1:[0-9]+]]
; CHECK-OPAQUE: %coder.sroa.{{[0-9]+}} = alloca { ptr, ptr, ptr, ptr, ptr }, align 8, !intel_dtrans_type ![[MD_L2:[0-9]+]]

; CHECK: ![[MD_I64:[0-9]+]] = !{i64 0, i32 0}
; CHECK: ![[MD_PSA:[0-9]+]] = !{%struct.allocator zeroinitializer, i32 1}
; CHECK: ![[MD_P32:[0-9]+]] = !{i32 0, i32 1}
; CHECK: ![[MD_PSF:[0-9]+]] = !{%struct.filter zeroinitializer, i32 1}
; CHECK: ![[MD_P8:[0-9]+]] = !{i8 0, i32 1}
; CHECK: ![[MD_L1]] = !{!"L", i32 2, ![[MD_P8]], ![[MD_I64]]}
; CHECK: ![[MD_L2]] = !{!"L", i32 5, ![[MD_PSA]], ![[MD_PSA]], ![[MD_P32]], ![[MD_P32]], ![[MD_PSF]]}

declare !intel.dtrans.func.type !13 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

!1 = !{%struct.filter zeroinitializer, i32 0}  ; %struct.filter
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.allocator zeroinitializer, i32 1}  ; %struct.allocator*
!4 = !{i32 0, i32 1}  ; i32*
!5 = !{%struct.filter zeroinitializer, i32 1}  ; %struct.filter*
!6 = !{!"F", i1 false, i32 3, !7, !7, !2, !2}  ; i8* (i8*, i64, i64)
!7 = !{i8 0, i32 1}  ; i8*
!8 = !{!6, i32 1}  ; i8* (i8*, i64, i64)*
!9 = !{!"F", i1 false, i32 2, !10, !7, !7}  ; void (i8*, i8*)
!10 = !{!"void", i32 0}  ; void
!11 = !{!9, i32 1}  ; void (i8*, i8*)*
!12 = !{i64 0, i32 1}  ; i64*
!13 = distinct !{!7}
!14 = !{!"S", %struct.coder zeroinitializer, i32 7, !1, !2, !3, !3, !4, !4, !5} ; { %struct.filter, i64, %struct.allocator*, %struct.allocator*, i32*, i32*, %struct.filter* }
!15 = !{!"S", %struct.allocator zeroinitializer, i32 3, !8, !11, !7} ; { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
!16 = !{!"S", %struct.filter zeroinitializer, i32 3, !12, !7, !2} ; { i64*, i8*, i64 }

!intel.dtrans.types = !{!14, !15, !16}

; end INTEL_FEATURE_SW_DTRANS
