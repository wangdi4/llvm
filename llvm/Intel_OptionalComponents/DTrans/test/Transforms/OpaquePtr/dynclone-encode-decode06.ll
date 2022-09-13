; REQUIRES: asserts

;  RUN: opt < %s -opaque-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -disable-output -debug-only=dtrans-dynclone-reencoding -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -opaque-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -disable-output -debug-only=dtrans-dynclone-reencoding -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

; This test is to verify that constant collection for re-encoding collects constants
; when a @llvm.smax intrinsic is used, instead of a select instruction. (CMPLRLLVM-36879)

; CHECK: (Reencoding) Constant collected for encoding: 20000000 : struct: __SOADT___DFR___DFT_struct.arc Index: 0

%__SOA___DFR___DFT_struct.node = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%__SOADT___DFR___DFT_struct.arc = type { i64, i32, i32, i64, i64, i32, i16 }
%__SOADT___DFR___DFDT_struct.network = type { [200 x i8], [200 x i8], i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, double, i64, i32, i32, ptr, ptr, ptr, ptr, ptr, i64, i64, i64, i64, i64 }

@net = internal global %__SOADT___DFR___DFDT_struct.network zeroinitializer

define i32 @main() {
  store i64 10000000, ptr getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, ptr @net, i64 0, i32 18), align 8
  %i = call i64 @read_min.49.79.109()
  %i2 = call i64 @primal_start_artificial.57.87.117()
  ret i32 0
}

define internal i64 @read_min.49.79.109() {
  %i = call ptr @calloc(i64 0, i64 0)
  %i3 = load i64, ptr getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, ptr @net, i64 0, i32 18), align 8
  %i4 = add nsw i64 %i3, 15
  %i5 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, ptr %i, i64 0, i32 0
  store i64 %i4, ptr %i5, align 8
  %i6 = load i64, ptr getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, ptr @net, i64 0, i32 18), align 8
  ; Using @llvm.smax, followed by shift or multiply should collect 20000000
  %i7 = call i64 @llvm.smax.i64(i64 %i6, i64 10000000)
  %i8 = shl nuw nsw i64 %i7, 1
  %i9 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, ptr %i, i64 0, i32 0
  store i64 %i8, ptr %i9, align 8
  %i14 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, ptr %i, i64 0, i32 0
  store i64 undef, ptr %i14, align 8
  %i16 = load i64, ptr getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, ptr @net, i64 0, i32 18), align 8
  %i17 = icmp sgt i64 0, 10000000
  %i18 = select i1 %i17, i64 0, i64 -20000000
  %i19 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, ptr %i, i64 0, i32 0
  store i64 -20000000, ptr %i19, align 8
  ret i64 0
}

define internal i64 @primal_start_artificial.57.87.117() {
  %a = load ptr, ptr getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, ptr @net, i64 0, i32 23)
  %i = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, ptr %a, i64 0, i32 0
  store i64 100000000, ptr %i, align 8
  ret i64 0
}

declare !intel.dtrans.func.type !12 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64)
declare i64 @llvm.smax.i64(i64, i64)

!1 = !{i64 0, i32 1}  ; i64*
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%__SOADT___DFR___DFT_struct.arc zeroinitializer, i32 2}  ; %__SOADT___DFR___DFT_struct.arc**
!4 = !{i64 0, i32 0}  ; i64
!5 = !{i32 0, i32 0}  ; i32
!6 = !{i16 0, i32 0}  ; i16
!7 = !{!"A", i32 200, !8}  ; [200 x i8]
!8 = !{i8 0, i32 0}  ; i8
!9 = !{double 0.0e+00, i32 0}  ; double
!10 = !{%__SOADT___DFR___DFT_struct.arc zeroinitializer, i32 1}  ; %__SOADT___DFR___DFT_struct.arc*
!11 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!11}
!13 = !{!"S", %__SOA___DFR___DFT_struct.node zeroinitializer, i32 13, !1, !2, !2, !2, !2, !2, !3, !3, !3, !1, !1, !2, !2} ; { i64*, i32*, i32*, i32*, i32*, i32*, %__SOADT___DFR___DFT_struct.arc**, %__SOADT___DFR___DFT_struct.arc**, %__SOADT___DFR___DFT_struct.arc**, i64*, i64*, i32*, i32* }
!14 = !{!"S", %__SOADT___DFR___DFT_struct.arc zeroinitializer, i32 7, !4, !5, !5, !4, !4, !5, !6} ; { i64, i32, i32, i64, i64, i32, i16 }
!15 = !{!"S", %__SOADT___DFR___DFDT_struct.network zeroinitializer, i32 33, !7, !7, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !9, !4, !5, !5, !10, !10, !10, !10, !10, !4, !4, !4, !4, !4} ; { [200 x i8], [200 x i8], i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, double, i64, i32, i32, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, i64, i64, i64, i64, i64 }

!intel.dtrans.types = !{!13, !14, !15}
