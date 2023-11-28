; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; extern "C" void __assert_fail(char *, char *, int, const char *);
; typedef double a;
; struct b {
;   int c();
; };
; template <class, int> int e() {
;   b f;
;   for (int g = 0; g < f.c(); ++g)
;     ;
; }
; template <class d, int = 0> class h {
; public:
;   h(d);
;   int i;
; };
; template <class d, int j> h<d, j>::h(d) : i(e<d, j>()) {}
; class k {
; public:
;   template <class l> void m(l) const;
; };
; class n {
; public:
;   n(k);
; };
; int o;
; template <int, int p, class l> void r(l aa, n ab) {
; #pragma omp teams distribute firstprivate(ab)
;   for (int g = 0; g < o; ++g)
;     aa.template operator()<p>(ab, g, 0);
; }
; template <class l> void k::m(l aa) const {
;   n a(*this);
;   r<0, 1>(aa, a);
; }
; template <int> class q {
; public:
;   q(int, int, n ab) : ac(ab) {}
;   void ad() { ae < 0 ? void() : __assert_fail("", "", 4, __PRETTY_FUNCTION__); }
;   int ae;
;   n ac;
; };
; template <class> class s {};
; template <class d, int j> class t {
; public:
;   void af(s<d>, q<j> v) {
;     h(2.0);
;     v.ad();
;   }
; };
; typedef enum { w } x;
; template <class d, x> class u { void ag(s<d> &, k const &); };
; template <class d, x ah> void u<d, ah>::ag(s<d> &ai, k const &aj) {
;   aj.m([=]<int ak>(n ab, int v, int ad) {
;     q<ak> al(v, ad, ab);
;     t<d, ak> am;
;     am.af(ai, al);
;   });
; }
; template void u<a, w>::ag(s<a> &, k const &);

; This LIT test is generated from the source above and simplified. It validates that if there exists a block with both the ZTT block and the loop exit region as its predecessors, the destruction of the firstprivate symbol takes place within the loop exit region.

; CHECK: loop.region.exit.split:
; CHECK-NEXT: call void @_ZTS1n.omp.destr(ptr %ab.fpriv)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64_gen"

%class.n = type { i8 }

define linkonce_odr void @_Z1rILi0ELi1EZN1uIdL1x0EE2agER1sIdERK1kEUlTni1niiE_EvT1_S9_(ptr %ab) {
DIR.OMP.TEAMS.324:
  %.omp.ub = alloca i32, i32 0, align 4
  br label %DIR.OMP.TEAMS.2

DIR.OMP.TEAMS.2:                                  ; preds = %DIR.OMP.TEAMS.324
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]
  br label %DIR.OMP.DISTRIBUTE.4

DIR.OMP.DISTRIBUTE.4:                             ; preds = %DIR.OMP.TEAMS.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED.WILOCAL"(ptr %ab, %class.n zeroinitializer, i32 1, ptr null, ptr @_ZTS1n.omp.destr),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %ab, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]
  br label %for.cond.i.i.i.i

for.cond.i.i.i.i:                                 ; preds = %for.inc.i.i.i.i, %DIR.OMP.DISTRIBUTE.4
  %g.0.i.i.i.i = phi i32 [ 0, %DIR.OMP.DISTRIBUTE.4 ], [ %inc.i.i.i.i, %for.inc.i.i.i.i ]
  %call.i.i.i.i = load i32, ptr null, align 4
  %cmp.i.i.i.i = icmp slt i32 %g.0.i.i.i.i, %call.i.i.i.i
  br i1 %cmp.i.i.i.i, label %for.inc.i.i.i.i, label %_ZN1hIdLi0EEC2Ed.exit.i.i

for.inc.i.i.i.i:                                  ; preds = %for.cond.i.i.i.i
  %inc.i.i.i.i = add i32 %g.0.i.i.i.i, 0
  br label %for.cond.i.i.i.i

_ZN1hIdLi0EEC2Ed.exit.i.i:                        ; preds = %for.cond.i.i.i.i
  unreachable

DIR.OMP.END.DISTRIBUTE.6:                         ; No predecessors!
  br label %DIR.OMP.END.DISTRIBUTE.628

DIR.OMP.END.DISTRIBUTE.628:                       ; preds = %DIR.OMP.END.DISTRIBUTE.6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISTRIBUTE"() ]
  br label %DIR.OMP.END.TEAMS.7

DIR.OMP.END.TEAMS.7:                              ; preds = %DIR.OMP.END.DISTRIBUTE.628
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @_ZTS1n.omp.destr(ptr)

uselistorder ptr @llvm.directive.region.entry, { 1, 0 }
uselistorder ptr @llvm.directive.region.exit, { 1, 0 }
