; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; struct S1 {
;   int  x;
;   int  y;
;   int  z;
; };
%struct.S1 = type { i32, i32, i32 }

; struct S2 {
;   int        a[32];
;   struct S1  s1;
;   struct S2 *next;
; };
%struct.S2 = type { [32 x i32], %struct.S1, %struct.S2* }

; struct S3 {
;   struct S2 *ps2;
;   struct S2 *ps1;
; };
%struct.S3 = type { %struct.S2*, %struct.S1* }


define void @test(i32 %a, %struct.S3* %ps3) {
  %p = call noalias i8* @malloc(i64 152)
  %ps2 = bitcast i8* %p to %struct.S2*

  ; Read s2->s1.x via complete GEP
  %px.r = getelementptr %struct.S2, %struct.S2* %ps2, i64 0, i32 1, i32 0
  %x = load i32, i32* %px.r

  ; Write s2->s1.x via decomposed GEP
  %ps1 = getelementptr %struct.S2, %struct.S2* %ps2, i64 0, i32 1
  %px.w = getelementptr %struct.S1, %struct.S1* %ps1, i64 0, i32 0
  store i32 %a, i32* %px.w

  ; Get the address of s2->s1.y but don't do anything with it.
  %py = getelementptr %struct.S2, %struct.S2* %ps2, i64 0, i32 1, i32 1

  ; Write s2->s1.z via computed offset
  %pz.i8 = getelementptr i8, i8* %p, i64 136
  %pz = bitcast i8* %pz.i8 to i32*
  store i32 %x, i32* %pz

  ; Read s2->a[4] via computed offset
  %pa4.i8 = getelementptr i8, i8* %p, i64 16
  %pa4 = bitcast i8* %pa4.i8 to i32*
  %a4 = load i32, i32* %pa4

  ; Write s2->a[6] via complete GEP
  %pa6 = getelementptr %struct.S2, %struct.S2* %ps2, i64 0, i32 0, i32 6
  store i32 %x, i32* %pa4

  ; Store the address of s3->s2 in s2->next
  %pps3ps2 = getelementptr %struct.S3, %struct.S3* %ps3, i64 0, i32 0
  %ps3ps2.as.pi = bitcast %struct.S2** %pps3ps2 to i64*
  %s3ps2.as.i = load i64, i64* %ps3ps2.as.pi
  %ppnext = getelementptr %struct.S2, %struct.S2* %ps2, i64 0, i32 2
  %pnext.as.pi = bitcast %struct.S2** %ppnext to i64*
  store i64 %s3ps2.as.i, i64* %pnext.as.pi

  ; Store the address of s2 (as returned by malloc) in s3->s2
; This case isn't properly handled yet. (Multiple casts of malloc ret val.)
;  %ps2.as.i = ptrtoint i8* %p to i64
;  store i64 %ps2.as.i, i64* %ps3ps2.as.pi ; Address computed above

  ; Store the address of s2->s1 in s3->s1
  ; Using the address of s2->s1 from the GEP above (in the decomposed GEP case)
  %ps1.as.i = ptrtoint %struct.S1* %ps1 to i64
  %pps3ps1 = getelementptr %struct.S3, %struct.S3* %ps3, i64 0, i32 1
  %ps3ps1.as.pi = bitcast %struct.S1** %pps3ps1 to i64*
  store i64 %ps1.as.i, i64* %ps3ps1.as.pi

  ret void
}

declare noalias i8* @malloc(i64)

; TODO: Handle false safety issues (unimplemented) for the above cases.
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S1 = type { i32, i32, i32 }

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S2 = type { [32 x i32], %struct.S1, %struct.S2* }

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S3 = type { %struct.S2*, %struct.S1* }

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [32 x i32]

; TODO: Add checks for field access when that is implemented.
