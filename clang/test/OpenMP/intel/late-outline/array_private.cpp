// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -O0 -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-use-single-elem-array-funcs \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//CHECK-DAG: [[TF:.*]] = type { float, float }
//CHECK-DAG: [[TD:.*]] = type { double, double }

template<typename _Tp>
struct complex {
      complex(const _Tp& __r = _Tp(), const _Tp& __i = _Tp());
      complex(const complex&) = default;
      complex<_Tp>& operator=(const _Tp&);
      ~complex();
private:
      _Tp _M_real;
      _Tp _M_imag;
};

// Checks for single element ctors/copyassigns/dtors for vlas.

//CHECK: define {{.*}}fn_vlas
void fn_vlas(int c) {
  //CHECK: [[CF:%cf[0-9]*]] = alloca [[TF]],

  complex<double> e[c];
  //CHECK: [[V:%vla[0-9]*]] = alloca [[TD]],

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(ptr [[V]],
  //CHECK-SAME: _ZTS7complexIdE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIdE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(e)
  for (int d = 0; d < 1; d++);

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(ptr [[V]],
  //CHECK-SAME: _ZTS7complexIdE.omp.copy_constr,
  //CHECK-SAME: _ZTS7complexIdE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for firstprivate(e)
  for (int d = 0; d < 1; d++);

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.LASTPRIVATE:NONPOD"(ptr [[V]],
  //CHECK-SAME: _ZTS7complexIdE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIdE.omp.copy_assign,
  //CHECK-SAME: _ZTS7complexIdE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for lastprivate(e)
  for (int d = 0; d < 1; d++);

  // Element type seen first
  complex<float> cf;
  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(ptr [[CF]],
  //CHECK-SAME: _ZTS7complexIfE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIfE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(cf)
  for (int d = 0; d < 1; d++);

  complex<float> acf[c];
  //CHECK: [[V:%vla[0-9]*]] = alloca [[TF]],

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(ptr [[V]],
  //CHECK-SAME: _ZTS7complexIfE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIfE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(acf)
  for (int d = 0; d < 1; d++);
}

//CHECK: define {{.*}}ptr @_ZTS7complexIdE.omp.def_constr{{.*}}{
//CHECK-NOT: arrayctor
//CHECK: ret ptr
//CHECK: }

//CHECK: define {{.*}}void @_ZTS7complexIdE.omp.destr(ptr{{.*}}{
//CHECK-NOT: arraydestroy
//CHECK: ret void
//CHECK: }

//CHECK: define {{.*}}void @_ZTS7complexIdE.omp.copy_constr(ptr{{.*}}{
//CHECK-NOT: arrayctor
//CHECK: ret void
//CHECK: }

//CHECK: define {{.*}}void @_ZTS7complexIdE.omp.copy_assign(ptr{{.*}}{
//CHECK-NOT: arraycpy
//CHECK: ret void
//CHECK: }

//CHECK: define {{.*}}ptr @_ZTS7complexIfE.omp.def_constr{{.*}}{
//CHECK-NOT: arrayctor
//CHECK: ret ptr
//CHECK: }

//CHECK: define {{.*}}void @_ZTS7complexIfE.omp.destr(ptr{{.*}}{
//CHECK-NOT: arraydestroy
//CHECK: ret void
//CHECK: }

// Checks for single element ctors/copyassigns/dtors for fixed arrays.

//CHECK: define {{.*}}fn_fixed_arrays
void fn_fixed_arrays() {
  //CHECK: [[E:%e[0-9]*]] = alloca [4 x [[TD]]],
  //CHECK: [[CF:%cf[0-9]*]] = alloca [[TF]],
  //CHECK: [[ACF:%acf[0-9]*]] = alloca [8 x [[TF]]],

  complex<double> e[4];

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(ptr [[E]],
  //CHECK-SAME: _ZTS7complexIdE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIdE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(e)
  for (int d = 0; d < 1; d++);

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(ptr [[E]],
  //CHECK-SAME: _ZTS7complexIdE.omp.copy_constr,
  //CHECK-SAME: _ZTS7complexIdE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for firstprivate(e)
  for (int d = 0; d < 1; d++);

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.LASTPRIVATE:NONPOD"(ptr [[E]],
  //CHECK-SAME: _ZTS7complexIdE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIdE.omp.copy_assign,
  //CHECK-SAME: _ZTS7complexIdE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for lastprivate(e)
  for (int d = 0; d < 1; d++);

  // Element type seen first
  complex<float> cf;
  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(ptr [[CF]],
  //CHECK-SAME: _ZTS7complexIfE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIfE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(cf)
  for (int d = 0; d < 1; d++);

  complex<float> acf[8];

  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(ptr [[ACF]],
  //CHECK-SAME: _ZTS7complexIfE.omp.def_constr,
  //CHECK-SAME: _ZTS7complexIfE.omp.destr)
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(acf)
  for (int d = 0; d < 1; d++);
}
// end INTEL_COLLAB
