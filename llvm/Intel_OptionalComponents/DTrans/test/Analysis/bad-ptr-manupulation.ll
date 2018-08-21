; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; The test checks propagation of 'Bad pointer manipulation' safety check from an array
; which is a structure field to its parent structure.

; TEST1
; Test that 'Bad pointer manipulation' was propogated.

%struct.A = type { i32, [10 x i32] }
%struct.B = type { i32, %struct.A }
%struct.C = type { [10 x i32], %struct.B }

@c = external dso_local global %struct.C, align 4

define dso_local i32 @foo1(i64 %arg) {
  entry:
  %gep = getelementptr inbounds %struct.C, %struct.C* @c, i32 0, i32 1, i32 1, i32 1
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %gep, i64 0, i64 %arg
  %ld = load i32, i32* %arrayidx, align 4
  ret i32 %ld
}

; CHECK-LABEL:  LLVMType: %struct.A
; CHECK:  Bad pointer manipulation | Nested structure
; CHECK:  LLVMType: %struct.B
; CHECK:  Bad pointer manipulation | Nested structure | Contains nested structure
; CHECK:  LLVMType: %struct.C
; CHECK:  Bad pointer manipulation | Contains nested structure

%struct.K = type { i32, [10 x i32] }
%struct.L = type { i32, %struct.K }
%struct.M = type { [10 x i32], %struct.L }

@m = external dso_local global %struct.M, align 4

define dso_local i32 @foo3(i64 %arg) {
  entry:
  %arrayidx = getelementptr inbounds %struct.M, %struct.M* @m, i32 0, i32 1, i32 1, i32 1, i64 %arg
  %ld = load i32, i32* %arrayidx, align 4
  ret i32 %ld
}

; CHECK-LABEL:  LLVMType: %struct.K
; CHECK:  Bad pointer manipulation | Nested structure
; CHECK:  LLVMType: %struct.L
; CHECK:  Bad pointer manipulation | Nested structure | Contains nested structure
; CHECK:  LLVMType: %struct.M
; CHECK:  Bad pointer manipulation | Contains nested structure

%struct.X = type { i32, [10 x i32] }
%struct.Y = type { i32, %struct.X }
%struct.Z = type { [10 x i32], %struct.Y }

@z = external dso_local global %struct.Z, align 4

define dso_local i32 @foo2(i64 %arg) {
  entry:
  %gep1 = getelementptr inbounds %struct.Z, %struct.Z* @z, i32 0, i32 1
  %gep2 = getelementptr inbounds %struct.Y, %struct.Y* %gep1, i32 0, i32 1
  %gep3 = getelementptr inbounds %struct.X, %struct.X* %gep2, i32 0, i32 1
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %gep3, i64 0, i64 %arg
  %ld = load i32, i32* %arrayidx, align 4
  ret i32 %ld
}

; CHECK-LABEL:  LLVMType: %struct.X
; CHECK:  Bad pointer manipulation | Nested structure
; CHECK:  LLVMType: %struct.Y
; CHECK:  Bad pointer manipulation | Nested structure | Contains nested structure
; CHECK:  LLVMType: %struct.Z
; CHECK:  Bad pointer manipulation | Contains nested structure


; TEST2
; Test that a char array access was not confused with a byte-flattened GEP
; causing 'Bad pointer manipulation'.

%struct._D = type { [10 x i8] }

@d = external dso_local global %struct._D, align 4

define dso_local i32 @foo4(i64 %arg) {
  entry:
  %gep1 = getelementptr inbounds %struct._D, %struct._D* @d, i64 0, i32 0
  %gep2 = getelementptr inbounds [10 x i8], [10 x i8]* %gep1, i64 0, i64 0
  %arrayidx = getelementptr inbounds i8, i8* %gep2, i64 %arg
  %ld = load i8, i8* %arrayidx, align 1
  %zext = zext i8 %ld to i32
  ret i32 %zext
}

; CHECK-LABEL:  LLVMType: %struct._D
; CHECK: No issues found

