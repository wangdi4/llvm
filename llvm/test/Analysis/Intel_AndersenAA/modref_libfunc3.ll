; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -aa-pipeline=anders-aa -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -aa-pipeline=anders-aa -evaluate-loopcarried-alias -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s

; This tests the basic functionality of Mod/Ref using the AndersenAA analysis
; to verify that calls to I/O library routines do not result in pointers that
; are not impacted by the call being treated as modified/referenced.

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = private unnamed_addr constant [13 x i8] c"Result = %d\0A\00", align 1
@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8

; This function will perform the memory allocations that need to be tracked
; via the Andersens alias analysis. And then use the pointers in a loop
; containing calls to printf and fflush to verify that the ModRef queries will
; not report the calls as modifying the array memory.
define internal i32 @test1() {
entry:
  %str1.ptr = getelementptr inbounds [13 x i8], [13 x i8]* @.str, i64 0, i64 0
  %ld.str1 = load i8, i8* %str1.ptr
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %call2 = tail call noalias i8* @malloc(i64 1024)
  %ld.call2 = load i8, i8* %call2
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1
  %ar2 = bitcast i8* %call2 to i32*
  %ld.ar2 = load i32, i32* %ar2
  br label %for.init.body

for.init.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.init.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %v = trunc i64 %indvars.iv to i32
  %v2 = mul nsw i32 %v, %v
  %arrayidx.init1 = getelementptr inbounds i32, i32* %ar1, i64 %indvars.iv
  %ld.arrayidx.init1 = load i32, i32* %arrayidx.init1
  %arrayidx.init2 = getelementptr inbounds i32, i32* %ar2, i64 %indvars.iv
  %ld.arrayidx.init2 = load i32, i32* %arrayidx.init2
  store i32 %v, i32* %arrayidx.init1
  store i32 %v2, i32* %arrayidx.init2

  %exitcond = icmp eq i64 %indvars.iv, 1024
  br i1 %exitcond, label %for.init.end, label %for.init.body
for.init.end:
  br label %for.test.body

for.test.body:
  %indvars.iv2 = phi i64 [ 0, %for.init.end ], [ %indvars.iv2.next, %for.test.body ]
  %res = phi i32 [0, %for.init.end ], [%sum, %for.test.body ]

  %arrayidx.test1 = getelementptr inbounds i32, i32* %ar1, i64 %indvars.iv2
  %ld.arrayidx.test1 = load i32, i32* %arrayidx.test1
  %arrayidx.test2 = getelementptr inbounds i32, i32* %ar2, i64 %indvars.iv2
  %ld.arrayidx.test2 = load i32, i32* %arrayidx.test2
  %elem1 = load i32, i32* %arrayidx.test1
  %elem2 = load i32, i32* %arrayidx.test2
  %sum = add nsw i32 %elem1, %elem2
  %indvars.iv2.next = add nuw nsw i64 %indvars.iv2, 1

  ; Invoke printf on the running total variable. No mod/ref of the malloc arrays.
  %tmp = call i32 (i8*, ...) @printf(i8*
    getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0),
    i32 %sum)
  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @stdout

  ; Invoke fflush on an external pointer. No mod/ref of the malloc arrays.
  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)

  %exitcond2 = icmp eq i64 %indvars.iv2, 1024
  br i1 %exitcond2, label %for.test.end, label %for.test.body
for.test.end:
  ret i32 %res
}

define dso_local i32 @main(i32 %argc, i8** nocapture readonly %argv) {
  %res = call i32 @test1()
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare dso_local i32 @printf(i8* nocapture readonly, ...)
declare dso_local i32 @fflush(%struct._IO_FILE* nocapture)

; CHECK-LABEL: Function: test1:
; CHECK-DAG: NoModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i8* %call2	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i32* %ar2	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.init1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.init2	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.test1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.test2	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: Just Ref:  Ptr: i8* %str1.ptr <->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 %sum)
; CHECK-DAG: NoModRef:  Ptr: i8* %call1	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i8* %call2	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i32* %ar1	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i32* %ar2	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.init1	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.init2	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.test1	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i32* %arrayidx.test2	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: NoModRef:  Ptr: i8* %str1.ptr	<->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
; CHECK-DAG: Both ModRef:  Ptr: %struct._IO_FILE** @stdout <->  %tmp1 = call i32 @fflush(%struct._IO_FILE* %fp)
