; RUN: opt -passes=instcombine -S %s | FileCheck %s

; Catchswitch blocks may only contain phi and catchswitch.

; CHECK-LABEL: bb9:
; CHECK: phi i8*
; CHECK-NEXT: catchswitch
; CHECK-LABEL: bb12:

; ModuleID = 'small.ll'
source_filename = "memory_attributes-host-x86_64-pc-windows-msvc.ii"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27045"

%struct.snork = type { i8**, i8*, [24 x i8] }
%struct.spam = type { i32*, [4 x i8], i32, %struct.wobble }
%struct.wobble = type { %struct.wobble.0, %struct.blam*, %struct.spam*, i8 }
%struct.wobble.0 = type { i32 (...)**, i64, i32, i32, i32, i64, i64, %struct.eggs*, %struct.spam.1*, %struct.bar* }
%struct.eggs = type { %struct.eggs*, i32, i32, i8* }
%struct.spam.1 = type { %struct.spam.1*, i32, void (i32, %struct.wobble.0*, i32)* }
%struct.bar = type { [8 x i8], %struct.bar.2* }
%struct.bar.2 = type { %struct.pluto, %struct.pluto**, i64, i32, i8, %struct.wombat }
%struct.pluto = type { %struct.quux, i32 }
%struct.quux = type { i32 (...)** }
%struct.wombat = type { i8*, i8 }
%struct.blam = type { i32 (...)**, i8*, i8*, i8**, i8**, i8*, i8*, i8**, i8**, i32, i32, i32*, i32*, %struct.bar* }
%struct.bar.3 = type { %struct.ham }
%struct.ham = type { %struct.bar.4, %struct.blam.5 }
%struct.bar.4 = type { i32 (...)** }
%struct.blam.5 = type { %struct.baz }
%struct.baz = type { %struct.spam.6 }
%struct.spam.6 = type { %struct.widget }
%struct.widget = type { %struct.eggs.7, i64, i64 }
%struct.eggs.7 = type { i8*, [8 x i8] }
%struct.spam.8 = type { %struct.blam.9, %struct.blam.5, i32, %struct.spam.11 }
%struct.blam.9 = type { i32 (...)**, %struct.wobble.10 }
%struct.wobble.10 = type { i8*, i8 }
%struct.spam.11 = type { %struct.barney }
%struct.barney = type { %struct.snork.12*, %struct.eggs.16* }
%struct.snork.12 = type { %struct.quux.13 }
%struct.quux.13 = type { %struct.wombat.14 }
%struct.wombat.14 = type { %struct.widget.15*, %struct.eggs.16* }
%struct.widget.15 = type opaque
%struct.eggs.16 = type { i32 (...)**, i32, i32 }

@global = external global %struct.snork
@global.1 = external dllimport global %struct.spam, align 8
@global.2 = external dso_local unnamed_addr constant [31 x i8], align 1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

define dso_local void @main() local_unnamed_addr #1 personality i32 (...)* @__CxxFrameHandler3 {
bb:
  %tmp = alloca %struct.bar.3, align 8
  invoke void @baz.3()
          to label %bb1 unwind label %bb3

bb1:                                              ; preds = %bb
  invoke void @quux()
          to label %bb2 unwind label %bb6

bb2:                                              ; preds = %bb1
  unreachable

bb3:                                              ; preds = %bb
  %tmp4 = cleanuppad within none []
  %tmp5 = bitcast %struct.bar.3* %tmp to i8*
  cleanupret from %tmp4 unwind label %bb9

bb6:                                              ; preds = %bb1
  %tmp7 = cleanuppad within none []
  %tmp8 = bitcast %struct.bar.3* %tmp to i8*
  cleanupret from %tmp7 unwind label %bb9

bb9:                                              ; preds = %bb6, %bb3
  %tmp10 = phi i8* [ %tmp8, %bb6 ], [ %tmp5, %bb3 ]
  %tmp11 = catchswitch within none [label %bb12] unwind label %bb15

bb12:                                             ; preds = %bb9
  %tmp13 = catchpad within %tmp11 [%struct.snork* @global, i32 8, %struct.spam.8** undef]
  invoke void @baz() [ "funclet"(token %tmp13) ]
          to label %bb14 unwind label %bb15

bb14:                                             ; preds = %bb12
  unreachable

bb15:                                             ; preds = %bb12, %bb9
  %tmp16 = cleanuppad within none []
  call void @llvm.lifetime.end.p0i8(i64 40, i8* nonnull %tmp10) #3
  cleanupret from %tmp16 unwind to caller
}

declare dso_local void @baz() local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #2

declare dllimport void @baz.3() unnamed_addr #1

declare dso_local void @quux() unnamed_addr #1 align 2

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { "unsafe-fp-math"="true" }
attributes #2 = { nofree }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
