// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:   -triple x86_64-unknown-linux-gnu -emit-pch -o %t %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:   -triple x86_64-unknown-linux-gnu -include-pch %t %s     \
// RUN:   | FileCheck %s

#ifndef HEADER
#define HEADER

struct S1 {
  ~S1() {}
};

struct S2 {
  S2() {}
};

struct S3 {
  S3() {}
  S3(const S3& s) : i(s.i) {}
  ~S3() {}
  int i;
};

struct S4 {
  int i;
};

void foo() {}
void bar(int);

int glob_int = 2;
int *glob_ptr;

// CHECK-LABEL: @main
int main(int argc, char **argv) {
  // CHECK: alloca i32,
  // CHECK: [[ARGC_ADDR:%argc.addr.*]] = alloca i32,
  // CHECK: [[ARGV_ADDR:%argv.addr.*]] = alloca ptr,
  // CHECK: [[S1_ADDR:%s1.*]] = alloca %struct.S1,
  // CHECK: [[S2_ADDR:%s2.*]] = alloca %struct.S2,
  // CHECK: [[S3_ADDR:%s3.*]] = alloca %struct.S3,
  // CHECK: [[ARR1_ADDR:%arr1.*]] = alloca [10 x %struct.S1],
  // CHECK: [[ARR2_ADDR:%arr2.*]] = alloca [10 x %struct.S2],
  // CHECK: [[S3ARR_ADDR:%s3arr.*]] = alloca [10 x %struct.S3],
  // CHECK: [[ARGCREF_ADDR:%argcref.*]] = alloca ptr,
  // CHECK: [[Z_ADDR:%z.*]] = alloca i32,
  // CHECK: [[ZARR_ADDR:%zarr.*]] = alloca [20 x i32],
  // CHECK: [[N1_ADDR:%n1.*]] = alloca i64,
  // CHECK: [[N2_ADDR:%.+]] = alloca i64,
  // CHECK: [[N3_ADDR:%.+]] = alloca i64,
  // CHECK: [[N4_ADDR:%.+]] = alloca i64,
  // CHECK: [[N5_ADDR:%.+]] = alloca i64,
  // CHECK: [[N6_ADDR:%.+]] = alloca i64,
  // CHECK: [[N7_ADDR:%.+]] = alloca i64,
  // CHECK: [[N8_ADDR:%.+]] = alloca i64,
  // CHECK: [[N9_ADDR:%.+]] = alloca i64,
  // CHECK: [[N10_ADDR:%n10.*]] = alloca i64,
  // CHECK: [[A_ADDR:%a.*]] = alloca ptr,
  // CHECK: [[B_ADDR:%b.*]] = alloca ptr,
  // CHECK: [[S4:%s4.*]] = alloca %struct.S4,
  // CHECK: call ptr @llvm.stacksave()
  // CHECK: [[ARR3_ADDR:%.+]] = alloca double, i64
  S1 s1;
  S2 s2;
  S3 s3;
  S1 arr1[10];
  S2 arr2[10];
  S3 s3arr[10];
  double arr3[argc];
  int &argcref = argc;
  int z = 4;
  int zarr[20];

// CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
// CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[ARGV_ADDR]]
// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[S1_ADDR]],
// CHECK-SAME: ptr @_ZTS2S1.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S1.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[S2_ADDR]],
// CHECK-SAME: ptr @_ZTS2S2.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S2.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[ARR1_ADDR]],
// CHECK-SAME: ptr @_ZTS2S1.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S1.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[ARR2_ADDR]],
// CHECK-SAME: ptr @_ZTS2S2.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S2.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[ARR3_ADDR]]

// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr [[S3_ADDR]],
// CHECK-SAME: ptr @_ZTS2S3.omp.copy_constr,
// CHECK-SAME: ptr @_ZTS2S3.omp.destr)

// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(
// CHECK-SAME: ptr [[S3ARR_ADDR]],
// CHECK-SAME: ptr @_ZTS2S3.omp.copy_constr,
// CHECK-SAME: ptr @_ZTS2S3.omp.destr)

// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[Z_ADDR]]
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[ZARR_ADDR]]
#pragma omp parallel shared(argv) private(s1, s2, arr1, arr2, arr3) \
                                  firstprivate(s3) firstprivate(s3arr) \
                                  firstprivate(z,zarr)
  // CHECK: call void @_Z3foov()
  foo();
// CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"()

// CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
// CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[ARGV_ADDR]]

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[S1_ADDR]],
// CHECK-SAME: ptr @_ZTS2S1.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S1.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[S2_ADDR]],
// CHECK-SAME: ptr @_ZTS2S2.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S2.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[ARR1_ADDR]],
// CHECK-SAME: ptr @_ZTS2S1.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S1.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[ARR2_ADDR]],
// CHECK-SAME: ptr @_ZTS2S2.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S2.omp.destr)

// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[ARR3_ADDR]]

// CHECK-SAME: "QUAL.OMP.LINEAR:TYPED"(ptr [[ARGC_ADDR]]
#pragma omp parallel for shared(argv) private(s1, s2, arr1, arr2, arr3) linear(val(argc) : argc)
  for (int i = 0; i < argc; ++i)
    // CHECK: call void @_Z3foov()
    foo();
// CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()

// CHECK: [[ARGCREF:%.+]] = load ptr, ptr [[ARGCREF_ADDR]],
// CHECK: region.entry() [ "DIR.OMP.SIMD"()
// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[S1_ADDR]],
// CHECK-SAME: ptr @_ZTS2S1.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S1.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[S2_ADDR]],
// CHECK-SAME: ptr @_ZTS2S2.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S2.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[ARR1_ADDR]],
// CHECK-SAME: ptr @_ZTS2S1.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S1.omp.destr

// CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[ARR2_ADDR]],
// CHECK_SAME: ptr @_ZTS2S2.omp.def_constr,
// CHECK-SAME: ptr @_ZTS2S2.omp.destr)

// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[ARR3_ADDR]]

// CHECK-SAME: "QUAL.OMP.LINEAR:BYREF.TYPED"(ptr [[ARGCREF_ADDR]], i32 0, i32 1, i32 1)

// CHECK-SAME: "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr [[ARGV_ADDR]], ptr null, i32 1, i32 1)
#pragma omp simd private(s1, s2, arr1, arr2, arr3) linear(uval(argcref)) linear(argv)
  for (int i = 0; i < argc; ++i)
    // CHECK: call void @_Z3foov()
    foo();
// CHECK: region.exit{{.*}}"DIR.OMP.END.SIMD"()

  long int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10;
// CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
// CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr [[N1_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.SUB:TYPED"(ptr [[N2_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.MAX:TYPED"(ptr [[N3_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.MIN:TYPED"(ptr [[N4_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.BAND:TYPED"(ptr [[N5_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.BOR:TYPED"(ptr [[N6_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.AND:TYPED"(ptr [[N7_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.OR:TYPED"(ptr [[N8_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.BXOR:TYPED"(ptr [[N9_ADDR]]
// CHECK-SAME: "QUAL.OMP.REDUCTION.MUL:TYPED"(ptr [[N10_ADDR]]
#pragma omp parallel reduction(+:n1) reduction(-:n2) reduction(max:n3) reduction (min:n4) reduction(&:n5) reduction (|:n6) reduction(&&:n7) reduction(||:n8) reduction(^:n9) reduction(*:n10)
  // CHECK: call void @_Z3foov()
  foo();
// CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"()
// CHECK: region.entry() [ "DIR.OMP.CRITICAL"()
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, ptr
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.CRITICAL"()
#pragma omp critical
  n2 = n1;
// CHECK: region.entry() [ "DIR.OMP.MASTER"()
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, ptr
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.MASTER"()
#pragma omp master
  n2 = n1;
// CHECK: region.entry() [ "DIR.OMP.SINGLE"()
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, ptr
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.SINGLE"()
#pragma omp single
  n2 = n1;
// CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
// CHECK-SAME: "QUAL.OMP.ORDERED"(i32 0)
// CHECK: region.entry() [ "DIR.OMP.ORDERED"()
// CHECK-NEXT: load i64, ptr
// CHECK-NEXT: store i64
// CHECK: region.exit{{.*}}"DIR.OMP.END.ORDERED"()
// CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
#pragma omp parallel for ordered
  for (int i = 0; i < 10; ++i) {
#pragma omp ordered
    n2 = n1;
  }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.STATIC"(i32 0)
#pragma omp parallel for schedule(static)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.STATIC"(i32 64)
#pragma omp parallel for schedule(static, 64)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 1)
#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 128)
#pragma omp parallel for schedule(dynamic, 128)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.GUIDED"(i32 1)
#pragma omp parallel for schedule(guided)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.GUIDED"(i32 256)
#pragma omp parallel for schedule(guided, 256)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.AUTO"(i32 0)
#pragma omp parallel for schedule(auto)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.RUNTIME"(i32 0)
#pragma omp parallel for schedule(runtime)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC"(i32 16)
#pragma omp parallel for schedule(monotonic:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC"(i32 16)
#pragma omp parallel for schedule(nonmonotonic:dynamic,16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC:SIMD"(i32 16)
#pragma omp parallel for schedule(simd:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC.SIMD"(i32 16)
#pragma omp parallel for schedule(simd,monotonic:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC.SIMD"(i32 16)
#pragma omp parallel for schedule(simd,nonmonotonic:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC.SIMD"(i32 16)
#pragma omp parallel for schedule(monotonic,simd:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
//CHECK-SAME: "QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC.SIMD"(i32 16)
#pragma omp parallel for schedule(nonmonotonic,simd:dynamic,16)
  for (int i = 0; i < 10; ++i) { }

  int N = 10;
  const int M = 10;
  long input1[N];
  long **input2 = 0;
  long result[M][M][N];
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET"()
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target map(to : input1[2:N], input2[0:N][:M]) map(from : result[2:][:M][0:])
#pragma omp parallel for
  for (int i = 0; i < N; i++)
    result[i][i][i] = input1[i] + input2[i][i];
  {
    int *a, *b;
    int z = 3, y = 9;
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET"()
// CHECK-SAME: "QUAL.OMP.DEVICE"(i32 4),
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TOFROM:SCALAR"()
// CHECK-SAME: "QUAL.OMP.NOWAIT"()
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
// CHECK: region.exit(token {{.*}} [ "DIR.OMP.END.TASK"() ]
    #pragma omp target device(4) is_device_ptr(a,b) \
                       defaultmap(tofrom:scalar) nowait
    {
      a = b;
    }
// CHECK: call void @_Z3foov()
  foo();
// CHECK: [[L0:%[0-9]+]] = load ptr, ptr %a{{.*}}
// CHECK: [[L1:%[0-9]+]] = load ptr, ptr %b{{.*}}
// CHECK: [[TARGD_TOKENVAL:%[0-9]+]] = call token{{.*}}region.entry()
// CHECK-SAME: "DIR.OMP.TARGET.DATA"()
// CHECK-SAME: "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(ptr %a, ptr %b)
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %z{{.*}}, ptr %z{{.*}}, i64 4, i64 3
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L0]], ptr [[L0]], i64 0, i64 64
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L1]], ptr [[L1]], i64 0, i64 64
// CHECK: region.exit(token [[TARGD_TOKENVAL]]) [ "DIR.OMP.END.TARGET.DATA"() ]
    #pragma omp target data map(tofrom:z) use_device_ptr(a,b)
    {
    }
// CHECK: [[TARGU_TOKENVAL:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET.UPDATE"(),
// CHECK-SAME "QUAL.OMP.FROMQUAL.OMP.MAP.FROM"(ptr %y{{.*}}, ptr %y{{.*}}, i64 4, i64 34
// CHECK-SAME "QUAL.OMP.FROMQUAL.OMP.MAP.TO"(ptr %z{{.*}}, ptr %z{{.*}}, i64 4, i64 34
// CHECK: region.exit(token [[TARGU_TOKENVAL]]) [ "DIR.OMP.END.TARGET.UPDATE"() ]
    #pragma omp target update to(z) from(y)
  }
  {
    int local_int = 1;
   bar(1);
// CHECK: call {{.*}}bar
// CHECK: [[TARG2_TOKENVAL:%[0-9]+]] = call token{{.*}}TARGET
// CHECK-SAME: FIRSTPRIVATE{{.*}}glob_int
// CHECK-SAME: FIRSTPRIVATE{{.*}}local_int
// CHECK: region.exit(token [[TARG2_TOKENVAL]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target is_device_ptr(glob_ptr)
    {
      glob_int = local_int + 1;
      glob_ptr++;
    }
    S4 s4;
// CHECK: [[TARG3_TOKENVAL:%[0-9]+]] = call token{{.*}}TARGET{{.*}}MAP.TOFROM{{.*}}ptr [[S4]]
// CHECK: getelementptr inbounds %struct.S4, ptr [[S4]]
// CHECK: region.exit(token [[TARG3_TOKENVAL]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target
    {
      s4.i = 1;
    }
// CHECK: [[TARG4_TOKENVAL:%[0-9]+]] = call token{{.*}}TARGET{{.*}}DEFAULTMAP.TOFROM:SCALAR{{.*}}MAP.TOFROM{{.*}}glob_int{{.*}}MAP.TOFROM{{.*}}local_int
// CHECK: region.exit(token [[TARG4_TOKENVAL]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target defaultmap(tofrom:scalar)
    {
      glob_int = local_int + 1;
    }
  }
// CHECK: [[TARGTE_TV:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET"()
// CHECK: [[TE_TV:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.NUM_TEAMS"(i32 16), "QUAL.OMP.THREAD_LIMIT"(i32 4)
// CHECK: [[DIST_TV:%[0-9]+]] = call token{{.*}}region.entry()
// CHECK-SAME: [ "DIR.OMP.DISTRIBUTE"()
// CHECK-SAME: "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 8)
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %i
  #pragma omp target
  #pragma omp teams num_teams(16) thread_limit(4)
  {
    #pragma omp distribute dist_schedule(static, 8)
    for (int i = 0; i < N; i++)
      foo();
  }
// CHECK: region.exit(token [[DIST_TV]]) [ "DIR.OMP.END.DISTRIBUTE"() ]
// CHECK: region.exit(token [[TE_TV]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TARGTE_TV]]) [ "DIR.OMP.END.TARGET"() ]

// CHECK: [[BARRIER_TOKENVAL:%[0-9]+]] = call token{{.*}}DIR.OMP.BARRIER
// CHECK: fence acq_rel
// CHECK: region.exit(token [[BARRIER_TOKENVAL]]) [ "DIR.OMP.END.BARRIER"
  #pragma omp barrier
// CHECK: [[FLUSH_TOKENVAL:%[0-9]+]] = call token{{.*}}DIR.OMP.FLUSH
// CHECK: region.exit(token [[FLUSH_TOKENVAL]]) [ "DIR.OMP.END.FLUSH"
  #pragma omp flush

  {
    int fli = 3, flj = 4;
// CHECK: [[FLUSH_TOKENVAL1:%[0-9]+]] = call token{{.*}}DIR.OMP.FLUSH{{.*}}QUAL.OMP.FLUSH{{.*}}fli{{.*}}flj
// CHECK: region.exit(token [[FLUSH_TOKENVAL1]]) [ "DIR.OMP.END.FLUSH"
    #pragma omp flush(fli,flj)
  }

// CHECK: [[SECT1ATV:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL
// CHECK: [[SECT1BTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTIONS{{.*}}NOWAIT{{.*}}PRIVATE{{.*}}sect1{{.*}}FIRSTPRIVATE{{.*}}sect2{{.*}}LASTPRIVATE{{.*}}sect3{{.*}}REDUCTION.ADD{{.*}}sect4
// CHECK: [[SECT1CTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK: {{call|invoke}}{{.*}}bari(i32 noundef 1)
// CHECK: region.exit(token [[SECT1CTV]]) [ "DIR.OMP.END.SECTION"
// CHECK: [[SECT1DTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK: {{call|invoke}}{{.*}}bari(i32 noundef 2)
// CHECK: region.exit(token [[SECT1DTV]]) [ "DIR.OMP.END.SECTION"
// CHECK: [[SECT1ETV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK: {{call|invoke}}{{.*}}bari(i32 noundef 3)
// CHECK: region.exit(token [[SECT1ETV]]) [ "DIR.OMP.END.SECTION"
// CHECK: region.exit(token [[SECT1BTV]]) [ "DIR.OMP.END.SECTIONS"
// CHECK: region.exit(token [[SECT1ATV]]) [ "DIR.OMP.END.PARALLEL"
  {
    int sect1=0,sect2=0,sect3=0,sect4=0;
    #pragma omp parallel
    {
      #pragma omp sections nowait private(sect1) firstprivate(sect2) \
                           lastprivate(sect3) reduction(+:sect4)
      {
        #pragma omp section
        {
          bar(1);
        }
        #pragma omp section
        {
          bar(2);
        }
        #pragma omp section
        {
          bar(3);
        }
      }
    }
  }
// CHECK: [[SECT2BTV:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL.SECTIONS{{.*}}PRIVATE{{.*}}sect1{{.*}}FIRSTPRIVATE{{.*}}sect2{{.*}}LASTPRIVATE{{.*}}sect3{{.*}}REDUCTION.ADD{{.*}}sect4
// CHECK: [[SECT2CTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK: {{call|invoke}}{{.*}}bari(i32 noundef 1)
// CHECK: region.exit(token [[SECT2CTV]]) [ "DIR.OMP.END.SECTION"
// CHECK: [[SECT2DTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK: {{call|invoke}}{{.*}}bari(i32 noundef 2)
// CHECK: region.exit(token [[SECT2DTV]]) [ "DIR.OMP.END.SECTION"
// CHECK: [[SECT2ETV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK: {{call|invoke}}{{.*}}bari(i32 noundef 3)
// CHECK: region.exit(token [[SECT2ETV]]) [ "DIR.OMP.END.SECTION"
// CHECK: region.exit(token [[SECT2BTV]]) [ "DIR.OMP.END.PARALLEL.SECTIONS"
  {
    int sect1=0,sect2=0,sect3=0,sect4=0;
    #pragma omp parallel sections private(sect1) firstprivate(sect2) \
                                  lastprivate(sect3) reduction(+:sect4)
    {
      #pragma omp section
      {
        bar(1);
      }
      #pragma omp section
      {
        bar(2);
      }
      #pragma omp section
      {
        bar(3);
      }
    }
  }
  #pragma omp parallel
  {
    // CHECK: DIR.OMP.CRITICAL
    // CHECK-SAME: "QUAL.OMP.NAME"([7 x i8] c"critfoo")
    #pragma omp critical(critfoo)
    n2 = n1;
    // CHECK: DIR.OMP.CRITICAL
    // CHECK-SAME: "QUAL.OMP.NAME"([10 x i8] c"critbarbaz")
    // CHECK-SAME: "QUAL.OMP.HINT"(i32 42)
    #pragma omp critical(critbarbaz) hint(42)
    n2 = n1;
  }
  return 0;
}

// CHECK: define internal noundef ptr @_ZTS2S1.omp.def_constr(ptr noundef %0)
// CHECK: alloca ptr,
// CHECK-NEXT: alloca ptr,
// CHECK-NEXT:  store ptr %{{.+}}, ptr %
// CHECK-NEXT:  load ptr, ptr %
// CHECK-NEXT:  ret ptr %
// CHECK-NEXT: }

// CHECK: define internal void @_ZTS2S1.omp.destr(ptr noundef %0)
// CHECK: alloca ptr,
// CHECK-NEXT: store ptr %{{.+}}, ptr %
// CHECK-NEXT: load ptr, ptr %
// CHECK-NEXT: call void @_ZN2S1D1Ev(ptr {{[^,]*}} %{{.+}})
// CHECK-NEXT: ret void
// CHECK-NEXT: }

// CHECK: define internal noundef ptr @_ZTS2S2.omp.def_constr(ptr noundef %0)
// CHECK: alloca ptr,
// CHECK-NEXT: store ptr %{{.+}}, ptr %
// CHECK-NEXT: load ptr, ptr %
// CHECK-NEXT: call void @_ZN2S2C1Ev(ptr {{[^,]*}} %{{.+}})
// CHECK-NEXT: ret ptr %
// CHECK-NEXT: }

// CHECK: define internal void @_ZTS2S2.omp.destr(ptr noundef %0)
// CHECK: alloca ptr,
// CHECK-NEXT: store ptr %{{.+}}, ptr %
// CHECK-NEXT: ret void
// CHECK-NEXT: }

// CHECK-NOT: define internal ptr @_ZTSA10_2S1.omp.def_constr(ptr %0)

// CHECK-NOT: define internal void @_ZTSA10_2S1.omp.destr(ptr %0)

// CHECK_NOT: define internal ptr @_ZTSA10_2S2.omp.def_constr(ptr %0)

// CHECK-NOT: define internal void @_ZTSA10_2S2.omp.destr(ptr %0)

#pragma omp declare target
int a[100];
#pragma omp end declare target

extern void modify_array_on_target();

//CHECK-LABEL: enter_exit_data
void enter_exit_data() {
  //CHECK: "DIR.OMP.TARGET.ENTER.DATA"
  //CHECK-SAME: QUAL.OMP.MAP.TO:ALWAYS
  //CHECK: "DIR.OMP.END.TARGET.ENTER.DATA"
  #pragma omp target enter data map(always,to: a[7:17])
  //CHECK: call{{.*}}modify_array_on_target
  modify_array_on_target();
  //CHECK: "DIR.OMP.TARGET.EXIT.DATA"
  //CHECK-SAME: QUAL.OMP.MAP.FROM:ALWAYS
  //CHECK: "DIR.OMP.END.TARGET.EXIT.DATA"
  #pragma omp target exit data map(always,from: a[9:13])
}

class S {
  public:
    int *y;
    void foo() {
  //CHECK: "DIR.OMP.TARGET.DATA"
  //CHECK-SAME: "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(ptr %y)
  //CHECK: "DIR.OMP.END.TARGET.DATA"
#pragma omp target data use_device_ptr(y)
         y++;
    }
};

void barfoo() {
    int a[10];
    S s; s.y = &a[0];
    a[2] = 111;
    s.foo();
}

#endif // HEADER
// end INTEL_COLLAB
