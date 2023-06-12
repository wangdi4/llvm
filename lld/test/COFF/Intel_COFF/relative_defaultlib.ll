; REQUIRES: x86
; UNSUPPORTED: !system-windows

; Tests if lld-link supports /DEFAULTLIB relative filepaths correctly. A
; relative filepath to reldef.lib is provided as a DEFAULTLIB entry below.
; Linking should not produce a 'could not open' error.

; Generate reldef.lib and place it in a subdirectory
; RUN: llc %S/Inputs/relative_defaultlib_lib.ll -o %t.obj -filetype=obj
; RUN: llvm-lib /out:reldef.lib %t.obj
; RUN: mkdir %t-dir
; RUN: mkdir %t-dir/reldef
; RUN: mkdir %t-dir/reldef/libdir
; RUN: mv reldef.lib %t-dir/reldef/libdir/.

; Link object
; RUN: llc %s -o %t2.obj -filetype=obj
; RUN: lld-link %t2.obj /LIBPATH:%t-dir/reldef | FileCheck %s --allow-empty
; RUN: rm -rf %t-dir

; CHECK-NOT: lld-link: error: could not open

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() local_unnamed_addr {
entry:
  %call = tail call noundef i32 @"?foo@@YAHH@Z"(i32 noundef 32)
  ret i32 0
}

declare dso_local noundef i32 @"?foo@@YAHH@Z"(i32 noundef) local_unnamed_addr

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !6}

!0 = !{!"/DEFAULTLIB:libcmt.lib"}
!1 = !{!"/DEFAULTLIB:libircmt.lib"}
!2 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!3 = !{!"/DEFAULTLIB:libdecimal.lib"}
!4 = !{!"/DEFAULTLIB:libmmt.lib"}
!5 = !{!"/DEFAULTLIB:oldnames.lib"}
!6 = !{!"/DEFAULTLIB:libdir/reldef.lib"}

