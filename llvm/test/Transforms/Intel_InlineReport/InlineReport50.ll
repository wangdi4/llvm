; Inline report
; RUN: opt < %s -inline -inline-report=0xe807 -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that non-inlining of PrintFloats is due to it being a VarArgs
; function

; CHECK: COMPILE FUNC: PrintFloats
; CHECK: EXTERN: printf
; CHECK: llvm.va_start{{.*}}Callee is intrinsic
; CHECK: EXTERN: printf
; CHECK: llvm.va_end{{.*}}Callee is intrinsic
; CHECK: EXTERN: putchar

; CHECK: COMPILE FUNC: main
; CHECK: PrintFloats{{.*}}Callee is varargs

%struct.__va_list_tag = type { i32, i32, i8*, i8* }
@.str = private unnamed_addr constant [17 x i8] c"Printing floats:\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c" [%.2f]\00", align 1

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare dso_local i32 @printf(i8*, ...)
declare void @llvm.va_start(i8*)
declare void @llvm.va_end(i8*)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
declare i32 @putchar(i32)

define dso_local void @PrintFloats(i32 %n, ...) local_unnamed_addr {
entry:
  %vl = alloca [1 x %struct.__va_list_tag], align 16
  %call = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i64 0, i64 0))
  %0 = bitcast [1 x %struct.__va_list_tag]* %vl to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %0) #3
  %arraydecay1 = bitcast [1 x %struct.__va_list_tag]* %vl to i8*
  call void @llvm.va_start(i8* %arraydecay1)
  br label %for.cond

for.cond:                                         ; preds = %vaarg.end, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %vaarg.end ]
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %fp_offset_p = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %vl, i64 0, i64 0, i32 1
  %fp_offset = load i32, i32* %fp_offset_p, align 4
  %fits_in_fp = icmp ult i32 %fp_offset, 161
  br i1 %fits_in_fp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %for.body
  %1 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %vl, i64 0, i64 0, i32 3
  %reg_save_area = load i8*, i8** %1, align 16
  %2 = sext i32 %fp_offset to i64
  %3 = getelementptr i8, i8* %reg_save_area, i64 %2
  %4 = add i32 %fp_offset, 16
  store i32 %4, i32* %fp_offset_p, align 4
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %for.body
  %overflow_arg_area_p = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %vl, i64 0, i64 0, i32 2
  %overflow_arg_area = load i8*, i8** %overflow_arg_area_p, align 8
  %overflow_arg_area.next = getelementptr i8, i8* %overflow_arg_area, i64 8
  store i8* %overflow_arg_area.next, i8** %overflow_arg_area_p, align 8
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr.in = phi i8* [ %3, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %vaarg.addr = bitcast i8* %vaarg.addr.in to double*
  %5 = load double, double* %vaarg.addr, align 8
  %call3 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), double %5)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.va_end(i8* nonnull %arraydecay1)
  %putchar = call i32 @putchar(i32 10)
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %0) #3
  ret void
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  call void (i32, ...) @PrintFloats(i32 3, double 3.141590e+00, double 2.718280e+00, double 1.414210e+00)
  ret i32 0
}
