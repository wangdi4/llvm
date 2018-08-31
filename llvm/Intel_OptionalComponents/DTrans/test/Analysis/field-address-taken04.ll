; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; TEST1
; Check that 'Field address taken' is assigned to correct structure.
%struct.F = type { i64, float, %struct.E }
%struct.E = type { i64, [10 x i8] }

@f = external dso_local global %struct.F, align 8
@ch = external dso_local local_unnamed_addr global i8*, align 8

define dso_local void @foo3(i64 %arg) local_unnamed_addr {
  entry:
  %gep = getelementptr inbounds %struct.F, %struct.F* @f, i64 0, i32 2, i32 1, i64 0
  store i8* %gep, i8** @ch, align 8
  ret void
}
; CHECK-LABEL:  LLVMType: %struct.E
; CHECK:  Safety data: Field address taken | Nested structure
; CHECK:  LLVMType: %struct.F
; CHECK:  Safety data: Contains nested structure


; TEST2
; Check that 'Field address taken' is not set when storing the address of the
; third element of the array.

%struct.G = type { i64, [10 x i8] }

@g = external dso_local global %struct.G, align 8

define dso_local void @foo4(i64 %arg) local_unnamed_addr {
  entry:
  %gep = getelementptr inbounds %struct.G, %struct.G* @g, i64 0, i32 1, i64 4
  store i8* %gep, i8** @ch, align 8
  ret void
}

; CHECK-LABEL:  LLVMType: %struct.G
; CHECK:  Safety data: No issues found

; TEST3
; Check that 'Fiend address taken' is correctly set for non-array case.

%struct.H = type { i64, i64 }

@h = external dso_local global %struct.H, align 8
@ptr = external dso_local local_unnamed_addr global i64*, align 8

define dso_local void @foo5(i64 %arg) local_unnamed_addr {
  entry:
  %gep = getelementptr inbounds %struct.H, %struct.H* @h, i64 0, i32 1
  store i64* %gep, i64** @ptr, align 8
  ret void
}

; CHECK-LABEL:  LLVMType: %struct.H
; CHECK:  Safety data: Field address taken

