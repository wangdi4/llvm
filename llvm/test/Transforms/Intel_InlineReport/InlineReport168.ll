; RUN: opt -passes='function(jump-threading),inline' -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,function(jump-threading),inline,inlinereportemitter' -inline-report=0xf886 -disable-output < %s 2>&1 | FileCheck %s

; Show that after jump threading, the call to note_dep is duplicated and
; appears twice in both the classic and metadata inline reports.

; CHECK-LABEL: DEAD STATIC FUNC: note_dep
; CHECK-LABEL: COMPILE FUNC: add_dependence
; CHECK: EXTERN: fancy_abort
; CHECK: EXTERN: fancy_abort
; CHECK: INLINE: note_dep {{.*}}Inlining is profitable
; CHECK: INDIRECT: {{.*}}Call site is indirect]]
; CHECK: INLINE: note_dep {{.*}}Callee has single callsite and local linkage
; CHECK: INDIRECT: {{.*}}Call site is indirect

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
%struct._ZTS19sched_deps_info_def.sched_deps_info_def = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i8 }
@sched_deps_info = dso_local local_unnamed_addr global ptr null, align 8
@.str = private unnamed_addr constant [13 x i8] c"sched-deps.c\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"?\00", align 1
@cur_insn = internal unnamed_addr global ptr null, align 8

declare dso_local void @fancy_abort(ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr

; Function Attrs: nounwind uwtable
define internal fastcc void @note_dep(ptr noundef %e, i32 noundef %ds) unnamed_addr {
entry:
  %0 = load ptr, ptr @sched_deps_info, align 8
  %note_dep = getelementptr inbounds %struct._ZTS19sched_deps_info_def.sched_deps_info_def, ptr %0, i64 0, i32 11
  %1 = load ptr, ptr %note_dep, align 8
  %tobool.not = icmp eq ptr %1, null
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void %1(ptr noundef %e, i32 noundef %ds) #2
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @add_dependence(ptr noundef %insn, ptr noundef %elem, i32 noundef %dep_type) local_unnamed_addr {
entry:
  switch i32 %dep_type, label %cond.true [
    i32 0, label %if.end5
    i32 11, label %if.end5.fold.split
    i32 12, label %if.end5.fold.split24
  ]

cond.true:                                        ; preds = %entry
  call void @fancy_abort(ptr noundef nonnull @.str, i32 noundef 3808, ptr noundef nonnull @.str.1)
  br label %if.end5

if.end5.fold.split:                               ; preds = %entry
  br label %if.end5

if.end5.fold.split24:                             ; preds = %entry
  br label %if.end5

if.end5:                                          ; preds = %if.end5.fold.split24, %if.end5.fold.split, %cond.true, %entry
  %ds.0 = phi i32 [ 16777216, %entry ], [ 67108864, %cond.true ], [ 33554432, %if.end5.fold.split ], [ 67108864, %if.end5.fold.split24 ]
  %0 = load ptr, ptr @cur_insn, align 8
  %cmp6.not = icmp eq ptr %0, null
  br i1 %cmp6.not, label %if.then18, label %if.then8

if.then8:                                         ; preds = %if.end5
  %cmp9 = icmp eq ptr %0, %insn
  br i1 %cmp9, label %if.end16.thread, label %cond.true11

cond.true11:                                      ; preds = %if.then8
  call void @fancy_abort(ptr noundef nonnull @.str, i32 noundef 3816, ptr noundef nonnull @.str.1)
  br label %if.end16.thread

if.end16.thread:                                  ; preds = %cond.true11, %if.then8
  call fastcc void @note_dep(ptr noundef %elem, i32 noundef %ds.0)
  br label %if.end19

if.then18:                                        ; preds = %if.end5
  store ptr %insn, ptr @cur_insn, align 8
  call fastcc void @note_dep(ptr noundef %elem, i32 noundef %ds.0)
  store ptr null, ptr @cur_insn, align 8
  br label %if.end19

if.end19:                                         ; preds = %if.then18, %if.end16.thread
  ret void
}

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}
!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
