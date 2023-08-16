; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; extern "C" void __assert_fail(char *, char *, int, const char *);
; typedef double a;
; template <int b> struct c {
;   int d() { return b; }
; };
; template <class, int b> int f() {
;   c<b> g;
;   for (int h = 0; h < g.d(); ++h)
;     ;
; }
; template <class e, int> class i {
; public:
;   i(e);
;   int j;
; };
; template <class e, int k> i<e, k>::i(e) : j(f<e, k>()) {}
; class l {
; public:
;   template <class m> void n(m) const;
; };
; class aa {
; public:
;   aa(l);
;   int o();
; };
; int p;
; template <int, int r, class m> void q(m ab, aa ac) {
; #pragma omp teams distribute firstprivate(ac)
;   for (int h = 0; h < p; ++h)
;     ab.template operator()<r>(ac, h, 0);
; }
; template <class m> void l::n(m ab) const {
;   aa a(*this);
;   q<0, 1>(ab, a);
; }
; template <int> class ad {
; public:
;   ad(int, int, aa ac) : s(ac) {}
;   void ae() {
;     t and s.o() ? void() : __assert_fail("", "", 4, __PRETTY_FUNCTION__);
;   }
;   int t;
;   aa s;
; };
; template <class e> class w {
;   int x;
;
; public:
;   e af(int) const;
; };
; template <class e> e w<e>::af(int) const {
;   x ? void() : __assert_fail("", "", 7, __PRETTY_FUNCTION__);
;   __assert_fail("", "", 4, __PRETTY_FUNCTION__);
; }
; template <class e, int k> class u {
; public:
;   void ag(w<e> ah, ad<k> v) {
;     i<e, k>(2.0);
;     v.ae();
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;     ah.af(0);
;   }
; };
; typedef enum { ai } aj;
; template <class e, aj> class ak { void al(w<e> &, l const &); };
; template <class e, aj am> void ak<e, am>::al(w<e> &ah, l const &an) {
;   an.n([=]<int b>(aa ac, int v, int ae) {
;     ad<b> site(v, ae, ac);
;     u<e, b> ao;
;     ao.ag(ah, site);
;   });
; }
; template void ak<a, ai>::al(w<a> &, l const &);

; This LIT test is generated from the source above and simplified. It validates that if there doesn't exist a block with both the ZTT block and the loop exit region as its predecessors, the destruction of the firstprivate symbol takes place within the exit block of WRegion.

; CHECK: DIR.OMP.END.DISTRIBUTE.628:
; CHECK-NEXT: call void @_ZTS2aa.omp.destr(ptr %ac.fpriv)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64_gen"

%class.aa = type { i8 }

define linkonce_odr void @_Z1qILi0ELi1EZN2akIdL2aj0EE2alER1wIdERK1lEUlTni2aaiiE_EvT1_S9_(ptr %ac) {
DIR.OMP.TEAMS.324:
  %.omp.ub = alloca i32, i32 0, align 4
  br label %DIR.OMP.DISTRIBUTE.4

DIR.OMP.DISTRIBUTE.4:                             ; preds = %DIR.OMP.TEAMS.324
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED.WILOCAL"(ptr %ac, %class.aa zeroinitializer, i32 1, ptr null, ptr @_ZTS2aa.omp.destr),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %ac, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]
  %cmp4.not = icmp sgt i32 0, 0
  br i1 false, label %DIR.OMP.END.DISTRIBUTE.628, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.DISTRIBUTE.4
  br label %for.cond.i.i.i.i

for.cond.i.i.i.i:                                 ; preds = %for.inc.i.i.i.i, %omp.inner.for.body
  %h.0.i.i.i.i = phi i32 [ 0, %omp.inner.for.body ], [ %inc.i.i.i.i, %for.inc.i.i.i.i ]
  %cmp.i.i.i.i = icmp slt i32 %h.0.i.i.i.i, 1
  br i1 %cmp.i.i.i.i, label %for.inc.i.i.i.i, label %_ZN1iIdLi1EEC2Ed.exit.i.i

for.inc.i.i.i.i:                                  ; preds = %for.cond.i.i.i.i
  %inc.i.i.i.i = add i32 %h.0.i.i.i.i, 0
  br label %for.cond.i.i.i.i

_ZN1iIdLi1EEC2Ed.exit.i.i:                        ; preds = %for.cond.i.i.i.i
  ret void

DIR.OMP.END.DISTRIBUTE.628:                       ; preds = %DIR.OMP.DISTRIBUTE.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @_ZTS2aa.omp.destr(ptr)
