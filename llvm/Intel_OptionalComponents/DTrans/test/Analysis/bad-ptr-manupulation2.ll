; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; The test checks that 'Bad pointer manipulation' is not assigned on simple array element store.

%struct.str = type { i32, i32, [2 x [40 x i8]] }
@a = external dso_local global %struct.str, align 8

define dso_local i8 @foo() {
  entry:
  %structgep = getelementptr inbounds %struct.str, %struct.str* @a, i64 0, i32 2
  %arrgep = getelementptr inbounds [2 x [40 x i8]], [2 x [40 x i8]]* %structgep, i64 0, i64 1
  %gep = getelementptr inbounds [40 x i8], [40 x i8]* %arrgep, i64 0, i64 12
  %arrayidx = getelementptr inbounds i8, i8* %gep, i64 8
  %ld = load i8, i8* %arrayidx, align 4
  ret i8 %ld
}

; CHECK-LABEL:  LLVMType: %struct.str
; CHECK:  Safety data: No issues found
; CHECK:  LLVMType: [2 x [40 x i8]]
; CHECK:  LLVMType: [40 x i8]

