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

//CHECK: define {{.*}}test_fixed_array
void test_fixed_array()
{
  //CHECK: [[JARR:%Jarr.*]] = alloca [3 x [4 x i32]],
  int Jarr[3][4];
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK: "QUAL.OMP.PRIVATE:TYPED"([3 x [4 x i32]]* [[JARR]], i32 0, i64 12)
  #pragma omp parallel private(Jarr)
  {}
}

//CHECK: define {{.*}}test_variable_length_arrays
void test_variable_length_arrays(unsigned long n, unsigned long m)
{
  //CHECK: [[NADDR:%.*]] = alloca i64, align 8
  //CHECK-NEXT: [[MADDR:%.*]] = alloca i64, align 8
  {
    int vla[n][m];
    //CHECK: [[N:%[0-9]+]] = load i64, i64* [[NADDR]], align 8
    //CHECK: [[M:%[0-9]+]] = load i64, i64* [[MADDR]], align 8
    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 [[N]], [[M]]
    //CHECK: [[VLA:%vla.*]] = alloca i32, i64 [[ELTS]],

    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 [[N]], [[M]]
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK: "QUAL.OMP.PRIVATE:TYPED"(i32* [[VLA]], i32 0, i64 [[ELTS]])
    #pragma omp parallel private(vla)
    {}
  }

  {
    int vla1[3][n];
    //CHECK: [[N:%[0-9]+]] = load i64, i64* [[NADDR]], align 8
    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 3, [[N]]
    //CHECK: [[VLA1:%vla.*]] = alloca i32, i64 [[ELTS]],

    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 3, [[N]]
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK: "QUAL.OMP.PRIVATE:TYPED"(i32* [[VLA1]], i32 0, i64 [[ELTS]])
    #pragma omp parallel private(vla1)
    {}
  }

  {
    int vla2[n][3];
    //CHECK: [[N:%[0-9]+]] = load i64, i64* [[NADDR]], align 8
    //CHECK: [[VLA2:%vla.*]] = alloca [3 x i32], i64 [[N]],

    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 [[N]], 3
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK: "QUAL.OMP.PRIVATE:TYPED"([3 x i32]* [[VLA2]], i32 0, i64 [[ELTS]])
    #pragma omp parallel private(vla2)
    {}
  }
}

//CHECK: define {{.*}}test_array_sections
void test_array_sections(unsigned long n, unsigned long m) {
//CHECK: [[N_ADDR:%.*]] = alloca i64, align 8
//CHECK-NEXT: [[M_ADDR:%.*]] = alloca i64, align 8
//CHECK: [[Y_ARRAY:%y_Array.*]] = alloca [[YT:\[20 x \[42 x \[100 x i32\]\]\]]],
  int y_Array[20][42][100];

  // Typed array sections arguments are:
  // p# %vla, <type specifier>, i# %number_of_elements, i# %offset_in_elements

//CHECK:    [[SEC_BASE_CAST:%.*]] = ptrtoint [[YT]]* [[Y_ARRAY]] to i64
//CHECK-NEXT: [[AI:%.*]] = getelementptr inbounds [[YT]], [[YT]]* [[Y_ARRAY]], i64 0, i64 7
//CHECK-NEXT: [[AI1:%.*]] = getelementptr inbounds [42 x [100 x i32]], [42 x [100 x i32]]* [[AI]], i64 0, i64 0
//CHECK-NEXT: [[ARRAYDECAY:%.*]] = getelementptr inbounds [100 x i32], [100 x i32]* [[AI1]], i64 0, i64 0
//CHECK-NEXT: [[AI2:%.*]] = getelementptr inbounds i32, i32* [[ARRAYDECAY]], i64 0
//CHECK-NEXT: [[SEC_LOWER_CAST:%.*]] = ptrtoint i32* [[AI2]] to i64
//CHECK-NEXT: [[AI3:%.*]] = getelementptr inbounds [[YT]], [[YT]]* [[Y_ARRAY]], i64 0, i64 7
//CHECK-NEXT: [[AI4:%.*]] = getelementptr inbounds [42 x [100 x i32]], [42 x [100 x i32]]* [[AI3]], i64 0, i64 5
//CHECK-NEXT: [[ARRAYDECAY5:%.*]] = getelementptr inbounds [100 x i32], [100 x i32]* [[AI4]], i64 0, i64 0
//CHECK-NEXT: [[AI6:%.*]] = getelementptr inbounds i32, i32* [[ARRAYDECAY5]], i64 99
//CHECK-NEXT: [[SEC_UPPER_CAST:%.*]] = ptrtoint i32* [[AI6]] to i64
//CHECK-NEXT: [[TMP0:%.*]] = sub i64 [[SEC_UPPER_CAST]], [[SEC_LOWER_CAST]]
//CHECK-NEXT: [[TMP1:%.*]] = sdiv exact i64 [[TMP0]], 4
//CHECK-NEXT: [[SEC_NUMBER_OF_ELEMENTS:%.*]] = add i64 [[TMP1]], 1
//CHECK-NEXT: [[TMP2:%.*]] = sub i64 [[SEC_LOWER_CAST]], [[SEC_BASE_CAST]]
//CHECK-NEXT: [[SEC_OFFSET_IN_ELEMENTS:%.*]] = sdiv exact i64 [[TMP2]], 4
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: [[YT]]* [[Y_ARRAY]], i32 0,
//CHECK-SAME: i64 [[SEC_NUMBER_OF_ELEMENTS]], i64 [[SEC_OFFSET_IN_ELEMENTS]])
  #pragma omp parallel for reduction(+:y_Array[7][:6][:]) // 7 * 42 * 100 + 0 * 100+0
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 5; k++) {
        y_Array[1][2][k] += 1;
      }
    }
  }
//CHECK: "DIR.OMP.END.PARALLEL.LOOP"()

//CHECK: [[SEC_BASE_CAST:%sec.base.cast.*]] = ptrtoint [[YT]]* [[Y_ARRAY]] to i64
//CHECK: [[AI:%.*]] = getelementptr inbounds [[YT]], [[YT]]* [[Y_ARRAY]], i64 0, i64 6
//CHECK: [[AI1:%.*]] = getelementptr inbounds [42 x [100 x i32]], [42 x [100 x i32]]* [[AI]], i64 0, i64 21
//CHECK: [[AI2:%.*]] = getelementptr inbounds [100 x i32], [100 x i32]* [[AI1]], i64 0, i64 4
//CHECK: [[SEC_LOWER_CAST:%.*]] = ptrtoint i32* [[AI2]] to i64
//CHECK: [[TMP0:%.*]] = sub i64 [[SEC_LOWER_CAST]], [[SEC_BASE_CAST]]
//CHECK: [[SEC_OFFSET_IN_ELEMENTS:%.*]] = sdiv exact i64 [[TMP0]], 4
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: [[YT]]* [[Y_ARRAY]], i32 0, i64 1,
//CHECK-SAME: i64 [[SEC_OFFSET_IN_ELEMENTS]])
#pragma omp parallel for reduction(+:y_Array[6][21][4])
for (int i = 0; i < 3; i++) {
  for (int j = 0; j < 4; j++) {
    for (int k = 0; k < 5; k++) {
      y_Array[6][21][4] += 1;
    }
  }
}
//CHECK: "DIR.OMP.END.PARALLEL.LOOP"()

  short A_vla[100][10][n];
//CHECK: [[TMP15:%.*]] = load i64, i64* [[N_ADDR]], align 8
//CHECK: [[TMP17:%.*]] = mul nuw i64 1000, [[TMP15]]
//CHECK: [[VLA:%vla.*]] = alloca i16, i64 [[TMP17]], align 16
//CHECK: [[SEC_BASE_CAST23:%sec.base.cast.*]] = ptrtoint i16* [[VLA]] to i64
//CHECK-NEXT: [[TMP20:%.*]] = mul nuw i64 10, [[TMP15]]
//CHECK-NEXT: [[TMP21:%.*]] = mul nsw i64 50, [[TMP20]]
//CHECK-NEXT: [[AI24:%.*]] = getelementptr inbounds i16, i16* [[VLA]], i64 [[TMP21]]
//CHECK-NEXT: [[TMP22:%.*]] = mul nsw i64 5, [[TMP15]]
//CHECK-NEXT: [[AI25:%.*]] = getelementptr inbounds i16, i16* [[AI24]], i64 [[TMP22]]
//CHECK-NEXT: [[AI26:%.*]] = getelementptr inbounds i16, i16* [[AI25]], i64 0
//CHECK-NEXT: [[SEC_LOWER_CAST27:%sec.lower.cast.*]] = ptrtoint i16* [[AI26]] to i64
//CHECK-NEXT: [[TMP23:%.*]] = load i64, i64* [[N_ADDR]], align 8
//CHECK-NEXT: [[LB_ADD_LEN:%.*]] = add nsw i64 -1, [[TMP23]]
//CHECK-NEXT: [[TMP24:%.*]] = mul nuw i64 10, [[TMP15]]
//CHECK-NEXT: [[TMP25:%.*]] = mul nsw i64 50, [[TMP24]]
//CHECK-NEXT: [[AI28:%.*]] = getelementptr inbounds i16, i16* [[VLA]], i64 [[TMP25]]
//CHECK-NEXT: [[TMP26:%.*]] = mul nsw i64 7, [[TMP15]]
//CHECK-NEXT: [[AI29:%.*]] = getelementptr inbounds i16, i16* [[AI28]], i64 [[TMP26]]
//CHECK-NEXT: [[AI30:%.*]] = getelementptr inbounds i16, i16* [[AI29]], i64 [[LB_ADD_LEN]]
//CHECK-NEXT: [[SEC_UPPER_CAST31:%sec.upper.cast.*]] = ptrtoint i16* [[AI30]] to i64
//CHECK-NEXT: [[TMP27:%.*]] = sub i64 [[SEC_UPPER_CAST31]], [[SEC_LOWER_CAST27]]
//CHECK-NEXT: [[TMP28:%.*]] = sdiv exact i64 [[TMP27]], 2
//CHECK-NEXT: [[SEC_NUMBER_OF_ELEMENTS32:%.*]] = add i64 [[TMP28]], 1
//CHECK-NEXT: [[TMP29:%.*]] = sub i64 [[SEC_LOWER_CAST27]], [[SEC_BASE_CAST23]]
//CHECK-NEXT: [[SEC_OFFSET_IN_ELEMENTS33:%.*]] = sdiv exact i64 [[TMP29]], 2
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: i16* [[VLA]], i16 0,
//CHECK-SAME: i64 [[SEC_NUMBER_OF_ELEMENTS32]], i64 [[SEC_OFFSET_IN_ELEMENTS33]])
  #pragma omp parallel for reduction(+:A_vla[50][5:3][0:n])
  for(int i = 0; i < 10; i++);
//CHECK: "DIR.OMP.END.PARALLEL.LOOP"()

  double B_vla[11][m][42];

//CHECK: [[TMP49:%.*]] = load i64, i64* [[M_ADDR]], align 8
//CHECK: [[TMP50:%.*]] = mul nuw i64 11, [[TMP49]]
//CHECK: [[VLA2:%vla.*]] = alloca [42 x double], i64 [[TMP50]],
//CHECK: [[SEC_BASE_CAST:%sec.base.cast.*]] = ptrtoint [42 x double]* [[VLA2]] to i64
//CHECK: [[TMP53:%.*]] = mul nsw i64 4, [[TMP49]]
//CHECK: [[AI:%.*]] = getelementptr inbounds [42 x double], [42 x double]* [[VLA2]], i64 [[TMP53]]
//CHECK: [[AI1:%.*]] = getelementptr inbounds [42 x double], [42 x double]* [[AI]], i64 3
//CHECK: [[TMP54:%.*]] = load i64, i64* [[N_ADDR]], align 8
//CHECK: [[AI2:%.*]] = getelementptr inbounds [42 x double], [42 x double]* [[AI1]], i64 0, i64 [[TMP54]]
//CHECK: [[SEC_LOWER_CAST:%sec.lower.cast.*]] = ptrtoint double* [[AI2]] to i64
//CHECK: [[TMP55:%.*]] = sub i64 [[SEC_LOWER_CAST]], [[SEC_BASE_CAST]]
//CHECK: [[SEC_OFFSET_IN_ELEMENTS:%.*]] = sdiv exact i64 [[TMP55]], 8
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: [42 x double]* [[VLA2]], double 0.000000e+00, i64 1,
//CHECK-SAME: i64 [[SEC_OFFSET_IN_ELEMENTS]])
  #pragma omp parallel for reduction(+:B_vla[4][3][n])
  for(int i = 0; i < 10; i++);
}

//CHECK: define {{.*}}foo_teams_thread_limit
void foo_teams_thread_limit()
{
  //CHECK: [[LOCAL:%local.*]] = alloca i32,
  //CHECK: [[TMP:%omp.clause.tmp.*]] = alloca i32,
  int local = 2;

  //CHECK: load i32, i32* [[LOCAL]]
  //CHECK-NEXT: add{{.*}}2
  //CHECK-NEXT: store{{.*}}[[TMP]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* [[LOCAL]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* [[TMP]], i32 0, i32 1)
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(i32* [[LOCAL]], i32 0)
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(i32* [[TMP]], i32 0)
  #pragma omp target
  #pragma omp teams num_teams(local) thread_limit(local+2)
  { }
}

// end INTEL_COLLAB
