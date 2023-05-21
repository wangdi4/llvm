; RUN: opt -passes=verify -S < %s | FileCheck %s

; invoke void @llvm.seh.scope.begin()
; invoke void @llvm.seh.scope.end()
; CHECK-NOT: invoke void @llvm.seh.scope.begin() {{.*}}funclet
; CHECK-NOT: invoke void @llvm.seh.scope.end() {{.*}}funclet
; CHECK-NOT: input module is broken!

@"??_7type_info@@6B@" = external constant ptr
%rtti.TypeDescriptor7 = type { ptr, ptr, [8 x i8] }
%class.c = type { %class.l.base, %class.H, [7 x i8], %class.f }
%class.l.base = type { %class.j.base, ptr }
%class.j.base = type { ptr }
%class.H = type { i8 }
%class.f = type { i32 }
%class.l = type { %class.j.base, ptr, %class.f }

declare dso_local void @llvm.seh.scope.begin()

declare dso_local void @llvm.seh.scope.end() #2

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__CxxFrameHandler3(...)

declare dso_local noundef ptr @"??0H@@QEAA@H@Z"(ptr noundef nonnull returned align 1 dereferenceable(1), i32 noundef) unnamed_addr #5

declare dso_local noundef ptr @"??0i@@QEAA@W4a@@_N@Z"(ptr noundef nonnull returned align 1 dereferenceable(1), i32 noundef, i1 noundef zeroext) unnamed_addr #5

declare dso_local noundef ptr @"??0j@@QEAA@PEAVh@@_N@Z"(ptr noundef nonnull returned align 8 dereferenceable(8), ptr noundef, i1 noundef zeroext, i32 noundef) unnamed_addr #5

declare dso_local void @"??1H@@QEAA@XZ"(ptr noundef nonnull align 1 dereferenceable(1) %this) unnamed_addr #3 align 2

declare dso_local void @"??1h@@IEAA@XZ"(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #6

declare dso_local void @_CxxThrowException(ptr, ptr) #4

$"??_8c@@7B@" = comdat any
@"??_8c@@7B@" = linkonce_odr unnamed_addr constant [2 x i32] [i32 0, i32 24], comdat, align 4
$"??_R0?AVf@@@8" = comdat any
@"??_R0?AVf@@@8" = linkonce_odr global %rtti.TypeDescriptor7 { ptr @"??_7type_info@@6B@", ptr null, [8 x i8] c".?AVf@@\00" }, comdat
declare dso_local void @"??_Dc@@QEAAXXZ"(ptr noundef nonnull align 8 dereferenceable(24) %this) unnamed_addr #3 align 2


define dso_local noundef nonnull ptr @"??0p@@AEAA@XZ"(ptr noundef nonnull readnone returned align 1 dereferenceable(1) %this) unnamed_addr #0 align 2 personality ptr @__CxxFrameHandler3 {
entry:
  %q = alloca %class.c, align 8
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %q) #8
  store ptr @"??_8c@@7B@", ptr %q, align 8
  %call.i.i6 = invoke noundef ptr @"??0j@@QEAA@PEAVh@@_N@Z"(ptr noundef nonnull align 8 dereferenceable(8) %q, ptr noundef undef, i1 noundef zeroext false, i32 noundef 0)
          to label %call.i.i.noexc unwind label %catch.dispatch

call.i.i.noexc:                                   ; preds = %entry
  %o.i.i = getelementptr inbounds %class.l, ptr %q, i64 0, i32 1
  %call3.i.i7 = invoke noundef ptr @"??0i@@QEAA@W4a@@_N@Z"(ptr noundef nonnull align 1 dereferenceable(1) %o.i.i, i32 noundef 0, i1 noundef zeroext false)
          to label %call3.i.i.noexc unwind label %catch.dispatch

call3.i.i.noexc:                                  ; preds = %call.i.i.noexc
  %s.i = getelementptr inbounds %class.c, ptr %q, i64 0, i32 1
  %vbtable.i = load ptr, ptr %q, align 8
  %0 = getelementptr inbounds i32, ptr %vbtable.i, i64 1
  %vbase_offs.i = load i32, ptr %0, align 4
  %1 = sext i32 %vbase_offs.i to i64
  %g.i = getelementptr inbounds i8, ptr %q, i64 %1
  %2 = load i32, ptr %g.i, align 4
  %call3.i8 = invoke noundef ptr @"??0H@@QEAA@H@Z"(ptr noundef nonnull align 1 dereferenceable(1) %s.i, i32 noundef %2)
          to label %call3.i.noexc unwind label %catch.dispatch

call3.i.noexc:                                    ; preds = %call3.i.i.noexc
  invoke void @llvm.seh.scope.begin()
          to label %invoke.cont.i unwind label %ehcleanup.i

invoke.cont.i:                                    ; preds = %call3.i.noexc
  invoke void @llvm.seh.scope.end()
          to label %invoke.cont unwind label %ehcleanup.i

ehcleanup.i:                                      ; preds = %invoke.cont.i, %call3.i.noexc
  %3 = cleanuppad within none []
  call void @"??1H@@QEAA@XZ"(ptr noundef nonnull align 1 dereferenceable(1) %s.i) #8 [ "funclet"(token %3) ]
  cleanupret from %3 unwind label %catch.dispatch

invoke.cont:                                      ; preds = %invoke.cont.i
  invoke void @llvm.seh.scope.begin()
          to label %invoke.cont2 unwind label %ehcleanup

invoke.cont2:                                     ; preds = %invoke.cont
  invoke void @llvm.seh.scope.end()
          to label %invoke.cont3 unwind label %ehcleanup

invoke.cont3:                                     ; preds = %invoke.cont2
  invoke void @llvm.seh.scope.begin()
          to label %invoke.cont.i.i unwind label %ehcleanup.i.i

invoke.cont.i.i:                                  ; preds = %invoke.cont3
  invoke void @llvm.seh.scope.end()
          to label %invoke.cont2.i.i unwind label %ehcleanup.i.i

invoke.cont2.i.i:                                 ; preds = %invoke.cont.i.i
  invoke void @llvm.seh.scope.begin()
          to label %invoke.cont.i.i.i unwind label %ehcleanup.i.i.i

invoke.cont.i.i.i:                                ; preds = %invoke.cont2.i.i
  invoke void @llvm.seh.scope.end()
          to label %"??_Dc@@QEAAXXZ.exit" unwind label %ehcleanup.i.i.i

ehcleanup.i.i.i:                                  ; preds = %invoke.cont.i.i.i, %invoke.cont2.i.i
  %4 = cleanuppad within none []
  call void @"??1h@@IEAA@XZ"(ptr noundef nonnull align 1 dereferenceable(1) %s.i) #8 [ "funclet"(token %4) ]
  cleanupret from %4 unwind to caller

ehcleanup.i.i:                                    ; preds = %invoke.cont.i.i, %invoke.cont3
  %5 = cleanuppad within none []
  invoke void @llvm.seh.scope.begin()
          to label %invoke.cont.i2.i unwind label %ehcleanup.i4.i

invoke.cont.i2.i:                                 ; preds = %ehcleanup.i.i
  invoke void @llvm.seh.scope.end()
          to label %"??1H@@QEAA@XZ.exit.i" unwind label %ehcleanup.i4.i

ehcleanup.i4.i:                                   ; preds = %invoke.cont.i2.i, %ehcleanup.i.i
  %6 = cleanuppad within %5 []
  call void @"??1h@@IEAA@XZ"(ptr noundef nonnull align 1 dereferenceable(1) %s.i) #8 [ "funclet"(token %6) ]
  cleanupret from %6 unwind to caller

"??1H@@QEAA@XZ.exit.i":                           ; preds = %invoke.cont.i2.i
  call void @"??1h@@IEAA@XZ"(ptr noundef nonnull align 1 dereferenceable(1) %s.i) #8 [ "funclet"(token %5) ]
  cleanupret from %5 unwind to caller

"??_Dc@@QEAAXXZ.exit":                            ; preds = %invoke.cont.i.i.i
  call void @"??1h@@IEAA@XZ"(ptr noundef nonnull align 1 dereferenceable(1) %s.i) #8
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %q) #8
  ret ptr %this

ehcleanup:                                        ; preds = %invoke.cont2, %invoke.cont
  %7 = cleanuppad within none []
  invoke void @llvm.seh.scope.end()
          to label %invoke.cont4 unwind label %catch.dispatch

invoke.cont4:                                     ; preds = %ehcleanup
  call void @"??_Dc@@QEAAXXZ"(ptr noundef nonnull align 8 dereferenceable(24) %q) #8 [ "funclet"(token %7) ]
  cleanupret from %7 unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %call3.i.i.noexc, %call.i.i.noexc, %entry, %ehcleanup.i, %ehcleanup, %invoke.cont4
  %8 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %9 = catchpad within %8 [ptr @"??_R0?AVf@@@8", i32 0, ptr null]
  call void @_CxxThrowException(ptr null, ptr null) [ "funclet"(token %9) ]
  unreachable
}
