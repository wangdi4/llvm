; REQUIRES: asserts
; RUN: opt < %s -passes='print<hir-region-identification>' -debug-only=hir-region-identification 2>&1 | FileCheck %s

; Verify that we suppress call instruction containing operand bundle ["funclet"(token %4)] as there is no support for them in the framework.

; CHECK: LOOPOPT_OPTREPORT: Loop %arraydestroy.body.i: Unsupported operand bundle.

; This is a windows exception handling test case. Here's the source code-

; struct object {
;  static int statcount;
;  static int statcount1;
;  static int threshold;
;  int count;
;  object () {
;    if (statcount >= threshold) {
;      threshold += 2;
;      throw statcount;
;    }
;    count = ++statcount;
;    printf("Making: %d\n", count);
;  }
;  object (const object &obj) {
;    count = ++statcount;
;    printf("Copying %d to %d\n", obj.count, count);
;  }
;  ~object () {
;    printf("Killed %d\n", count);
;  }
; };
; int object::statcount = 0;
; int object::statcount1 = 100;
; int object::threshold = 2;
;
; struct derived {
;  object obj[3];
;
;  derived () {}
; };
;
; int main () {
;
;  try {
;    derived local;
;  }
;  catch (...) {
;    printf("Caught explicit\n");
;  }
;
;  return 0;
; }

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%eh.ThrowInfo = type { i32, i32, i32, i32 }
%struct.derived = type { [3 x %struct.object] }
%struct.object = type { i32 }

@"\01?statcount@object@@2HA" = external local_unnamed_addr global i32, align 4
@"\01?threshold@object@@2HA" = external local_unnamed_addr global i32, align 4
@_TI1H = external unnamed_addr constant %eh.ThrowInfo, section ".xdata"
@"\01??_C@_0M@KPPGBBFJ@Making?3?5?$CFd?6?$AA@" = external unnamed_addr constant [12 x i8], align 1
@"\01??_C@_0L@NIAHONDG@Killed?5?$CFd?6?$AA@" = external unnamed_addr constant [11 x i8], align 1
@str = external hidden unnamed_addr constant [16 x i8]

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #0 personality ptr @__CxxFrameHandler3 {
entry:
  %tmp.i.i = alloca i32, align 4
  %local = alloca %struct.derived, align 4
  call void @llvm.lifetime.start.p0(i64 12, ptr nonnull %local) #3
  br label %arrayctor.loop.i

arrayctor.loop.i:                                 ; preds = %invoke.cont.i, %entry
  %arrayctor.cur.idx.i = phi i64 [ 0, %entry ], [ %arrayctor.cur.add.i, %invoke.cont.i ]
  %arrayctor.cur.ptr.i = getelementptr inbounds %struct.derived, ptr %local, i64 0, i32 0, i64 %arrayctor.cur.idx.i
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %tmp.i.i)
  %0 = load i32, ptr @"\01?statcount@object@@2HA", align 4, !tbaa !11
  %1 = load i32, ptr @"\01?threshold@object@@2HA", align 4, !tbaa !11
  %cmp.i.i = icmp slt i32 %0, %1
  br i1 %cmp.i.i, label %invoke.cont.i, label %if.then.i.i

if.then.i.i:                                      ; preds = %arrayctor.loop.i
  %arrayctor.cur.idx.i.lcssa = phi i64 [ %arrayctor.cur.idx.i, %arrayctor.loop.i ]
  %arrayctor.cur.ptr.i.lcssa = phi ptr [ %arrayctor.cur.ptr.i, %arrayctor.loop.i ]
  %.lcssa26 = phi i32 [ %0, %arrayctor.loop.i ]
  %.lcssa = phi i32 [ %1, %arrayctor.loop.i ]
  %add.i.i = add nsw i32 %.lcssa, 2
  store i32 %add.i.i, ptr @"\01?threshold@object@@2HA", align 4, !tbaa !11
  store i32 %.lcssa26, ptr %tmp.i.i, align 4, !tbaa !11
  invoke void @_CxxThrowException(ptr nonnull %tmp.i.i, ptr nonnull @_TI1H) #4
          to label %.noexc.i unwind label %ehcleanup.i

.noexc.i:                                         ; preds = %if.then.i.i
  unreachable

invoke.cont.i:                                    ; preds = %arrayctor.loop.i
  %inc.i.i = add nsw i32 %0, 1
  store i32 %inc.i.i, ptr @"\01?statcount@object@@2HA", align 4, !tbaa !11
  store i32 %inc.i.i, ptr %arrayctor.cur.ptr.i, align 4, !tbaa !15
  %call.i.i = tail call i32 (ptr, ...) @printf(ptr @"\01??_C@_0M@KPPGBBFJ@Making?3?5?$CFd?6?$AA@", i32 %inc.i.i) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %tmp.i.i)
  %arrayctor.cur.add.i = add nuw nsw i64 %arrayctor.cur.idx.i, 1
  %arrayctor.done.i = icmp eq i64 %arrayctor.cur.add.i, 3
  br i1 %arrayctor.done.i, label %invoke.cont, label %arrayctor.loop.i

ehcleanup.i:                                      ; preds = %if.then.i.i
  %2 = cleanuppad within none []
  %arraydestroy.isempty.i = icmp eq i64 %arrayctor.cur.idx.i.lcssa, 0
  br i1 %arraydestroy.isempty.i, label %arraydestroy.done2.i, label %arraydestroy.body.preheader.i

arraydestroy.body.preheader.i:                    ; preds = %ehcleanup.i
  br label %arraydestroy.body.i

arraydestroy.body.i:                              ; preds = %arraydestroy.body.i, %arraydestroy.body.preheader.i
  %arraydestroy.elementPast.i = phi ptr [ %arraydestroy.element.i, %arraydestroy.body.i ], [ %arrayctor.cur.ptr.i.lcssa, %arraydestroy.body.preheader.i ]
  %arraydestroy.element.i = getelementptr inbounds %struct.object, ptr %arraydestroy.elementPast.i, i64 -1
  %3 = load i32, ptr %arraydestroy.element.i, align 4, !tbaa !15
  %call.i4.i = call i32 (ptr, ...) @printf(ptr @"\01??_C@_0L@NIAHONDG@Killed?5?$CFd?6?$AA@", i32 %3) #3 [ "funclet"(token %2) ]
  %arraydestroy.done.i = icmp eq ptr %arraydestroy.element.i, %local
  br i1 %arraydestroy.done.i, label %arraydestroy.done2.i.loopexit, label %arraydestroy.body.i

arraydestroy.done2.i.loopexit:                    ; preds = %arraydestroy.body.i
  br label %arraydestroy.done2.i

arraydestroy.done2.i:                             ; preds = %arraydestroy.done2.i.loopexit, %ehcleanup.i
  cleanupret from %2 unwind label %catch.dispatch

invoke.cont:                                      ; preds = %invoke.cont.i
  %4 = getelementptr inbounds %struct.derived, ptr %local, i64 0, i32 0, i64 3
  br label %arraydestroy.body.i8

arraydestroy.body.i8:                             ; preds = %arraydestroy.body.i8, %invoke.cont
  %arraydestroy.elementPast.i3 = phi ptr [ %4, %invoke.cont ], [ %arraydestroy.element.i4, %arraydestroy.body.i8 ]
  %arraydestroy.element.i4 = getelementptr inbounds %struct.object, ptr %arraydestroy.elementPast.i3, i64 -1
  %5 = load i32, ptr %arraydestroy.element.i4, align 4, !tbaa !15
  %call.i.i6 = call i32 (ptr, ...) @printf(ptr @"\01??_C@_0L@NIAHONDG@Killed?5?$CFd?6?$AA@", i32 %5) #3
  %arraydestroy.done.i7 = icmp eq ptr %arraydestroy.element.i4, %local
  br i1 %arraydestroy.done.i7, label %"\01??1derived@@QEAA@XZ.exit", label %arraydestroy.body.i8

"\01??1derived@@QEAA@XZ.exit":                    ; preds = %arraydestroy.body.i8
  call void @llvm.lifetime.end.p0(i64 12, ptr nonnull %local) #3
  br label %try.cont

catch.dispatch:                                   ; preds = %arraydestroy.done2.i
  %6 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %7 = catchpad within %6 [ptr null, i32 64, ptr null]
  %puts = call i32 @puts(ptr @str) [ "funclet"(token %7) ]
  catchret from %7 to label %try.cont

try.cont:                                         ; preds = %catch, %"\01??1derived@@QEAA@XZ.exit"
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

declare i32 @__CxxFrameHandler3(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

; Function Attrs: inlinehint nounwind uwtable
declare i32 @printf(ptr, ...) local_unnamed_addr #2

declare void @_CxxThrowException(ptr, ptr) local_unnamed_addr

; Function Attrs: nounwind
declare i32 @puts(ptr nocapture readonly) #3

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { noreturn }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !6, !7}
!llvm.module.flags = !{!8, !9}
!llvm.ident = !{!10}

!0 = !{!"/DEFAULTLIB:libircmt.lib"}
!1 = !{!"/DEFAULTLIB:libmmt.lib"}
!2 = !{!"/DEFAULTLIB:libcmt.lib"}
!3 = !{!"/DEFAULTLIB:oldnames.lib"}
!4 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!5 = !{!"/DEFAULTLIB:libdecimal.lib"}
!6 = !{!"/DEFAULTLIB:cilkrts.lib"}
!7 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!8 = !{i32 1, !"wchar_size", i32 2}
!9 = !{i32 7, !"PIC Level", i32 2}
!10 = !{!"clang version 5.0.0 (cfe/trunk)"}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = !{!16, !12, i64 0}
!16 = !{!"struct@?AUobject@@", !12, i64 0}
