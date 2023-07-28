; RUN: opt -aa-pipeline=tbaa,basic-aa -passes='function(require<tbaa>),cgscc(inline),function(tbaa-prop,sroa,require<basic-aa>,instcombine,gvn),module(globalopt)' -S < %s | FileCheck %s
;
; The compiler should recover the tbaa information for the expression
; s.geta(i) and s.getb(j) so that these two expressions can be determined
; to be non-overlapped.
;
; struct S {
;  int a[4];
;  int b[4];
;
;  int& geta(int i) {
;    return a[i];
;  }
;  int& getb(int i) {
;    return b[i];
;  }
;};
;
;int foo(S& s, int i, int j) {
;  s.geta(i) = 0;
;  s.getb(j) = 1;
;  return s.geta(i);
;}
;
;
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { [4 x i32], [4 x i32] }

$_ZN1S4getaEi = comdat any

$_ZN1S4getbEi = comdat any

; Function Attrs: uwtable
define i32 @_Z3fooR1Sii(ptr dereferenceable(32) %s, i32 %i, i32 %j)  {
entry:
  %s.addr = alloca ptr, align 8
  %i.addr = alloca i32, align 4
  %j.addr = alloca i32, align 4
  store ptr %s, ptr %s.addr, align 8, !tbaa !1
  store i32 %i, ptr %i.addr, align 4, !tbaa !4
  store i32 %j, ptr %j.addr, align 4, !tbaa !4
  %0 = load ptr, ptr %s.addr, align 8
  %1 = load i32, ptr %i.addr, align 4, !tbaa !4
  %call = call dereferenceable(4) ptr @_ZN1S4getaEi(ptr %0, i32 %1)
  store i32 0, ptr %call, align 4, !tbaa !4
  %2 = load ptr, ptr %s.addr, align 8
  %3 = load i32, ptr %j.addr, align 4, !tbaa !4
  %call1 = call dereferenceable(4) ptr @_ZN1S4getbEi(ptr %2, i32 %3)
  store i32 1, ptr %call1, align 4, !tbaa !4
  %4 = load ptr, ptr %s.addr, align 8
  %5 = load i32, ptr %i.addr, align 4, !tbaa !4
  %call2 = call dereferenceable(4) ptr @_ZN1S4getaEi(ptr %4, i32 %5)
  %6 = load i32, ptr %call2, align 4, !tbaa !4
; CHECK: ret i32 0
  ret i32 %6
}

; Function Attrs: nounwind uwtable
define linkonce_odr dereferenceable(4) ptr @_ZN1S4getaEi(ptr %this, i32 %i)  comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %i.addr = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8, !tbaa !6
  store i32 %i, ptr %i.addr, align 4, !tbaa !4
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load i32, ptr %i.addr, align 4, !tbaa !4
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [4 x i32], ptr %this1, i64 0, i64 %idxprom
  %1 = call ptr @llvm.intel.fakeload.p0(ptr %arrayidx, metadata !8)
  ret ptr %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dereferenceable(4) ptr @_ZN1S4getbEi(ptr %this, i32 %i) #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %i.addr = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8, !tbaa !6
  store i32 %i, ptr %i.addr, align 4, !tbaa !4
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load i32, ptr %i.addr, align 4, !tbaa !4
  %idxprom = sext i32 %0 to i64
  %b = getelementptr inbounds %struct.S, ptr %this1, i32 0, i32 1
  %arrayidx = getelementptr inbounds [4 x i32], ptr %b, i64 0, i64 %idxprom
  %1 = call ptr @llvm.intel.fakeload.p0(ptr %arrayidx, metadata !11)
  ret ptr %1
}

; CHECK-NOT: call ptr @llvm.intel.fakeload

; Function Attrs: nounwind

declare ptr @llvm.intel.fakeload.p0(ptr, metadata)

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17977)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSP1S", !2, i64 0}
!8 = !{!9, !5, i64 0}
!9 = !{!"struct@_ZTS1S", !10, i64 0, !10, i64 16}
!10 = !{!"array@_ZTSA4_i", !5, i64 0}
!11 = !{!9, !5, i64 16}

