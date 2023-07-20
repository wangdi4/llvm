; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt -passes='cgscc(inline),module(ip-cloning)' -disable-output -inline-report=0x2f847 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,cgscc(inline),module(ip-cloning),inlinereportemitter' -disable-output -inline-report=0x2f8c6 < %s 2>&1 | FileCheck %s

; Check that in the compact form of the inline report, that the summary tables
; and compact callsite annotations get copied when a function is cloned. 

; CHECK-LABEL: COMPILE FUNC: _Z3fooiPFbiiE
; CHECK: INDIRECT: {{.*}}Call site is indirect
; CHECK: <C> INLINE: myfoo {{.*}}Callee has single callsite and local linkage
; CHECK: SUMMARIZED INLINED CALL SITE COUNTS
; CHECK: 1 mygoo

; CHECK-LABEL: COMPILE FUNC: _Z3fooiPFbiiE.1
; CHECK: INDIRECT: {{.*}}Call site is indirect
; CHECK: <C> INLINE: myfoo {{.*}}Callee has single callsite and local linkage
; CHECK: SUMMARIZED INLINED CALL SITE COUNTS
; CHECK: 1 mygoo

; CHECK-LABEL: COMPILE FUNC: _Z3fooiPFbiiE.2
; CHECK: INDIRECT: {{.*}}Call site is indirect
; CHECK: <C> INLINE: myfoo {{.*}}Callee has single callsite and local linkage
; CHECK: SUMMARIZED INLINED CALL SITE COUNTS
; CHECK: 1 mygoo

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@index = external local_unnamed_addr global i32, align 4
@arr = external local_unnamed_addr global [10 x i32], align 16

define internal void @mygoo() {
  ret void
}

define internal void @myfoo() {
  call void @mygoo()
  ret void
}

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

define internal fastcc i32 @_Z3fooiPFbiiE(i32 %i, ptr nocapture %fp1) unnamed_addr #0 {
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
  call void @myfoo()
  ret i32 %conv
}

attributes #0 = { noinline }

; end INTEL_FEATURE_SW_ADVANCED
