; CMPLRLLVM-27767: This test is almost same as dynclone-encode-decode01.ll
; except -20000000 is stored to the field1 of %struct.test.01 in "init" routine
; as follows. Using FSV, we can find %S3 is always -20000000.
; This test verifies the generated code for encoding and decoding of the
; constant.
;
;   %L = load i64, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1)
;   %IC = icmp sgt i64 %L, 10000000
;   %M2 = mul i64 -2, %L
;   %S3 = select i1 %IC, i64 %M2, i64 -20000000
;   store i64 %S3, i64* %F1, align 8

; This test verifies that basic transformations are done correctly for
; Load/Store instructions when DynClone+Reencoding transformation
; is triggered.

;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to 16 with encoding for large constants.
; Size of original %struct.test.01: 56
; Size after transformation: 30
;
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }
; CHECK: %struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }
; CHECK: %__DYN_struct.test.01 = type <{ i64*, i32, i32, i32, i32, i16, i16, i16 }>

%struct.nw = type { i32, i64 }
; Variable for candidate fields initialization.
@nw = internal global %struct.nw zeroinitializer, align 8

; CHECK-LABEL:   define internal void @proc1()

; This routine has basic instructions to transform. proc1 will be
; cloned but none of the instructions in cloned routine is transformed.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*

; Accessing 1st field, which is shrunken from i64 to i16.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
; CHECK: [[BC2:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP1:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC2]], i32 0, i32 5
; CHECK: %F1 = bitcast i16* [[GEP1]] to i64*

; Loading 1st field, which is shrunken from i64 to i16.
; No need for encoding since loaded value will be stored in the shrunken field.
  %LF1 = load i64, i64* %F1, align 8
; CHECK: [[BC3:%[0-9]+]] = bitcast i64* %F1 to i16*
; CHECK: [[LD1:%[0-9]+]] = load i16, i16* [[BC3]], align 2
; CHECK: %LF1 = zext i16 [[LD1]] to i64

; Accessing 6th field, which is shrunken.
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
; CHECK: [[BC8:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP2:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC8]], i32 0, i32 7
; CHECK: %F6 = bitcast i16* [[GEP2]] to i64*

; Storing 6th field, which is shrunken from i64 to i16.
; No need for decoding since stored value comes from the shrunken field.
  store i64 %LF1, i64* %F6, align 8
; CHECK: [[BC9:%[0-9]+]] = trunc i64 %LF1 to i16
; CHECK: [[BC10:%[0-9]+]] = bitcast i64* %F6 to i16*
; CHECK:  store i16 [[BC9]], i16* [[BC10]], align 2

  ret void
}

; This routine has basic instructions to transform. proc2 will be
; cloned but none of the instructions in cloned routine is transformed.
define void @proc2() {
  %call1 = tail call  i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*

; Accessing 1st field, which is shrunken from i64 to i16.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
; CHECK: [[BC1:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP1:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC1]], i32 0, i32 5
; CHECK: %F1 = bitcast i16* [[GEP1]] to i64*

; Loading 1st field, which is shrunken from i64 to i16. Needs decoding.
  %LF1 = load i64, i64* %F1, align 8
; CHECK: [[BC2:%[0-9]+]] = bitcast i64* %F1 to i16*
; CHECK: [[LD1:%[0-9]+]] = load i16, i16* [[BC2]], align 2
; CHECK: %LF1 = call i64 @__DYN_decoder(i16 [[LD1]])

; Accessing 6th field, which is shrunken.
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
; CHECK: [[BC3:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP2:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC3]], i32 0, i32 7
; CHECK: %F6 = bitcast i16* [[GEP2]] to i64*

; Storing 6th field, which is shrunken from i64 to i16. Needs encoding.
  store i64 300000, i64* %F6, align 8
; CHECK: [[CALL1:%[0-9]+]] = call i16 @__DYN_encoder(i64 300000)
; CHECK: [[BC4:%[0-9]+]] = bitcast i64* %F6 to i16*
; CHECK:  store i16 [[CALL1]], i16* [[BC4]], align 2

  ret void
}

; CHECK-LABEL: define internal void @init()

; This routine is selected as InitRoutine.
define void @init() {
; CHECK:   [[DALLOC1:%dyn.alloc[0-9]+]] = alloca i64
; CHECK:   [[DALLOC2:%dyn.alloc]] = alloca i8*
; CHECK:   [[DSAFE:%dyn.safe]] = alloca i8
; CHECK:   store i8 0, i8* [[DSAFE]]
; CHECK:   [[DMAX:%d.max]] = alloca i64
; CHECK:   store i64 0, i64* [[DMAX]]
; CHECK:   [[DMIN:%d.min]] = alloca i64
; CHECK:   store i64 65529, i64* [[DMIN]]

; Initialize nw.field_1
  store i64 2000000, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1), align 8
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
; CHECK:   [[LD1:%d.ld[0-9]+]] = load i64, i64* [[DMIN]]
; CHECK:   [[CMP1:%d.cmp[0-9]+]] = icmp slt i64 [[LD1]], %g2
; CHECK:   [[SEL1:%d.sel[0-9]+]] = select i1 [[CMP1]], i64 [[LD1]], i64 %g2
; CHECK:   store i64 [[SEL1]], i64* [[DMIN]]
; CHECK:   [[LD2:%d.ld[0-9]+]] = load i64, i64* [[DMAX]]
; CHECK:   [[CMP2:%d.cmp[0-9]+]] = icmp sgt i64 [[LD2]], %g2
; CHECK:   [[SEL2:%d.sel[0-9]+]] = select i1 [[CMP2]], i64 [[LD2]], i64 %g2
; CHECK:   store i64 [[SEL2]], i64* [[DMAX]]
  store i64 %g2, i64* %F6, align 8
  %L = load i64, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1)
  %S1 = mul i64 2, %L
; Makes sure no runtime range checks are generated by checking
; loads of min and max values before the next IR instruction.
; CHECK-NOT: load i64, i64* [[DMIN]]
; CHECK-NOT: load i64, i64* [[DMAX]]
; CHECK: %S2 = add i64 15, %S1
  store i64 %S1, i64* %F1, align 8
  %S2 = add i64 15, %S1
  store i64 %S2, i64* %F6, align 8
  %IC = icmp sgt i64 %L, 10000000
  %M2 = mul i64 -2, %L
  %S3 = select i1 %IC, i64 %M2, i64 -20000000
  store i64 %S3, i64* %F1, align 8
; Makes sure no runtime range checks are generated by checking
; loads of min and max values before the next IR instruction.
; CHECK-NOT: load i64, i64* [[DMIN]]
; CHECK-NOT: load i64, i64* [[DMAX]]
; CHECK: %End = load i64

  %End = load i64, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1)
; More runtime checks are generated here using DMIN and DMAX values at
; the end of init routine.

  ret void
}

; The return values come from parsing stores in init(),proc1() and proc2().
; Since fields are dependent they have the same set of constant to encode.
; From init():
;     @nw.field_1 = 2000000;
;     field_1 = unknown;
;     field_6 = unknown;
;     field_1 = @nw.field_1 * 2;
;     field_6 = @nw.field_1 * 2 + 15;
;     field_1 = (@nw.field_1 > 10000000) ? -2 * @nw.field_1 : -20000000;
; From main():
;     @nw.field_1 = 1000000;
; From proc2():
;     field_6 = 300000;
;
; Complete set of possible constant values for @nw.field_1 is
;   {-20000000, 1000000, 2000000}
; Complete set of possible constant values for field_1 and field_6 is
;   { 300000, 2000000, 2000015, 4000000, 4000015}
;

; CHECK-LABEL: define internal i16 @__DYN_encoder(i64 %0) #1 {
; CHECK: entry:
; CHECK:  [[CMP1:%[0-9]+]] = icmp ule i64 [[ARG:%[0-9]+]], 65529
; CHECK:  br i1 [[CMP1]], label %default, label %switch_bb
; CHECK: switch_bb:
; CHECK:    switch i64 %0, label %default [
; CHECK:      i64 -20000000, label %case
; CHECK:      i64 300000, label %case1
; CHECK:      i64 2000000, label %case2
; CHECK:      i64 2000015, label %case3
; CHECK:      i64 4000000, label %case4
; CHECK:      i64 4000015, label %case5
; CHECK:    ]
; CHECK:  default:
; CHECK:    [[TR1:%[0-9]+]] = trunc i64 [[ARG:%[0-9]+]] to i16
; CHECK:    br label %return
; CHECK:  return:
; CHECK:    %phival = phi i16 [ [[TR1]], %default ], [ -6, %case ], [ -5, %case1 ], [ -4, %case2 ], [ -3, %case3 ], [ -2, %case4 ], [ -1, %case5 ]
; CHECK:    ret i16 %phival
; CHECK:  case:
; CHECK:    br label %return
; CHECK:  case1:
; CHECK:    br label %return
; CHECK:  case2:
; CHECK:    br label %return
; CHECK:  case3:
; CHECK:    br label %return
; CHECK:  case4:
; CHECK:    br label %return

; CHECK-LABEL: define internal i64 @__DYN_decoder(i16 %0) #1 {
; CHECK: entry:
; CHECK:  [[CMP2:%[0-9]+]] = icmp ule i16 [[ARG:%[0-9]+]], -7
; CHECK:  br i1 [[CMP2]], label %default, label %switch_bb
; CHECK: switch_bb:
; CHECK:  switch i16 [[ARG:%[0-9]+]], label %default [
; CHECK:  i16 -6, label %case
; CHECK:  i16 -5, label %case1
; CHECK:  i16 -4, label %case2
; CHECK:  i16 -3, label %case3
; CHECK:  i16 -2, label %case4
; CHECK:  i16 -1, label %case5
; CHECK:  ]
; CHECK: default:
; CHECK:  [[TRUNC:%[0-9]+]] = zext i16 [[ARG]] to i64
; CHECK:  br label %return
; CHECK: return:
; CHECK:  %phival = phi i64 [ [[TRUNC]], %default ], [ -20000000, %case ], [ 300000, %case1 ], [ 2000000, %case2 ], [ 2000015, %case3 ], [ 4000000, %case4 ], [ 4000015, %case5 ]
; CHECK:  ret i64 %phival
; CHECK:case:
; CHECK:  br label %return
; CHECK:case1:
; CHECK:  br label %return
; CHECK:case2:
; CHECK:  br label %return
; CHECK:case3:
; CHECK:  br label %return
; CHECK:case4:
; CHECK:  br label %return
; CHECK:case5:
; CHECK:  br label %return
; CHECK:}

; CHECK: attributes #1 = { "min-legal-vector-width"="0" }

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  store i64 1000000, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1), align 8
  call void @init();
;  call void @proc1();
;  call void @proc2();
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)
attributes #0 = { "target-features"="+avx2" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64, i32 }
!8 = !{!"S", %struct.nw zeroinitializer, i32 2, !1, !2} ; { i32, i64 }

!intel.dtrans.types = !{!7, !8}
