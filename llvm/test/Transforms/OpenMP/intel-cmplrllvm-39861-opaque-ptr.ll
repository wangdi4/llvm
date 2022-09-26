; RUN: opt -S -opaque-pointers -passes='cgscc(openmp-opt-cgscc)' < %s 2>&1 | FileCheck %s

; CMPLRLLVM-39892: Check that @printf is converted to a declare without triggering
; an assertion.

; CHECK: declare dso_local i32 @printf(

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.30.30705"

%struct.ident_t = type { i32, i32, i32, i32, ptr }

$__local_stdio_printf_options = comdat any

$"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA" = comdat any

@"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA" = weak_odr dso_local global i64 0, comdat, align 8
@str = private unnamed_addr constant [16 x i8] c"passed bran.cpp\00", align 1
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0 }

define dso_local noundef i32 @main() local_unnamed_addr {
bb:
  %i = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  tail call void @__kmpc_end(ptr nonnull @.kmpc_loc.0.0)
  ret i32 0
}

define internal i32 @printf(ptr noundef %arg, ...) unnamed_addr {
bb:
  %i = alloca ptr, align 8
  %i1 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i1)
  call void @llvm.va_start(ptr nonnull %i1)
  %i2 = load ptr, ptr %i, align 8, !tbaa !16
  %i3 = call ptr @__acrt_iob_func(i32 noundef 1)
  %i4 = call ptr @__local_stdio_printf_options()
  %i5 = load i64, ptr %i4, align 8, !tbaa !20
  %i6 = call i32 @__stdio_common_vfprintf(i64 noundef %i5, ptr noundef %i3, ptr noundef %arg, ptr noundef null, ptr noundef %i2)
  call void @llvm.va_end(ptr nonnull %i1)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i1)
  ret i32 0 
}

declare dso_local void @exit(i32 noundef) local_unnamed_addr

declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr

declare void @__kmpc_end(ptr) local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #0

declare dso_local ptr @__acrt_iob_func(i32 noundef) local_unnamed_addr

define weak_odr dso_local ptr @__local_stdio_printf_options() local_unnamed_addr comdat {
bb:
  ret ptr @"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA"
}

declare dso_local i32 @__stdio_common_vfprintf(i64 noundef, ptr nocapture noundef, ptr noundef readonly, ptr nocapture noundef, ptr noundef) local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}
!nvvm.annotations = !{}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14, !15}

!0 = !{!"/DEFAULTLIB:libcmt.lib"}
!1 = !{!"/DEFAULTLIB:libircmt.lib"}
!2 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!3 = !{!"/DEFAULTLIB:libdecimal.lib"}
!4 = !{!"/DEFAULTLIB:libmmt.lib"}
!5 = !{!"/DEFAULTLIB:oldnames.lib"}
!6 = !{!"/DEFAULTLIB:libiomp5md.lib"}
!7 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!9 = !{i32 1, !"wchar_size", i32 2}
!10 = !{i32 7, !"openmp", i32 50}
!11 = !{i32 7, !"PIC Level", i32 2}
!12 = !{i32 7, !"uwtable", i32 2}
!13 = !{i32 1, !"ThinLTO", i32 0}
!14 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!15 = !{i32 1, !"LTOPostLink", i32 1}
!16 = !{!17, !17, i64 0}
!17 = !{!"pointer@?APEAD", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C++ TBAA"}
!20 = !{!21, !21, i64 0}
!21 = !{!"long long", !18, i64 0}
