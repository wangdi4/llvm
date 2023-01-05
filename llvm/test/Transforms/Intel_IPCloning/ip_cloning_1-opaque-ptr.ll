; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; It checks two function clones are created when -ip-cloning is enabled.
; _Z3fooiPFbiiE is cloned based on @_Z8compare1ii and @_Z8compare2ii
; function address constants, which are passed as 2nd argument at
; call-sites of _Z3fooiPFbiiE.

; RUN: opt  -opaque-pointers < %s -passes='module(ip-cloning)' -debug-only=ipcloning -disable-output 2>&1 | FileCheck %s

; CHECK: Cloned call:
; CHECK: Cloned call:

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@index = external local_unnamed_addr global i32, align 4
@arr = external local_unnamed_addr global [10 x i32], align 16

define internal zeroext i1 @_Z8compare1ii(i32 %a, i32 %b) {
entry:
  %cmp = icmp sgt i32 %a, %b
  ret i1 %cmp
}

define internal zeroext i1 @_Z8compare2ii(i32 %a, i32 %b) {
entry:
  %cmp = icmp slt i32 %a, %b
  ret i1 %cmp
}

define i32 @main() local_unnamed_addr {
entry:
  %i = load i32, ptr @index, align 4
  %call = tail call fastcc i32 @_Z3fooiPFbiiE(i32 %i, ptr nonnull @_Z8compare1ii)
  %call1 = tail call i32 (ptr, ...) @printf(ptr getelementptr inbounds ([4 x i8], ptr @.str, i64 0, i64 0), i32 %call)
  %i1 = load i32, ptr @index, align 4
  %add = add nsw i32 %i1, 1
  %call2 = tail call fastcc i32 @_Z3fooiPFbiiE(i32 %add, ptr nonnull @_Z8compare2ii)
  %call3 = tail call i32 (ptr, ...) @printf(ptr getelementptr inbounds ([4 x i8], ptr @.str, i64 0, i64 0), i32 %call2)
  ret i32 0
}

declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr

define internal fastcc i32 @_Z3fooiPFbiiE(i32 %i, ptr nocapture %fp1) unnamed_addr {
entry:
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @arr, i64 0, i64 %idxprom
  %i1 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %i, 1
  %idxprom1 = sext i32 %add to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @arr, i64 0, i64 %idxprom1
  %i2 = load i32, ptr %arrayidx2, align 4
  %call = tail call zeroext i1 %fp1(i32 %i1, i32 %i2)
  %conv = zext i1 %call to i32
  ret i32 %conv
}
; end INTEL_FEATURE_SW_ADVANCED
