; INTEL_FEATURE_SW_DTRANS

; RUN: opt -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-TYPED --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK
; RUN: opt -opaque-pointers -S -passes=sroa %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE --check-prefix=CHECK

; Test that a literal structure created by SROA by extracting fields from a
; structure that has DTrans type metadata gets DTrans metadata associated with
; in when the literal structure contains a pointer type. Also, verify that an
; alloca of a pointer created from extracting a single field from a structure
; gets DTrans metadata.

%struct.coder = type { %struct.filter*, i64, i64, %struct.allocator*, %struct.allocator*, i32*, i32*, %struct.filter* }
%struct.allocator = type { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
%struct.filter = type { i64, i8* }

define void @test() {
  %coder = alloca %struct.coder, align 8
  %bc = bitcast %struct.coder* %coder to i8*
  call void @llvm.memset.p0i8.i64(i8* %bc, i8 0, i64 64, i1 false)
  %id = getelementptr inbounds %struct.coder, %struct.coder* %coder, i32 0, i32 1
  store i64 0, i64* %id, align 8
  ret void
}

; CHECK-TYPED: %coder.sroa.{{[0-9]+}} = alloca %struct.filter*, align 8, !intel_dtrans_type ![[MD_PSF:[0-9]+]]
; CHECK-TYPED: %coder.sroa.{{[0-9]+}} = alloca { i64, %struct.allocator*, %struct.allocator*, i32*, i32*, %struct.filter* }, align 8, !intel_dtrans_type ![[MD_L:[0-9]+]]
; CHECK-OPAQUE: %coder.sroa.{{[0-9]+}} = alloca ptr, align 8, !intel_dtrans_type ![[MD_PSF:[0-9]+]]
; CHECK-OPAQUE: %coder.sroa.{{[0-9]+}} = alloca { i64, ptr, ptr, ptr, ptr, ptr }, align 8, !intel_dtrans_type ![[MD_L:[0-9]+]]

; CHECK: ![[MD_PSF]] = !{%struct.filter zeroinitializer, i32 1}
; CHECK: ![[MD_I64:[0-9]+]] = !{i64 0, i32 0}
; CHECK: ![[MD_PSA:[0-9]+]] = !{%struct.allocator zeroinitializer, i32 1}
; CHECK: ![[MD_P32:[0-9]+]] = !{i32 0, i32 1}
; CHECK: ![[MD_L]] = !{!"L", i32 6, ![[MD_I64]], ![[MD_PSA]], ![[MD_PSA]], ![[MD_P32]], ![[MD_P32]], ![[MD_PSF]]}

declare !intel.dtrans.func.type !11 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

!1 = !{%struct.filter zeroinitializer, i32 1}  ; %struct.filter*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.allocator zeroinitializer, i32 1}  ; %struct.allocator*
!4 = !{i32 0, i32 1}  ; i32*
!5 = !{!"F", i1 false, i32 3, !6, !6, !2, !2}  ; i8* (i8*, i64, i64)
!6 = !{i8 0, i32 1}  ; i8*
!7 = !{!5, i32 1}  ; i8* (i8*, i64, i64)*
!8 = !{!"F", i1 false, i32 2, !9, !6, !6}  ; void (i8*, i8*)
!9 = !{!"void", i32 0}  ; void
!10 = !{!8, i32 1}  ; void (i8*, i8*)*
!11 = distinct !{!6}
!12 = !{!"S", %struct.coder zeroinitializer, i32 8, !1, !2, !2, !3, !3, !4, !4, !1} ; { %struct.filter*, i64, i64, %struct.allocator*, %struct.allocator*, i32*, i32*, %struct.filter* }
!13 = !{!"S", %struct.allocator zeroinitializer, i32 3, !7, !10, !6} ; { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
!14 = !{!"S", %struct.filter zeroinitializer, i32 2, !2, !6} ; { i64, i8* }

!intel.dtrans.types = !{!12, !13, !14}

; end INTEL_FEATURE_SW_DTRANS
