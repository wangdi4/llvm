; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Check that functions like @foo are partially inlined if called in leaf
; functions of long consecutive call chains.
; RUN: opt < %s -passes=partial-inliner -force-run-lto-partial-inline -S | FileCheck %s
; Remove the annotation added by long consecutive call chains heuristic and
; check that partial inlining is not triggered.
; RUN: sed 's/#0$//g' %s | opt -passes=partial-inliner -force-run-lto-partial-inline -S | FileCheck --check-prefix=NO-PARTIAL-INLINE %s
; The shared build has much less limit to PartialInliner and the negative
; check above won't work there. Thus the test is for intel_feature_sw_advanced
; only.

; The IR is generated from the following source code:
; #include <stdarg.h>
; #include <stdio.h>
; #include <stdlib.h>
;
; static int status = 0;
;
; __attribute__((noinline)) int foo(unsigned char opcode, unsigned char *buf,
;                                   ...) {
;   int i, size;
;   va_list args;
;   double d;
;   unsigned long ul;

;   if (!status)
;     return 0;

;   va_start(args, buf);

;   buf[0] = opcode;
;   switch (opcode) {
;   case 1:
;     ul = va_arg(args, unsigned long);
;     *((unsigned long *)&buf[1]) = ul;
;     size = sizeof(unsigned long) + 1;
;     break;
;   case 2:
;   case 3:
;     i = va_arg(args, int);
;     *((int *)&buf[1]) = i;
;     i = va_arg(args, int);
;     *((int *)&buf[sizeof(int) + 1]) = i;
;     size = 2 * sizeof(int) + 1;
;     break;
;   case 4:
;     i = va_arg(args, int);
;     *((int *)&buf[1]) = i;
;     i = va_arg(args, int);
;     *((int *)&buf[sizeof(int) + 1]) = i;
;     i = va_arg(args, int);
;     *((int *)&buf[2 * sizeof(int) + 1]) = i;
;     size = 3 * sizeof(int) + 1;
;     break;
;   case 5:
;   case 6:
;   case 7:
;   case 8:
;   case 9:
;   case 10:
;   case 11:
;     i = va_arg(args, int);
;     *((int *)&buf[1]) = i;
;     size = sizeof(int) + 1;
;     break;
;   case 12:
;   case 13:
;   case 14:
;   case 15:
;   case 16:
;     i = va_arg(args, int);
;     d = va_arg(args, double);
;     *((int *)&buf[1]) = i;
;     *((double *)&buf[sizeof(int) + 1]) = d;
;     size = sizeof(int) + sizeof(double) + 1;
;     break;
;   default:
;     exit(1);
;   }
;   return 1;
; }

; void bar(int *data, int size, unsigned char *buf) {
;   if (foo(9, buf, 1))
;     return;
;   for (int i = 0; i < size; i++)
;     data[i] &= 0xF;
; }

; CHECK-LABEL: @bar
; CHECK-LABEL: entry:
; CHECK-NEXT: [[GV_LOAD:%.*]] = load i1, ptr @status
; CHECK-NEXT: br i1 [[GV_LOAD]], label %codeRepl.i,
; CHECK-LABEL: codeRepl.i:
; CHECK-NEXT: call void (i8, ptr, ...) @foo.1.if.end(i8 9, ptr %buf, i32 noundef 1)

; NO-PARTIAL-INLINE-LABEL: @bar
; NO-PARTIAL-INLINE-LABEL: entry:
; NO-PARTIAL-INLINE: call i32 (i8, ptr, ...) @foo(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.__va_list_tag = type { i32, i32, ptr, ptr }

@status = internal unnamed_addr global i1 false, align 4

define dso_local i32 @foo(i8 noundef zeroext %opcode, ptr nocapture noundef writeonly %buf, ...) {
entry:
  %args = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %args)
  %.b = load i1, ptr @status, align 4
  br i1 %.b, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %arraydecay = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0
  call void @llvm.va_start(ptr nonnull %arraydecay)
  store i8 %opcode, ptr %buf, align 1
  switch i8 %opcode, label %sw.default [
    i8 1, label %sw.bb
    i8 2, label %sw.bb3
    i8 3, label %sw.bb3
    i8 4, label %sw.bb30
    i8 5, label %sw.bb70
    i8 6, label %sw.bb70
    i8 7, label %sw.bb70
    i8 8, label %sw.bb70
    i8 9, label %sw.bb70
    i8 10, label %sw.bb70
    i8 11, label %sw.bb70
    i8 12, label %sw.bb84
    i8 13, label %sw.bb84
    i8 14, label %sw.bb84
    i8 15, label %sw.bb84
    i8 16, label %sw.bb84
  ]

sw.bb:                                            ; preds = %if.end
  %gp_offset = load i32, ptr %arraydecay, align 16
  %fits_in_gp = icmp ult i32 %gp_offset, 41
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %sw.bb
  %0 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area = load ptr, ptr %0, align 16
  %1 = zext i32 %gp_offset to i64
  %2 = getelementptr i8, ptr %reg_save_area, i64 %1
  %3 = add nuw nsw i32 %gp_offset, 8
  store i32 %3, ptr %arraydecay, align 16
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %sw.bb
  %overflow_arg_area_p = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area = load ptr, ptr %overflow_arg_area_p, align 8
  %overflow_arg_area.next = getelementptr i8, ptr %overflow_arg_area, i64 8
  store ptr %overflow_arg_area.next, ptr %overflow_arg_area_p, align 8
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr = phi ptr [ %2, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %4 = load i64, ptr %vaarg.addr, align 8
  %arrayidx2 = getelementptr inbounds i8, ptr %buf, i64 1
  store i64 %4, ptr %arrayidx2, align 8
  br label %cleanup

sw.bb3:                                           ; preds = %if.end, %if.end
  %gp_offset6 = load i32, ptr %arraydecay, align 16
  %fits_in_gp7 = icmp ult i32 %gp_offset6, 41
  br i1 %fits_in_gp7, label %vaarg.in_reg8, label %vaarg.in_mem10

vaarg.in_reg8:                                    ; preds = %sw.bb3
  %5 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area9 = load ptr, ptr %5, align 16
  %6 = zext i32 %gp_offset6 to i64
  %7 = getelementptr i8, ptr %reg_save_area9, i64 %6
  %8 = add nuw nsw i32 %gp_offset6, 8
  store i32 %8, ptr %arraydecay, align 16
  br label %vaarg.end14

vaarg.in_mem10:                                   ; preds = %sw.bb3
  %overflow_arg_area_p11 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area12 = load ptr, ptr %overflow_arg_area_p11, align 8
  %overflow_arg_area.next13 = getelementptr i8, ptr %overflow_arg_area12, i64 8
  store ptr %overflow_arg_area.next13, ptr %overflow_arg_area_p11, align 8
  br label %vaarg.end14

vaarg.end14:                                      ; preds = %vaarg.in_mem10, %vaarg.in_reg8
  %gp_offset19 = phi i32 [ %gp_offset6, %vaarg.in_mem10 ], [ %8, %vaarg.in_reg8 ]
  %vaarg.addr15 = phi ptr [ %overflow_arg_area12, %vaarg.in_mem10 ], [ %7, %vaarg.in_reg8 ]
  %9 = load i32, ptr %vaarg.addr15, align 4
  %arrayidx16 = getelementptr inbounds i8, ptr %buf, i64 1
  store i32 %9, ptr %arrayidx16, align 4
  %fits_in_gp20 = icmp ult i32 %gp_offset19, 41
  br i1 %fits_in_gp20, label %vaarg.in_reg21, label %vaarg.in_mem23

vaarg.in_reg21:                                   ; preds = %vaarg.end14
  %10 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area22 = load ptr, ptr %10, align 16
  %11 = zext i32 %gp_offset19 to i64
  %12 = getelementptr i8, ptr %reg_save_area22, i64 %11
  %13 = add nuw nsw i32 %gp_offset19, 8
  store i32 %13, ptr %arraydecay, align 16
  br label %vaarg.end27

vaarg.in_mem23:                                   ; preds = %vaarg.end14
  %overflow_arg_area_p24 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area25 = load ptr, ptr %overflow_arg_area_p24, align 8
  %overflow_arg_area.next26 = getelementptr i8, ptr %overflow_arg_area25, i64 8
  store ptr %overflow_arg_area.next26, ptr %overflow_arg_area_p24, align 8
  br label %vaarg.end27

vaarg.end27:                                      ; preds = %vaarg.in_mem23, %vaarg.in_reg21
  %vaarg.addr28 = phi ptr [ %12, %vaarg.in_reg21 ], [ %overflow_arg_area25, %vaarg.in_mem23 ]
  %14 = load i32, ptr %vaarg.addr28, align 4
  %arrayidx29 = getelementptr inbounds i8, ptr %buf, i64 5
  store i32 %14, ptr %arrayidx29, align 4
  br label %cleanup

sw.bb30:                                          ; preds = %if.end
  %gp_offset33 = load i32, ptr %arraydecay, align 16
  %fits_in_gp34 = icmp ult i32 %gp_offset33, 41
  br i1 %fits_in_gp34, label %vaarg.in_reg35, label %vaarg.in_mem37

vaarg.in_reg35:                                   ; preds = %sw.bb30
  %15 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area36 = load ptr, ptr %15, align 16
  %16 = zext i32 %gp_offset33 to i64
  %17 = getelementptr i8, ptr %reg_save_area36, i64 %16
  %18 = add nuw nsw i32 %gp_offset33, 8
  store i32 %18, ptr %arraydecay, align 16
  br label %vaarg.end41

vaarg.in_mem37:                                   ; preds = %sw.bb30
  %overflow_arg_area_p38 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area39 = load ptr, ptr %overflow_arg_area_p38, align 8
  %overflow_arg_area.next40 = getelementptr i8, ptr %overflow_arg_area39, i64 8
  store ptr %overflow_arg_area.next40, ptr %overflow_arg_area_p38, align 8
  br label %vaarg.end41

vaarg.end41:                                      ; preds = %vaarg.in_mem37, %vaarg.in_reg35
  %gp_offset59.pr = phi i32 [ %gp_offset33, %vaarg.in_mem37 ], [ %18, %vaarg.in_reg35 ]
  %vaarg.addr42 = phi ptr [ %overflow_arg_area39, %vaarg.in_mem37 ], [ %17, %vaarg.in_reg35 ]
  %19 = load i32, ptr %vaarg.addr42, align 4
  %arrayidx43 = getelementptr inbounds i8, ptr %buf, i64 1
  store i32 %19, ptr %arrayidx43, align 4
  %fits_in_gp47 = icmp ult i32 %gp_offset59.pr, 41
  br i1 %fits_in_gp47, label %vaarg.in_reg48, label %vaarg.in_mem50

vaarg.in_reg48:                                   ; preds = %vaarg.end41
  %20 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area49 = load ptr, ptr %20, align 16
  %21 = zext i32 %gp_offset59.pr to i64
  %22 = getelementptr i8, ptr %reg_save_area49, i64 %21
  %23 = add nuw nsw i32 %gp_offset59.pr, 8
  store i32 %23, ptr %arraydecay, align 16
  br label %vaarg.end54

vaarg.in_mem50:                                   ; preds = %vaarg.end41
  %overflow_arg_area_p51 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area52 = load ptr, ptr %overflow_arg_area_p51, align 8
  %overflow_arg_area.next53 = getelementptr i8, ptr %overflow_arg_area52, i64 8
  store ptr %overflow_arg_area.next53, ptr %overflow_arg_area_p51, align 8
  br label %vaarg.end54

vaarg.end54:                                      ; preds = %vaarg.in_mem50, %vaarg.in_reg48
  %gp_offset59 = phi i32 [ %gp_offset59.pr, %vaarg.in_mem50 ], [ %23, %vaarg.in_reg48 ]
  %vaarg.addr55 = phi ptr [ %overflow_arg_area52, %vaarg.in_mem50 ], [ %22, %vaarg.in_reg48 ]
  %24 = load i32, ptr %vaarg.addr55, align 4
  %arrayidx56 = getelementptr inbounds i8, ptr %buf, i64 5
  store i32 %24, ptr %arrayidx56, align 4
  %fits_in_gp60 = icmp ult i32 %gp_offset59, 41
  br i1 %fits_in_gp60, label %vaarg.in_reg61, label %vaarg.in_mem63

vaarg.in_reg61:                                   ; preds = %vaarg.end54
  %25 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area62 = load ptr, ptr %25, align 16
  %26 = zext i32 %gp_offset59 to i64
  %27 = getelementptr i8, ptr %reg_save_area62, i64 %26
  %28 = add nuw nsw i32 %gp_offset59, 8
  store i32 %28, ptr %arraydecay, align 16
  br label %vaarg.end67

vaarg.in_mem63:                                   ; preds = %vaarg.end54
  %overflow_arg_area_p64 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area65 = load ptr, ptr %overflow_arg_area_p64, align 8
  %overflow_arg_area.next66 = getelementptr i8, ptr %overflow_arg_area65, i64 8
  store ptr %overflow_arg_area.next66, ptr %overflow_arg_area_p64, align 8
  br label %vaarg.end67

vaarg.end67:                                      ; preds = %vaarg.in_mem63, %vaarg.in_reg61
  %vaarg.addr68 = phi ptr [ %27, %vaarg.in_reg61 ], [ %overflow_arg_area65, %vaarg.in_mem63 ]
  %29 = load i32, ptr %vaarg.addr68, align 4
  %arrayidx69 = getelementptr inbounds i8, ptr %buf, i64 9
  store i32 %29, ptr %arrayidx69, align 4
  br label %cleanup

sw.bb70:                                          ; preds = %if.end, %if.end, %if.end, %if.end, %if.end, %if.end, %if.end
  %gp_offset73 = load i32, ptr %arraydecay, align 16
  %fits_in_gp74 = icmp ult i32 %gp_offset73, 41
  br i1 %fits_in_gp74, label %vaarg.in_reg75, label %vaarg.in_mem77

vaarg.in_reg75:                                   ; preds = %sw.bb70
  %30 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area76 = load ptr, ptr %30, align 16
  %31 = zext i32 %gp_offset73 to i64
  %32 = getelementptr i8, ptr %reg_save_area76, i64 %31
  %33 = add nuw nsw i32 %gp_offset73, 8
  store i32 %33, ptr %arraydecay, align 16
  br label %vaarg.end81

vaarg.in_mem77:                                   ; preds = %sw.bb70
  %overflow_arg_area_p78 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area79 = load ptr, ptr %overflow_arg_area_p78, align 8
  %overflow_arg_area.next80 = getelementptr i8, ptr %overflow_arg_area79, i64 8
  store ptr %overflow_arg_area.next80, ptr %overflow_arg_area_p78, align 8
  br label %vaarg.end81

vaarg.end81:                                      ; preds = %vaarg.in_mem77, %vaarg.in_reg75
  %vaarg.addr82 = phi ptr [ %32, %vaarg.in_reg75 ], [ %overflow_arg_area79, %vaarg.in_mem77 ]
  %34 = load i32, ptr %vaarg.addr82, align 4
  %arrayidx83 = getelementptr inbounds i8, ptr %buf, i64 1
  store i32 %34, ptr %arrayidx83, align 4
  br label %cleanup

sw.bb84:                                          ; preds = %if.end, %if.end, %if.end, %if.end, %if.end
  %gp_offset87 = load i32, ptr %arraydecay, align 16
  %fits_in_gp88 = icmp ult i32 %gp_offset87, 41
  br i1 %fits_in_gp88, label %vaarg.in_reg89, label %vaarg.in_mem91

vaarg.in_reg89:                                   ; preds = %sw.bb84
  %35 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area90 = load ptr, ptr %35, align 16
  %36 = zext i32 %gp_offset87 to i64
  %37 = getelementptr i8, ptr %reg_save_area90, i64 %36
  %38 = add nuw nsw i32 %gp_offset87, 8
  store i32 %38, ptr %arraydecay, align 16
  br label %vaarg.end95

vaarg.in_mem91:                                   ; preds = %sw.bb84
  %overflow_arg_area_p92 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area93 = load ptr, ptr %overflow_arg_area_p92, align 8
  %overflow_arg_area.next94 = getelementptr i8, ptr %overflow_arg_area93, i64 8
  store ptr %overflow_arg_area.next94, ptr %overflow_arg_area_p92, align 8
  br label %vaarg.end95

vaarg.end95:                                      ; preds = %vaarg.in_mem91, %vaarg.in_reg89
  %vaarg.addr96 = phi ptr [ %37, %vaarg.in_reg89 ], [ %overflow_arg_area93, %vaarg.in_mem91 ]
  %39 = load i32, ptr %vaarg.addr96, align 4
  %fp_offset_p = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 1
  %fp_offset = load i32, ptr %fp_offset_p, align 4
  %fits_in_fp = icmp ult i32 %fp_offset, 161
  br i1 %fits_in_fp, label %vaarg.in_reg98, label %vaarg.in_mem100

vaarg.in_reg98:                                   ; preds = %vaarg.end95
  %40 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area99 = load ptr, ptr %40, align 16
  %41 = zext i32 %fp_offset to i64
  %42 = getelementptr i8, ptr %reg_save_area99, i64 %41
  %43 = add nuw nsw i32 %fp_offset, 16
  store i32 %43, ptr %fp_offset_p, align 4
  br label %vaarg.end104

vaarg.in_mem100:                                  ; preds = %vaarg.end95
  %overflow_arg_area_p101 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area102 = load ptr, ptr %overflow_arg_area_p101, align 8
  %overflow_arg_area.next103 = getelementptr i8, ptr %overflow_arg_area102, i64 8
  store ptr %overflow_arg_area.next103, ptr %overflow_arg_area_p101, align 8
  br label %vaarg.end104

vaarg.end104:                                     ; preds = %vaarg.in_mem100, %vaarg.in_reg98
  %vaarg.addr105 = phi ptr [ %42, %vaarg.in_reg98 ], [ %overflow_arg_area102, %vaarg.in_mem100 ]
  %44 = load double, ptr %vaarg.addr105, align 8
  %arrayidx106 = getelementptr inbounds i8, ptr %buf, i64 1
  store i32 %39, ptr %arrayidx106, align 4
  %arrayidx107 = getelementptr inbounds i8, ptr %buf, i64 5
  store double %44, ptr %arrayidx107, align 8
  br label %cleanup

sw.default:                                       ; preds = %if.end
  call void @exit(i32 noundef 1)
  unreachable

cleanup:                                          ; preds = %vaarg.end, %vaarg.end27, %vaarg.end67, %vaarg.end81, %vaarg.end104, %entry
  %retval.0 = phi i32 [ 0, %entry ], [ 1, %vaarg.end104 ], [ 1, %vaarg.end81 ], [ 1, %vaarg.end67 ], [ 1, %vaarg.end27 ], [ 1, %vaarg.end ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %args)
  ret i32 %retval.0
}

define void @bar(ptr nocapture noundef %data, i32 noundef %size, ptr nocapture noundef writeonly %buf) {
entry:
  %call = tail call i32 (i8, ptr, ...) @foo(i8 noundef zeroext 9, ptr noundef %buf, i32 noundef 1) #0
  %tobool.not = icmp eq i32 %call, 0
  %cmp3 = icmp sgt i32 %size, 0
  %or.cond = and i1 %tobool.not, %cmp3
  br i1 %or.cond, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %data, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %and = and i32 %0, 15
  store i32 %and, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare void @llvm.va_start(ptr)

declare void @exit(i32 noundef)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

attributes #0 = { "lccc-call-in-leaf" }
; end INTEL_FEATURE_SW_ADVANCED
