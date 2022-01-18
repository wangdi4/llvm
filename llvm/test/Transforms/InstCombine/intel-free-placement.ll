; RUN: opt -passes="instcombine" -S %s | FileCheck %s
; CHECK: call{{.*}}_ZdlPvRKSt9nothrow_t

; CMPLRLLVM-27664
; _Zdl... is a "placement delete" which may ignore its arguments. The undef
; arg should not be treated as undefined behavior.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.foo = type { i32 }
%struct.pluto = type { i8 }

@global = external hidden global i8
@global.1 = external dso_local global %struct.foo, align 4
@global.2 = external dso_local unnamed_addr global i32, align 4
@global.3 = external dso_local constant i8*
@global.4 = external dso_local global %struct.pluto, align 1
@global.5 = external dso_local unnamed_addr constant [12 x i8], align 1

declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr

declare dso_local fastcc void @wombat() unnamed_addr section ".text.startup"

declare dso_local void @barney(%struct.foo* nonnull align 4 dereferenceable(4), i32) unnamed_addr align 2

declare dso_local nonnull i8* @_Znwm(i64) local_unnamed_addr

declare dso_local noalias noundef i8* @malloc(i64 noundef) local_unnamed_addr

declare dso_local noalias i8* @_ZnwmRKSt9nothrow_t(i64, %struct.pluto* nonnull align 1) local_unnamed_addr

declare dso_local void @_ZdlPvRKSt9nothrow_t(i8*, %struct.pluto* nonnull align 1) local_unnamed_addr

define dso_local i32 @main(i32 %arg, i8** %arg1) local_unnamed_addr personality i8* bitcast (i32 (...)* @hoge to i8*) {
bb:
  invoke void @widget(%struct.foo* nonnull align 4 dereferenceable(4) undef)
          to label %bb2 unwind label %bb3

bb2:                                              ; preds = %bb
  unreachable

bb3:                                              ; preds = %bb
  %tmp = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast (i8** @global.3 to i8*)
  call void @_ZdlPvRKSt9nothrow_t(i8* undef, %struct.pluto* nonnull align 1 undef)
  resume { i8*, i32 } %tmp
}

declare dso_local void @widget(%struct.foo* nonnull align 4 dereferenceable(4)) unnamed_addr align 2

declare dso_local i32 @hoge(...)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #0

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

declare dso_local void @__cxa_end_catch() local_unnamed_addr

declare dso_local noalias i8* @__cxa_allocate_exception(i64) local_unnamed_addr

declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr

attributes #0 = { nounwind readnone }

