; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S -vpo-paropt-use-interop=false %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S -vpo-paropt-use-interop=false %s | FileCheck %s
; Test for TARGET VARIANT DISPATCH construct where the base/variant functions
; are VarArg

; C source:
;
; #include <stdarg.h>
; #include <stdio.h>
; int foo_gpu(int num, ...) {
;   int i, sum = 0;
;   va_list argp;
;   va_start(argp, num);
;   for (i=0; i<num; i++)
;     sum -= va_arg(argp, int);
;   va_end(argp);
;   return sum;
; }
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int foo(int num, ...) {
;   int i, sum = 0;
;   va_list argp;
;   va_start(argp, num);
;   for (i=0; i<num; i++)
;     sum += va_arg(argp, int);
;   va_end(argp);
;   return sum;
; }
; int main() {
;   int rrr;
;   #pragma omp target variant dispatch device(0)
;      rrr = foo(2, 111, 222);
;   return rrr;
; }

; Check that the variant call is emitted:
; CHECK: %variant = call i32 (i32, ...) @foo_gpu(i32 2, i32 111, i32 222)

source_filename = "target_variant_dispatch_vararg.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.__va_list_tag = type { i32, i32, ptr, ptr }

define dso_local noundef i32 @main()  {
entry:
  %retval = alloca i32, align 4
  %rrr = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %call = call noundef i32 (i32, ...) @foo(i32 noundef 2, i32 noundef 111, i32 noundef 222) #4 [ "QUAL.OMP.DISPATCH.CALL"() ]
  store i32 %call, ptr %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]

  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %1 = load i32, ptr %rrr, align 4
  ret i32 %1
}

define dso_local noundef i32 @foo_gpu(i32 noundef %num, ...)  {
entry:
  %num.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %argp = alloca [1 x %struct.__va_list_tag], align 16
  store i32 %num, ptr %num.addr, align 4
  store i32 0, ptr %sum, align 4
  %arraydecay = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %argp, i64 0, i64 0
  call void @llvm.va_start(ptr %arraydecay)
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %1 = load i32, ptr %num.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %arraydecay1 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %argp, i64 0, i64 0
  %gp_offset_p = getelementptr inbounds %struct.__va_list_tag, ptr %arraydecay1, i32 0, i32 0
  %gp_offset = load i32, ptr %gp_offset_p, align 16
  %fits_in_gp = icmp ule i32 %gp_offset, 40
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %for.body
  %2 = getelementptr inbounds %struct.__va_list_tag, ptr %arraydecay1, i32 0, i32 3
  %reg_save_area = load ptr, ptr %2, align 16
  %3 = getelementptr i8, ptr %reg_save_area, i32 %gp_offset
  %4 = add i32 %gp_offset, 8
  store i32 %4, ptr %gp_offset_p, align 16
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %for.body
  %overflow_arg_area_p = getelementptr inbounds %struct.__va_list_tag, ptr %arraydecay1, i32 0, i32 2
  %overflow_arg_area = load ptr, ptr %overflow_arg_area_p, align 8
  %overflow_arg_area.next = getelementptr i8, ptr %overflow_arg_area, i32 8
  store ptr %overflow_arg_area.next, ptr %overflow_arg_area_p, align 8
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr = phi ptr [ %3, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %5 = load i32, ptr %vaarg.addr, align 4
  %6 = load i32, ptr %sum, align 4
  %sub = sub nsw i32 %6, %5
  store i32 %sub, ptr %sum, align 4
  br label %for.inc

for.inc:                                          ; preds = %vaarg.end
  %7 = load i32, ptr %i, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  %arraydecay2 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %argp, i64 0, i64 0
  call void @llvm.va_end(ptr %arraydecay2)
  %8 = load i32, ptr %sum, align 4
  ret i32 %8
}

declare void @llvm.va_start(ptr)

declare void @llvm.va_end(ptr)

define dso_local noundef i32 @foo(i32 noundef %num, ...) #2 {
entry:
  %num.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %argp = alloca [1 x %struct.__va_list_tag], align 16
  store i32 %num, ptr %num.addr, align 4
  store i32 0, ptr %sum, align 4
  %arraydecay = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %argp, i64 0, i64 0
  call void @llvm.va_start(ptr %arraydecay)
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %1 = load i32, ptr %num.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %arraydecay1 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %argp, i64 0, i64 0
  %gp_offset_p = getelementptr inbounds %struct.__va_list_tag, ptr %arraydecay1, i32 0, i32 0
  %gp_offset = load i32, ptr %gp_offset_p, align 16
  %fits_in_gp = icmp ule i32 %gp_offset, 40
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %for.body
  %2 = getelementptr inbounds %struct.__va_list_tag, ptr %arraydecay1, i32 0, i32 3
  %reg_save_area = load ptr, ptr %2, align 16
  %3 = getelementptr i8, ptr %reg_save_area, i32 %gp_offset
  %4 = add i32 %gp_offset, 8
  store i32 %4, ptr %gp_offset_p, align 16
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %for.body
  %overflow_arg_area_p = getelementptr inbounds %struct.__va_list_tag, ptr %arraydecay1, i32 0, i32 2
  %overflow_arg_area = load ptr, ptr %overflow_arg_area_p, align 8
  %overflow_arg_area.next = getelementptr i8, ptr %overflow_arg_area, i32 8
  store ptr %overflow_arg_area.next, ptr %overflow_arg_area_p, align 8
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr = phi ptr [ %3, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %5 = load i32, ptr %vaarg.addr, align 4
  %6 = load i32, ptr %sum, align 4
  %add = add nsw i32 %6, %5
  store i32 %add, ptr %sum, align 4
  br label %for.inc

for.inc:                                          ; preds = %vaarg.end
  %7 = load i32, ptr %i, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond, !llvm.loop !6

for.end:                                          ; preds = %for.cond
  %arraydecay2 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %argp, i64 0, i64 0
  call void @llvm.va_end(ptr %arraydecay2)
  %8 = load i32, ptr %sum, align 4
  ret i32 %8
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #2 = { "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
