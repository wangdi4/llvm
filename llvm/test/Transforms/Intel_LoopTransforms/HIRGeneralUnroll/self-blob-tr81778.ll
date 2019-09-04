; The origin test case is regC/tr81778

; RUN: opt -hir-ssa-deconstruction -print-after=hir-general-unroll -disable-output -hir-general-unroll -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" -disable-output -hir-details < %s 2>&1 | FileCheck %s

; CHECK:  BEGIN REGION { modified }
; CHECK:       + DO i64 i1 = 8 * %tgu, %1, 1   <DO_LOOP>  <MAX_TC_EST = 7>
; CHECK:       | <RVAL-REG> LINEAR i64 8 * %tgu {sb:2}
; CHECK:       |    <BLOB> LINEAR i64 %tgu {sb:12}
; CHECK:       | <RVAL-REG> LINEAR i64 %1 {sb:4}
;                                      ^ No blob DDRef for self blobs
; CHECK:       | <ZTT-REG> LINEAR i64 8 * %tgu {sb:2}
; CHECK:       |    <BLOB> LINEAR i64 %tgu {sb:12}
; CHECK:       | <ZTT-REG> LINEAR i64 %1 + 1 {sb:2}
; CHECK:       |    <BLOB> LINEAR i64 %1 {sb:4}
;                                     ^ Here should be a blob ddref as %1 + 1 is no longer a self-blob


;Module Before HIR; ModuleID = 'tr81778.c'
source_filename = "tr81778.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@stdout = external local_unnamed_addr global %struct._IO_FILE*, align 8
@mapchar = common local_unnamed_addr global [256 x i8] zeroinitializer, align 16
@.str = private unnamed_addr constant [8 x i8] c"Passed\0A\00", align 1
@str = private unnamed_addr constant [7 x i8] c"Passed\00"

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #0 {
entry:
  %0 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %call = tail call i32 @fflush(%struct._IO_FILE* %0)
  %cmp = icmp eq i32 %argc, 1
  %1 = select i1 %cmp, i64 255, i64 127
  br label %do.body

do.body:                                          ; preds = %entry, %do.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %do.body ]
  %arrayidx = getelementptr inbounds [256 x i8], [256 x i8]* @mapchar, i64 0, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i8
  store i8 %2, i8* %arrayidx, align 1
  %cmp7 = icmp eq i64 %indvars.iv, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp7, label %if.end9, label %do.body

if.end9:                                          ; preds = %do.body
  %3 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %call10 = tail call i32 @fflush(%struct._IO_FILE* %3)
  %puts = tail call i32 @puts(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @str, i64 0, i64 0))
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare i32 @fflush(%struct._IO_FILE* nocapture) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) #3

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }


