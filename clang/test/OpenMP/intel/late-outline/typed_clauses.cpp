// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp  \
// RUN:  -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -fopenmp-version=51 -emit-llvm %s -o - \
// RUN:  -fopenmp-typed-clauses | FileCheck %s
//
// expected-no-diagnostics

int afoo;
#pragma omp threadprivate(afoo)

struct F { int i; F(); F(const F&); ~F(); };
void test()
{
  //CHECK: [[I:%i.*]] = alloca i32, align 4
  //CHECK: [[IP:%ip.*]] = alloca i32*, align 8
  //CHECK: [[IR:%ir.*]] = alloca i32*, align 8
  //CHECK: [[F:%f.*]] = alloca [[STRUCT_F:%.*]], align 4
  //CHECK: [[Y:%y.*]] = alloca i32, align 4
  //CHECK: [[UNC_IV:%.omp.uncollapsed.iv.*]] = alloca i32, align 4
  //CHECK: [[UNC_IV2:%.omp.uncollapsed.iv.*]] = alloca i32, align 4
  //CHECK: [[UNC_LB:%.omp.uncollapsed.lb.*]] = alloca i32, align 4
  //CHECK: [[UNC_UB:%.omp.uncollapsed.ub.*]] = alloca i32, align 4
  //CHECK: [[UNC_LB3:%.omp.uncollapsed.lb.*]] = alloca i32, align 4
  //CHECK: [[UNC_UB4:%.omp.uncollapsed.ub.*]] = alloca i32, align 4
  //CHECK: [[B:%b.*]] = alloca i32, align 4
  //CHECK: [[IV:%.omp.iv.*]] = alloca i32, align 4
  //CHECK: [[UB:%.omp.ub.*]] = alloca i32, align 4
  //CHECK: [[Z4:%z4.*]] = alloca i32, align 4
  //CHECK: [[R:%r.*]] = alloca i32, align 4

  int i;
  int *ip;
  int &ir = i;
  F f;
  //CHECK: "DIR.OMP.PARALLEL"(),
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(i32* [[I]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(i32** [[IP]], i32* null, i32 1)
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(%struct.F* [[F]], [[STRUCT_F]] zeroinitializer, i32 1
  //CHECK-SAME: "QUAL.OMP.PRIVATE:BYREF.TYPED"(i32** [[IR]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(i32* [[Y]], i32 0, i32 1)
  #pragma omp parallel private(i) private(ip, f, ir)
  {
    int y = 1;
  }

  //CHECK: "DIR.OMP.PARALLEL.LOOP"(),
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* [[UNC_IV]], i32 0, i32* [[UNC_IV2]], i32 0)
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* [[UNC_LB]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* [[UNC_UB]], i32 0, i32* [[UNC_UB4]], i32 0)
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* [[UNC_LB3]], i32 0, i32 1)
  #pragma omp parallel for collapse(2)
  for(int z1=2; z1<16; ++z1)
  for(int z2=3; z2<16; ++z2) {
  }

  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.COPYIN:TYPED"(i32* @afoo, i32 0, i32 1)
  #pragma omp parallel copyin(afoo)
  {
  }

  int b = 10;

  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* [[B]], i32 0, i32 1)
  #pragma omp parallel firstprivate(b)
  {
    //CHECK: "DIR.OMP.SINGLE"()
    //CHECK-SAME: "QUAL.OMP.COPYPRIVATE:TYPED"(i32* [[B]], i32 0, i32 1)
    #pragma omp single copyprivate(b)
    {
    }
  }
  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(i32* [[B]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* [[IV]], i32 0)
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* [[UB]], i32 0)
  #pragma omp parallel for lastprivate(b)
  for (int z3=0; z3 < 10; ++z3) { }

  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.LINEAR:TYPED"(i32* [[B]], i32 0, i32 1, i32 2)
  //CHECK: "DIR.OMP.SIMD"()
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(i32* [[Z4]], i32 0, i32 1, i32 1)
  #pragma omp parallel for simd linear(b:2)
  for (int z4=0; z4 < 10; ++z4) { }

  int r;
  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* [[R]], i32 0, i32 1)
  #pragma omp parallel for reduction(+:r)
  for (int z5=0; z5 < 10; ++z5) { }
}

// Check for compiler-generated variable clauses
struct Str {
    char fld[4];
    Str();
    Str(int a);
    friend Str operator*(const Str&, const Str&);
    void member();
};

//CHECK: define {{.*}}member
void Str::member()
{
  //CHECK: [[THIS_ADDR:%this.*]] = alloca [[STR:%.*]]*, align 8
  //CHECK: [[THIS:%this.*]] = load [[STR]]*, [[STR]]** [[THIS_ADDR]], align 8
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"([[STR]]* [[THIS]], [[STR]] zeroinitializer, i32 1)
  #pragma omp target
  fld[2] = 1;
}

//CHECK: define {{.*}}test2
void test2()
{
  //CHECK: [[REF_TMP:%ref.tmp.*]] = alloca [[STR:%.*]], align 1
  //CHECK: [[FOOV:%foov.*]] = alloca [10 x i32], align 16
  //CHECK: [[D:%d.*]] = alloca double, align 8
  //CHECK: [[TP1:%.tmp.prefetch.*]] = alloca [10 x i32]*, align 8
  //CHECK: [[TP2:%.tmp.prefetch.*]] = alloca double*, align 8

  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"([[STR]]* [[REF_TMP]], [[STR]] zeroinitializer, i32 1)
  #pragma omp parallel
  {
    Str V0;
    Str V (V0 * 1);
  }

  int foov[10];
  double d;
  #pragma omp prefetch data(&foov:1:5) data(&d:1:1)
  //CHECK: "DIR.OMP.PREFETCH"()
  //CHECK-SAME: "QUAL.OMP.DATA:TYPED"([10 x i32]** [[TP1]], [10 x i32]* null, i32 1, i64 5)
  //CHECK-SAME: "QUAL.OMP.DATA:TYPED"(double** [[TP2]], double* null, i32 1, i64 1) ]
}
// end INTEL_COLLAB
