; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that decomposer generates VPInstructions with the proper convertions
; and types for operations on integer and pointer types.

; Source code
; void make_simple_name(name)
;     char *name;
; {
;     char *p = strrchr(name, '.');
;     if (p == (char *)0) return;
;     if (p == name) p++;
;     do {
;         if (*--p == '.') *p = '_';
;     } while (p != name);
; }

; CHECK: i64 [[NAME:%vp.*]] = ptrtoint i8* %name to i64
; CHECK: i64 [[MUL:%vp.*]] = mul i64 [[NAME]] i64 -1
; CHECK: i64 [[SPEC:%vp.*]] = ptrtoint i8* %spec.select to i64
; CHECK: i64 [[ADD:%vp.*]] = add i64 [[MUL]] i64 [[SPEC]]
; CHECK: i64 %vp{{[0-9]+}} = add i64 [[ADD]] i64 -1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @make_simple_name(i8* %name) local_unnamed_addr #0 {
entry:
  %call = tail call i8* @strrchr(i8* %name, i32 46)
  %cmp = icmp eq i8* %call, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %cmp1 = icmp eq i8* %call, %name
  %incdec.ptr = getelementptr inbounds i8, i8* %call, i64 1
  %spec.select = select i1 %cmp1, i8* %incdec.ptr, i8* %call
  br label %do.body

do.body:                                          ; preds = %do.cond, %if.end
  %p.1 = phi i8* [ %spec.select, %if.end ], [ %incdec.ptr4, %do.cond ]
  %incdec.ptr4 = getelementptr inbounds i8, i8* %p.1, i64 -1
  %0 = load i8, i8* %incdec.ptr4, align 1, !tbaa !2
  %cmp5 = icmp eq i8 %0, 46
  br i1 %cmp5, label %if.then7, label %do.cond

if.then7:                                         ; preds = %do.body
  store i8 95, i8* %incdec.ptr4, align 1, !tbaa !2
  br label %do.cond

do.cond:                                          ; preds = %do.body, %if.then7
  %cmp9 = icmp eq i8* %incdec.ptr4, %name
  br i1 %cmp9, label %cleanup.loopexit, label %do.body

cleanup.loopexit:                                 ; preds = %do.cond
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %entry
  ret void
}

; Function Attrs: nounwind readonly
declare dso_local i8* @strrchr(i8*, i32) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
