; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -disable-output -debug-only=dtrans-dynclone-reencoding -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -disable-output -debug-only=dtrans-dynclone-reencoding -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dynclone' 2>&1 | FileCheck %s

; This test is to verify that constant collection for re-encoding collects constants
; when a @llvm.smax intrinsic is used, instead of a select instruction. (CMPLRLLVM-36879)

; CHECK: (Reencoding) Constant collected for encoding: 20000000 : struct: __SOADT___DFR___DFT_struct.arc Index: 0

%__SOA___DFR___DFT_struct.node = type { i64*, i32*, i32*, i32*, i32*, i32*, %__SOADT___DFR___DFT_struct.arc**, %__SOADT___DFR___DFT_struct.arc**, %__SOADT___DFR___DFT_struct.arc**, i64*, i64*, i32*, i32* }
%__SOADT___DFR___DFT_struct.arc = type { i64, i32, i32, i64, i64, i32, i16 }
%__SOADT___DFR___DFDT_struct.network = type { [200 x i8], [200 x i8], i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, double, i64, i32, i32, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc*, i64, i64, i64, i64, i64 }

@net = internal global %__SOADT___DFR___DFDT_struct.network zeroinitializer

define i32 @main() {
  store i64 10000000, i64* getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, %__SOADT___DFR___DFDT_struct.network* @net, i64 0, i32 18), align 8
  %i = call i64 @read_min.49.79.109()
  %i2 = call i64 @primal_start_artificial.57.87.117()
  ret i32 0
}

define internal i64 @read_min.49.79.109() {
  %i = call i8* @calloc(i64 0, i64 0)
  %i1 = bitcast i8* %i to %__SOADT___DFR___DFT_struct.arc*
  %i3 = load i64, i64* getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, %__SOADT___DFR___DFDT_struct.network* @net, i64 0, i32 18), align 8
  %i4 = add nsw i64 %i3, 15
  %i5 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i1, i64 0, i32 0
  store i64 %i4, i64* %i5, align 8
  %i6 = load i64, i64* getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, %__SOADT___DFR___DFDT_struct.network* @net, i64 0, i32 18), align 8
  ; Using @llvm.smax, followed by shift or multiply should collect 20000000
  %i7 = call i64 @llvm.smax.i64(i64 %i6, i64 10000000)
  %i8 = shl nuw nsw i64 %i7, 1
  %i9 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i1, i64 0, i32 0
  store i64 %i8, i64* %i9, align 8
  %i14 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i1, i64 0, i32 0
  store i64 undef, i64* %i14, align 8
  %i16 = load i64, i64* getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, %__SOADT___DFR___DFDT_struct.network* @net, i64 0, i32 18), align 8
  %i17 = icmp sgt i64 0, 10000000
  %i18 = select i1 %i17, i64 0, i64 -20000000
  %i19 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i1, i64 0, i32 0
  store i64 -20000000, i64* %i19, align 8
  ret i64 0
}

define internal i64 @primal_start_artificial.57.87.117() {
  %a = load %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc** getelementptr inbounds (%__SOADT___DFR___DFDT_struct.network, %__SOADT___DFR___DFDT_struct.network* @net, i64 0, i32 23)
  %i = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %a, i64 0, i32 0
  store i64 100000000, i64* %i, align 8
  ret i64 0
}

declare i8* @calloc(i64, i64)
declare i64 @llvm.smax.i64(i64, i64)
