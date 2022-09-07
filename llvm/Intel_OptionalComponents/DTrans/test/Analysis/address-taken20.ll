; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Check that %struct.s2 has <incomplete> results in fields 0 and 2,
; because @getenv creates a false alias. Also check that %struct.s1 and
; %struct.s2 are Address taken.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.s1 = type { i32, i32 }
; CHECK: Name: struct.s1
; CHECK: Safety data:{{.*}}Address taken{{.*}}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.s2 = type { i32 (%struct.s2*)*, %struct.s1*, i32 (%struct.s2*)* }
; CHECK: Name: struct.s2
; CHECK: 0)Field LLVM Type: i32 (%struct.s2*)*
; CHECK: Multiple Value: [ @f1, null ] <incomplete>
; CHECK: 2)Field LLVM Type: i32 (%struct.s2*)*
; CHECK: Multiple Value: [ @f2, null ] <incomplete>
; CHECK: Safety data:{{.*}}Address taken{{.*}}
; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [16 x i8]
; CHECK: Safety data: No issues found

$pcstring = comdat any
@pcstring = internal unnamed_addr constant [16 x i8] c"POSIXLY_CORRECT\00", comdat, align 1
@posixly_correct = internal unnamed_addr global i8* null, align 8

%struct.s1 = type {i32, i32}
%struct.s2 = type {i32 (%struct.s2*)*, %struct.s1*, i32 (%struct.s2*)*}
@SV = internal global %struct.s1 zeroinitializer
@GV = internal global %struct.s2 zeroinitializer

declare dso_local i8* @getenv(...) local_unnamed_addr

define dso_local i32 @main(i32 %0, i8** %1) {
  %result1 = tail call i8* bitcast (i8* (...)* @getenv to i8* (i8*)*)(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @pcstring, i64 0, i64 0))
  store i8* %result1, i8** @posixly_correct, align 8
  %result2 = tail call i8* bitcast (i8* (...)* @getenv to i8* (i8*, i32)*)(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @pcstring, i64 0, i64 0), i32 5)
  store i8* %result2, i8** @posixly_correct, align 8
  %first = getelementptr inbounds %struct.s2, %struct.s2* @GV, i64 0, i32 0
  store i32 (%struct.s2*)* @f1, i32 (%struct.s2*)** %first, align 8
  %second = getelementptr inbounds %struct.s2, %struct.s2* @GV, i64 0, i32 2
  store i32 (%struct.s2*)* @f2, i32 (%struct.s2*)** %second, align 8
  %third = getelementptr inbounds %struct.s2, %struct.s2* @GV, i64 0, i32 0
  %value = load i32 (%struct.s2*)*, i32 (%struct.s2*)** %third, align 8
  %rv = tail call i32 %value(%struct.s2* @GV)
  ret i32 %rv
}

define internal i32 @f1(%struct.s2* %arg) {
   %first = getelementptr inbounds %struct.s2, %struct.s2* %arg, i64 0, i32 1
   %second = load %struct.s1*, %struct.s1** %first, align 8
   %third = getelementptr inbounds %struct.s1, %struct.s1* %second, i64 0, i32 0
   %fourth = getelementptr inbounds %struct.s1, %struct.s1* %second, i64 0, i32 1
   %v1 = load i32, i32* %third
   %v2 = load i32, i32* %fourth
   %result = add nsw i32 %v1, %v2
   ret i32 %result
}

define internal i32 @f2(%struct.s2* %arg) {
   %first = getelementptr inbounds %struct.s2, %struct.s2* %arg, i64 0, i32 1
   %second = load %struct.s1*, %struct.s1** %first, align 8
   %third = getelementptr inbounds %struct.s1, %struct.s1* %second, i64 0, i32 0
   %fourth = getelementptr inbounds %struct.s1, %struct.s1* %second, i64 0, i32 1
   %v1 = load i32, i32* %third
   %v2 = load i32, i32* %fourth
   %result = sub nsw i32 %v1, %v2
   ret i32 %result
}
