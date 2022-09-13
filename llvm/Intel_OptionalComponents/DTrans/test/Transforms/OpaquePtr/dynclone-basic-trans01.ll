; This test verifies that basic transformations are done correctly for
; GEP/ByteFlattenedGEP/Load/Store instructions when DynClone transformation
; is triggered.

;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16. Mapping of fields
; between original and new layouts. Offsets are specified in
; between ( and ).
;  OldIndex                 NewIndex
;   0  (0)         -->         1  (8)
;   1  (8)         -->         5  (24)
;   2  (16)        -->         2  (12)
;   3  (20)        -->         3  (16)
;   4  (24)        -->         6  (26)
;   5  (32)        -->         0  (0)
;   6  (40)        -->         7  (28)
;   7  (48)        -->         4  (20)
;
; Size of Original %struct.test.01: 56
; Size after transformation: 30
;
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 , i32 }

; CHECK-LABEL:   define internal void @proc1()

; This routine has basic instructions to transform. proc1 will be
; cloned but none of the instructions in cloned routine is transformed.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*

; Accessing 0th field, which is not shrunken.
  %F0 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 0
; CHECK: [[BC1:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: %F0 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC1]], i32 0, i32 1


; Loading 0th field, which is not shrunken.
  %LF0 = load i32, i32* %F0, align 4
; CHECK: %LF0 = load i32, i32* %F0, align 4

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

; Accessing 2nd field, which is not shrunken.
  %F2 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 2
; CHECK: [[BC4:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: %F2 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC4]], i32 0, i32 2

; Loading 2nd field, which is not shrunken.
  %LF2 = load i32, i32* %F2, align 4
; CHECK: %LF2 = load i32, i32* %F2, align 4

; Accessing 3rd field, which is not shrunken.
  %F3 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 3
; CHECK: [[BC5:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: %F3 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC5]], i32 0, i32 3

; Loading 3rd field, which is not shrunken.
  %LF3 = load i32, i32* %F3, align 4
; CHECK:  %LF3 = load i32, i32* %F3, align 4

; Accessing 4th field, which is not shrunken.
  %F4 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 4
; CHECK: [[BC6:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: %F4 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC6]], i32 0, i32 6

; Storing 4th field, which is not shrunken.
  store i16 10, i16* %F4, align 2
; CHECK: store i16 10, i16* %F4, align 2

; Accessing 5th field, which is not shrunken.
  %F5 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 5
; CHECK: [[BC7:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: %F5 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC7]], i32 0, i32 0

; Storing 5th field, which is not shrunken.
  store i64* null, i64** %F5, align 8
; CHECK: store i64* null, i64** %F5, align 8

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

; Accessing 7th field, which is not shrunken.
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 7
; CHECK: [[BC11:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: %F7 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC11]], i32 0, i32 4

; Storing 7th field, which is not shrunken.
  store i32 10, i32* %F7, align 4
; CHECK: store i32 10, i32* %F7, align 4

  %bp2 = bitcast %struct.test.01* %tp2 to i8*
; Accessing 0th field.
  %BFF0 = getelementptr i8, i8* %bp2, i64 0
; CHECK: %BFF0 = getelementptr i8, i8* %bp2, i64 8

; Accessing 1st field.
  %BFF1 = getelementptr i8, i8* %bp2, i64 8
; CHECK: %BFF1 = getelementptr i8, i8* %bp2, i64 24

; Accessing 2nd field.
  %BFF2 = getelementptr i8, i8* %bp2, i64 16
; CHECK: %BFF2 = getelementptr i8, i8* %bp2, i64 12

; Accessing 3rd field.
  %BFF3 = getelementptr i8, i8* %bp2, i64 20
; CHECK: %BFF3 = getelementptr i8, i8* %bp2, i64 16

; Accessing 4th field.
  %BFF4 = getelementptr i8, i8* %bp2, i64 24
; CHECK: %BFF4 = getelementptr i8, i8* %bp2, i64 26

; Accessing 5th field.
  %BFF5 = getelementptr i8, i8* %bp2, i64 32
; CHECK: %BFF5 = getelementptr i8, i8* %bp2, i64 0

; Accessing 6th field.
  %BFF6 = getelementptr i8, i8* %bp2, i64 40
; CHECK: %BFF6 = getelementptr i8, i8* %bp2, i64 28

; Accessing 7th field.
  %BFF7 = getelementptr i8, i8* %bp2, i64 48
; CHECK: %BFF7 = getelementptr i8, i8* %bp2, i64 20

  ret void
}

; This routine has basic instructions to transform. proc2 will be
; cloned but none of the instructions in cloned routine is transformed.
define void @proc2() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
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

; Make sure none of instructions is transformed in cloned
; routine.
; CHECK-LABEL:   define internal void @proc1{{.*}}
; CHECK: %F0 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 0
; CHECK: %LF0 = load i32, i32* %F0, align 4
; CHECK: %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
; CHECK: %LF1 = load i64, i64* %F1, align 8
; CHECK: %F2 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 2
; CHECK: %LF2 = load i32, i32* %F2, align 4
; CHECK: %F3 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 3
; CHECK: %LF3 = load i32, i32* %F3, align 4
; CHECK: %F4 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 4
; CHECK: store i16 10, i16* %F4, align 2
; CHECK: %F5 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 5
; CHECK: store i64* null, i64** %F5, align 8
; CHECK: %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
; CHECK: store i64 %LF1, i64* %F6, align 8
; CHECK: %F7 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 7
; CHECK: store i32 10, i32* %F7, align 4
; CHECK: %BFF0 = getelementptr i8, i8* %bp2, i64 0
; CHECK: %BFF1 = getelementptr i8, i8* %bp2, i64 8
; CHECK: %BFF3 = getelementptr i8, i8* %bp2, i64 20
; CHECK: %BFF4 = getelementptr i8, i8* %bp2, i64 24
; CHECK: %BFF5 = getelementptr i8, i8* %bp2, i64 32
; CHECK: %BFF6 = getelementptr i8, i8* %bp2, i64 40
; CHECK: %BFF7 = getelementptr i8, i8* %bp2, i64 48

; This routine is selected as InitRoutine.
define void @init() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
;  call void @proc1();
;  call void @proc2();
  ret i32 0
}
; Function Attrs: nounwind
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64 , i32 }

!intel.dtrans.types = !{!7}
