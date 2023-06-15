; RUN: opt -S -passes=jump-threading %s | FileCheck %s

; CHECK: %vtable.i.i8 = load ptr, ptr %agg.tmp.ensured.sroa.1.0, align 8

; The "processBranchOnOr" optimization in JT, will try to make this
; transformation:
;  %agg.tmp.ensured.sroa.1.0 = phi ptr [ %call.i.i.i6.i, %normal_succ ], [ null, %entry ]
; ...
;  "if %agg.tmp.ensured.sroa.1.0 != null"
;     %vtable.i.i8 = load ptr, ptr %agg.tmp.ensured.sroa.1.0, align 8
; =>
; %agg.tmp.... phi ...
; ...
;  "if %agg.tmp.ensured.sroa.1.0 != null"
;     %vtable.i.i8 = load ptr, ptr %call.i.i.i6.i, align 8
;
; The phi value is either %call.i.i.i6.i, or null.
; Because the "vtable" load is guarded by a check for %agg.tmp != null, JT
; assumes that %agg.tmp must be %call.i.i.i6.i.
; But in the case below, %call.i.i.i6.i is the result of an invoke.
; This result is only valid on the normal edge, not the unwind edge.
; The phi is required for correct SSA, as %call.i.i.i6.i cannot dominate
; its unwind successor block. It is only valid on the edge from "normal_succ".

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

@"__profc_?a@@YAXXZ" = external hidden global [6 x i64], section ".lprfc$M", align 8
@"__profd_?a@@YAXXZ" = external hidden global { i64, i64, i64, ptr, ptr, i32, [2 x i16] }, section ".lprfd$M", align 8

; Function Attrs: uwtable
define dso_local void @"?a@@YAXXZ"() local_unnamed_addr #0 personality ptr null {
entry:
  %call.i.i.i6.i = invoke noundef ptr null(ptr noundef null, i32 noundef undef)
          to label %normal_succ unwind label %ehcleanup.i

normal_succ:                   ; preds = %entry
  %call3.i = invoke noundef ptr null(ptr noundef nonnull align 1 dereferenceable(1) null)
          to label %"?ar@?$ao@$$A6A?AW4aw@@XZH@@SA?AV1@XZ.exit" unwind label %ehcleanup.i

ehcleanup.i:                                      ; preds = %normal_succ, %entry
  %agg.tmp.ensured.sroa.1.0 = phi ptr [ %call.i.i.i6.i, %normal_succ ], [ null, %entry ]
  %tobool.not.i.i.i4 = phi i1 [ false, %normal_succ ], [ true, %entry ]
  %agg.tmp.ensured.sroa.7.0 = phi i32 [ 8, %normal_succ ], [ 0, %entry ]
  %0 = cleanuppad within none []
  %tobool.not4.i.i6 = icmp eq ptr %agg.tmp.ensured.sroa.1.0, null
  %tobool.not.i.i7 = or i1 %tobool.not.i.i.i4, %tobool.not4.i.i6
  br i1 %tobool.not.i.i7, label %invoke.cont.i14, label %if.then.i.i10

if.then.i.i10:                                    ; preds = %ehcleanup.i
  %pgocount = load i64, ptr getelementptr inbounds ([6 x i64], ptr @"__profc_?a@@YAXXZ", i32 0, i32 4), align 8
  %1 = add i64 %pgocount, 1
  store i64 %1, ptr getelementptr inbounds ([6 x i64], ptr @"__profc_?a@@YAXXZ", i32 0, i32 4), align 8
  %vtable.i.i8 = load ptr, ptr %agg.tmp.ensured.sroa.1.0, align 8
  %2 = load ptr, ptr %vtable.i.i8, align 8
  %3 = ptrtoint ptr %2 to i64
  call void null(i64 %3, ptr @"__profd_?a@@YAXXZ", i32 0) [ "funclet"(token %0) ]
  %call2.i.i9 = call noundef ptr %2(ptr noundef nonnull align 8 dereferenceable(8) %agg.tmp.ensured.sroa.1.0, i32 noundef 0) #1 [ "funclet"(token %0) ]
  br label %invoke.cont.i14

invoke.cont.i14:                                  ; preds = %if.then.i.i10, %ehcleanup.i
  %agg.tmp.ensured.sroa.7.1 = phi i32 [ %agg.tmp.ensured.sroa.7.0, %ehcleanup.i ], [ 0, %if.then.i.i10 ]
  %tobool.not.i.i3.i11 = icmp eq i32 %agg.tmp.ensured.sroa.7.1, 0
  %tobool.not.i6.i13 = or i1 %tobool.not.i.i3.i11, %tobool.not4.i.i6
  br label %"??1?$ao@$$A6A?AW4aw@@XZH@@QEAA@XZ.exit20"

"??1?$ao@$$A6A?AW4aw@@XZH@@QEAA@XZ.exit20":       ; preds = %invoke.cont.i14
  %pgocount28 = load i64, ptr getelementptr inbounds ([6 x i64], ptr @"__profc_?a@@YAXXZ", i32 0, i32 3), align 8
  %4 = add i64 %pgocount28, 1
  store i64 %4, ptr getelementptr inbounds ([6 x i64], ptr @"__profc_?a@@YAXXZ", i32 0, i32 3), align 8
  cleanupret from %0 unwind to caller

"?ar@?$ao@$$A6A?AW4aw@@XZH@@SA?AV1@XZ.exit":      ; preds = %normal_succ
  %pgocount29 = load i64, ptr @"__profc_?a@@YAXXZ", align 8
  %5 = add i64 %pgocount29, 1
  store i64 %5, ptr @"__profc_?a@@YAXXZ", align 8
  %tobool.not4.i.i = icmp eq ptr %call.i.i.i6.i, null
  br label %"??1?$ao@$$A6A?AW4aw@@XZH@@QEAA@XZ.exit"

"??1?$ao@$$A6A?AW4aw@@XZH@@QEAA@XZ.exit":         ; preds = %"?ar@?$ao@$$A6A?AW4aw@@XZH@@SA?AV1@XZ.exit"
  ret void
}

attributes #0 = { uwtable }
attributes #1 = { nounwind }

