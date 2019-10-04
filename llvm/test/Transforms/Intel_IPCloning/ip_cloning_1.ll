; REQUIRES: asserts
; It checks two function clones are created when -ip-cloning is enabled.
; _Z3fooiPFbiiE is cloned based on @_Z8compare1ii and @_Z8compare2ii
; function address constants, which are passed as 2nd argument at
; call-sites of _Z3fooiPFbiiE.

; RUN: opt < %s -ip-cloning -debug-only=ipcloning -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -debug-only=ipcloning -disable-output 2>&1 | FileCheck %s

; CHECK: Cloned call:
; CHECK: Cloned call:

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@index = external local_unnamed_addr global i32, align 4
@arr = external local_unnamed_addr global [10 x i32], align 16

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal zeroext i1 @_Z8compare1ii(i32 %a, i32 %b)  {
entry:
  %cmp = icmp sgt i32 %a, %b
  ret i1 %cmp
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal zeroext i1 @_Z8compare2ii(i32 %a, i32 %b)  {
entry:
  %cmp = icmp slt i32 %a, %b
  ret i1 %cmp
}

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr  {
entry:
  %0 = load i32, i32* @index, align 4
  %call = tail call fastcc i32 @_Z3fooiPFbiiE(i32 %0, i1 (i32, i32)* nonnull @_Z8compare1ii)
  %call1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %call)
  %1 = load i32, i32* @index, align 4
  %add = add nsw i32 %1, 1
  %call2 = tail call fastcc i32 @_Z3fooiPFbiiE(i32 %add, i1 (i32, i32)* nonnull @_Z8compare2ii)
  %call3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %call2)
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr 

; Function Attrs: noinline norecurse uwtable
define internal fastcc i32 @_Z3fooiPFbiiE(i32 %i, i1 (i32, i32)* nocapture %fp1) unnamed_addr  {
entry:
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %i, 1
  %idxprom1 = sext i32 %add to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], [10 x i32]* @arr, i64 0, i64 %idxprom1
  %1 = load i32, i32* %arrayidx2, align 4
  %call = tail call zeroext i1 %fp1(i32 %0, i32 %1)
  %conv = zext i1 %call to i32
  ret i32 %conv
}

