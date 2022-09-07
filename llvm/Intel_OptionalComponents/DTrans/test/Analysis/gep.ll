; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

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

; Test the cases where the GEP can be decoded cleanly.
define void @test_good_cases(i32 %a, %struct.S3* %ps3) {
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
  store i32 %x, i32* %pa6

  ; Store the address of s3->s2 in s2->next
  %pps3ps2 = getelementptr %struct.S3, %struct.S3* %ps3, i64 0, i32 0
  %ps3ps2.as.pi = bitcast %struct.S2** %pps3ps2 to i64*
  %s3ps2.as.i = load i64, i64* %ps3ps2.as.pi
  %ppnext = getelementptr %struct.S2, %struct.S2* %ps2, i64 0, i32 2
  %pnext.as.pi = bitcast %struct.S2** %ppnext to i64*
  store i64 %s3ps2.as.i, i64* %pnext.as.pi

  ; Store the address of s2 (as returned by malloc) in s3->s2
  %ps2.as.i = ptrtoint i8* %p to i64
  store i64 %ps2.as.i, i64* %ps3ps2.as.pi ; Address computed above

  ; Store the address of s2->s1 in s3->s1
  ; Using the address of s2->s1 from the GEP above (in the decomposed GEP case)
  %ps1.as.i = ptrtoint %struct.S1* %ps1 to i64
  %pps3ps1 = getelementptr %struct.S3, %struct.S3* %ps3, i64 0, i32 1
  %ps3ps1.as.pi = bitcast %struct.S1** %pps3ps1 to i64*
  store i64 %ps1.as.i, i64* %ps3ps1.as.pi

  ; Use the s3 pointer as a dynamic array to get a different pointer of the
  ; same type.
  %ps3too = getelementptr %struct.S3, %struct.S3* %ps3, i64 3

  ; Use an index that was loaded at runtime to index into the s3 pointer as a
  ; dynamic array.
  %idx = zext i32 %x to i64
  %ps3unkown = getelementptr %struct.S3, %struct.S3* %ps3, i64 %idx

  ; Allocate an array of pointers to S1 and get the address of the fourth ptr
  ; by way of a bitcast.
  %pp = call noalias i8* @malloc(i64 80)
  %pps1 = bitcast i8* %pp to %struct.S1**
  %p4s1 = getelementptr i8, i8* %pp, i64 24

  ret void
}

; Test the special case where a structures with an i8 value at the start
; of their memory can have a GEP that acts like a bitcast.
%struct.S4 = type { i8, i32 }
%struct.S5 = type { %struct.S4, i32 }
%struct.S6 = type { [256 x i8], i32 }
@g.testS4 = internal unnamed_addr global %struct.S4 zeroinitializer
@g.testS5 = internal unnamed_addr global %struct.S5 zeroinitializer
define void @test_gep_as_bitcast(%struct.S6 *%pS6) {
  ; The GEP returns an i8* which is then cast back to the original type
  %t1 = bitcast i8* getelementptr (%struct.S5, %struct.S5* @g.testS5,
                                   i64 0, i32 0, i32 0) to %struct.S5*

  ; This is just like the line above, but it casts to the nested S4 type
  %t2 = bitcast i8* getelementptr (%struct.S5, %struct.S5* @g.testS5,
                                   i64 0, i32 0, i32 0) to %struct.S4*

  ; This GEP acts like a bitcast on the S4 global.
  %t3 = getelementptr %struct.S4, %struct.S4* @g.testS4, i64 0, i32 0

  ; If the %pS6 argument is a dynamic array, we can index from the pointer
  ; to additional S6 pointers and still use the GEP-as-bitcast idiom.
  %t4 = getelementptr %struct.S6, %struct.S6* %pS6,
                      i64 3, i32 0, i32 0
  %t5 = bitcast i8* %t4 to %struct.S6*
  ret void
}

; Test cases where GEP operators are used to access globals.
%struct.S7 = type { i32, i32 }
@g_instance.S7 = internal unnamed_addr global %struct.S7 zeroinitializer
%struct.S8 = type { i32, i32 }
@g_instance.S8 = internal unnamed_addr global %struct.S8 zeroinitializer
%struct.S9 = type { i32, i32 }
@g_instance.S9 = internal unnamed_addr global %struct.S9 zeroinitializer

define void @test_gep_operators() {
  ; Load the value of g_instance.S7.a and store it in g_instance.S7.b.
  %s7.a = load i32, i32* getelementptr (%struct.S7, %struct.S7* @g_instance.S7,
                                        i64 0, i32 0)
  store i32 %s7.a, i32* getelementptr (%struct.S7, %struct.S7* @g_instance.S7,
                                       i64 0, i32 1)

  ; Load the value of g_instance.S8.b and store it in g_instance.S8.a using
  ; intermediate bitcast instructions.
  %tmp1 = bitcast i32* getelementptr (%struct.S8, %struct.S8* @g_instance.S8,
                                      i64 0, i32 0) to i8*
  %ps8.a = bitcast i8* %tmp1 to i32*
  %tmp2 = bitcast i32* getelementptr (%struct.S8, %struct.S8* @g_instance.S8,
                                      i64 0, i32 1) to i8*
  %ps8.b = bitcast i8* %tmp2 to i32*
  %b = load i32, i32* %ps8.b
  store i32 %b, i32* %ps8.a

  ; Use byte-flattened GEP operator via bitcast operator access fields of
  ; g_instance.S9
  %ps9.a = bitcast i8* getelementptr (i8,
                                      i8* bitcast (%struct.S9* @g_instance.S9
                                        to i8*), i64 0) to i32*
  %ps9.b = bitcast i8* getelementptr (i8,
                                      i8* bitcast (%struct.S9* @g_instance.S9
                                        to i8*), i64 4) to i32*
  %a = load i32, i32* %ps9.a
  store i32 %a, i32* %ps9.b
  store i32 0, i32* %ps9.a

  ret void
}

; The redundant types here are to allow separate testing of safety conditions.

; struct S1 {
;   int  x;
;   int  y;
;   int  z;
; };
%struct.bad.S1 = type { i32, i32, i32 }

; struct S2 {
;   int  x;
;   int  y;
; };
%struct.bad.S2 = type { i32, i32 }

; struct S3 {
;   int n;
;   struct S2;
; };
%struct.bad.S3 = type { i32, %struct.bad.S2 }

; struct S4 {
;   int  x;
;   int  y;
; };
%struct.bad.S4 = type { i32, i32 }

; struct S5 {
;   int x;
;   int y;
;   int z;
; };
%struct.bad.S5 = type { i32, i32, i32 }

; struct S6 {
;   int  x;
;   int  y;
; };
%struct.bad.S6 = type { i32, i32 }

; struct S7 {
;   int n;
;   struct S6 s6[4];
; };
%struct.bad.S7 = type { i32, [4 x %struct.bad.S6] }

; Test cases where byte-flattened GEP access doesn't align with an actual
; element in the structure and where the structure being referenced is
; ambiguous.
define void @test_bad_cases(%struct.bad.S1* %ps1, %struct.bad.S3* %ps3,
                            %struct.bad.S4* %ps4, %struct.bad.S5* %ps5,
                            %struct.bad.S7* %ps7) {
  %tmp1 = bitcast %struct.bad.S1* %ps1 to i8*
  %bad1 = getelementptr i8, i8* %tmp1, i64 2

  %tmp3 = bitcast %struct.bad.S3* %ps3 to i8*
  ; This uses an offset in the nested %struct.bad.S2 type, so both
  ; %struct.bad.S2 and %struct.bad.S3 should get a bad pointer manipulation
  ; flag because of this instruction.
  %bad3 = getelementptr i8, i8* %tmp3, i64 6

  %tmp4 = bitcast %struct.bad.S4* %ps4 to i8*
  %tmp5 = bitcast %struct.bad.S5* %ps5 to i8*
  %tmpSel = select i1 undef, i8* %tmp4, i8* %tmp5
  ; This would be a valid offset for either structure, but since it is an
  ; ambiguous reference, we need to treat it as a safety issue.
  %badSel = getelementptr i8, i8* %tmpSel, i64 4

  %tmp7 = bitcast %struct.bad.S7* %ps7 to i8*
  ; This uses an offset in the nested array of %struct.bad.S6 type, so
  ; [4 x %struct.bad.S6], %struct.bad.S6, and %struct.bad.S7 should all get
  ; a bad pointer manipulation flag because of this instruction.
  %bad7 = getelementptr i8, i8* %tmp7, i64 6

  ret void
}

declare noalias i8* @malloc(i64)

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S1 = type { i32, i32, i32 }
; CHECK: Field info: Read Written
; CHECK: Field info:
; CHECK-NOT: Read
; CHECK-NOT: Written
; CHECK: Frequency
; CHECK: Field info: Written
; CHECK: Safety data: Field address taken memory | Nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S2 = type { [32 x i32], %struct.S1, %struct.S2* }
; CHECK: Field info:
; CHECK-NOT: Read
; CHECK-NOT: Written
; CHECK: Frequency
; CHECK: Field info:
; CHECK-NOT: Read
; CHECK-NOT: Written
; CHECK: Frequency
; CHECK: Field info: Written
; CHECK: Safety data: Field address taken memory | Contains nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S3 = type { %struct.S2*, %struct.S1* }
; CHECK: Field info: Read Written
; CHECK: Field info: Written
; CHECK: Safety data: No issues found

; CHECK: LLVMType: %struct.S4 = type { i8, i32 }
; CHECK: Safety data: Global instance | Nested structure
; CHECK: LLVMType: %struct.S5 = type { %struct.S4, i32 }
; CHECK: Safety data: Global instance | Contains nested structure
; CHECK: LLVMType: %struct.S6 = type { [256 x i8], i32 }
; CHECK: Safety data: No issues found

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S7 = type { i32, i32 }
; CHECK: Field info: Read
; CHECK: Field info: Written
; CHECK: Safety data: Global instance

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S8 = type { i32, i32 }
; CHECK: Field info: Written
; CHECK: Field info: Read
; CHECK: Safety data: Global instance

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.S9 = type { i32, i32 }
; CHECK: Field info: Read Written
; CHECK: Field info: Written
; CHECK: Safety data: Global instance

; CHECK: LLVMType: %struct.bad.S1 = type { i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation
; CHECK: LLVMType: %struct.bad.S2 = type { i32, i32 }
; CHECK: Safety data: Bad pointer manipulation | Nested structure
; CHECK: LLVMType: %struct.bad.S3 = type { i32, %struct.bad.S2 }
; CHECK: Safety data: Bad pointer manipulation | Contains nested structure
; CHECK: LLVMType: %struct.bad.S4 = type { i32, i32 }
; CHECK: Safety data: Ambiguous GEP
; CHECK: LLVMType: %struct.bad.S5 = type { i32, i32, i32 }
; CHECK: Safety data: Ambiguous GEP
; CHECK: LLVMType: %struct.bad.S6 = type { i32, i32 }
; CHECK: Safety data: Bad pointer manipulation | Nested structure
; CHECK: LLVMType: %struct.bad.S7 = type { i32, [4 x %struct.bad.S6] }
; CHECK: Safety data: Bad pointer manipulation | Contains nested structure

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [32 x i32]
; CHECK: Safety data: No issues found
; CHECK: LLVMType: [4 x %struct.bad.S6]
; CHECK: Safety data: Bad pointer manipulation

; TODO: Add checks for field access when that is implemented.
