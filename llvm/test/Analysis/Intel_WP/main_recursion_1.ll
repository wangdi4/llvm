; This test checks that norecurs attribute is set for main routine
; also in addition to all other 4 user defined routines (allocate, free,
; dealloc, assign) by "Deduce function attributes". "norecuse" attribute
; is set to main only when whole-program-safe is detected and no uses of
; main are noticed.

; RUN: opt < %s -wholeprogramanalysis -functionattrs -rpo-functionattrs -disable-output -stats 2>&1 | FileCheck %s
; REQUIRES: asserts

; CHECK:   5 functionattrs - Number of functions marked as norecurse


%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@stderr = external global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [10 x i8] c"ptr: %d \0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define internal i8* @allocate()  {
entry:
  %call = call noalias i8* @malloc(i64 8) 
  ret i8* %call
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) 

; Function Attrs: noinline nounwind uwtable
define internal void @assign(i8* %ptr)  {
entry:
  %ptr.addr = alloca i8*, align 8
  store i8* %ptr, i8** %ptr.addr, align 8
  %0 = load i8*, i8** %ptr.addr, align 8
  %1 = bitcast i8* %0 to i32*
  store i32 10, i32* %1, align 4
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @dump(i8* %ptr)  {
entry:
  %ptr.addr = alloca i8*, align 8
  store i8* %ptr, i8** %ptr.addr, align 8
  %0 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8
  %1 = load i8*, i8** %ptr.addr, align 8
  %2 = bitcast i8* %1 to i32*
  %3 = load i32, i32* %2, align 4
  %call = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %0, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i32 0, i32 0), i32 %3)
  ret void
}

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...) 


; Function Attrs: noinline nounwind uwtable
define internal void @dealloc(i8* %ptr)  {
entry:
  %ptr.addr = alloca i8*, align 8
  store i8* %ptr, i8** %ptr.addr, align 8
  %0 = load i8*, i8** %ptr.addr, align 8
  call void @free(i8* %0) 
  ret void
}

; Function Attrs: nounwind
declare void @free(i8*) 

; Function Attrs: nounwind uwtable
define i32 @main()  {
entry:
  %ptr1 = alloca i8*, align 8
  %call = call i8* @allocate()
  store i8* %call, i8** %ptr1, align 8
  %0 = load i8*, i8** %ptr1, align 8
  call void @assign(i8* %0)
  %1 = load i8*, i8** %ptr1, align 8
  call void @dump(i8* %1)
  %2 = load i8*, i8** %ptr1, align 8
  call void @dealloc(i8* %2)
  ret i32 0
}
