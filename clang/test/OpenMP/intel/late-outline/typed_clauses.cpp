// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp  \
// RUN:  -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -fopenmp-version=51 -emit-llvm %s -o - \
// RUN:  -fopenmp-typed-clauses | FileCheck %s
//
// expected-no-diagnostics

//CHECK: [[STR:%struct.Str.*]] = type { [4 x i8] }

int afoo;
#pragma omp threadprivate(afoo)

struct F { int i; F(); F(const F&); ~F(); };
void test()
{
  //CHECK: [[I:%i.*]] = alloca i32, align 4
  //CHECK: [[IP:%ip.*]] = alloca ptr, align 8
  //CHECK: [[IR:%ir.*]] = alloca ptr, align 8
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
  //CHECK: [[R:%r.*]] = alloca i32, align 4

  int i;
  int *ip;
  int &ir = i;
  F f;
  //CHECK: "DIR.OMP.PARALLEL"(),
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[IP]], ptr null, i32 1)
  //CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr [[F]], [[STRUCT_F]] zeroinitializer, i32 1
  //CHECK-SAME: "QUAL.OMP.PRIVATE:BYREF.TYPED"(ptr [[IR]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[Y]], i32 0, i32 1)
  #pragma omp parallel private(i) private(ip, f, ir)
  {
    int y = 1;
  }

  //CHECK: "DIR.OMP.PARALLEL.LOOP"(),
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[UNC_IV]], i32 0, ptr [[UNC_IV2]], i32 0)
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[UNC_LB]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[UNC_UB]], i32 0, ptr [[UNC_UB4]], i32 0)
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[UNC_LB3]], i32 0, i32 1)
  #pragma omp parallel for collapse(2)
  for(int z1=2; z1<16; ++z1)
  for(int z2=3; z2<16; ++z2) {
  }

  //CHECK: [[AFOO_ADDR:%.*]] = call ptr @llvm.threadlocal.address.p0(ptr @afoo)
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.COPYIN:TYPED"(ptr [[AFOO_ADDR]], i32 0, i32 1)
  #pragma omp parallel copyin(afoo)
  {
  }

  int b = 10;

  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[B]], i32 0, i32 1)
  #pragma omp parallel firstprivate(b)
  {
    //CHECK: "DIR.OMP.SINGLE"()
    //CHECK-SAME: "QUAL.OMP.COPYPRIVATE:TYPED"(ptr [[B]], i32 0, i32 1)
    #pragma omp single copyprivate(b)
    {
    }
  }
  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[B]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV]], i32 0)
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[UB]], i32 0)
  #pragma omp parallel for lastprivate(b)
  for (int z3=0; z3 < 10; ++z3) { }

  int r;
  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr [[R]], i32 0, i32 1)
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
  //CHECK: [[THIS_ADDR:%this.*]] = alloca ptr, align 8
  //CHECK: [[THIS:%this.*]] = load ptr, ptr [[THIS_ADDR]], align 8
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[THIS]], [[STR]] zeroinitializer, i32 1)
  #pragma omp target
  fld[2] = 1;
}

//CHECK: define {{.*}}test2
void test2()
{
  //CHECK: [[REF_TMP:%ref.tmp.*]] = alloca [[STR]], align 1

  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[REF_TMP]], [[STR]] zeroinitializer, i32 1)
  #pragma omp parallel
  {
    Str V0;
    Str V (V0 * 1);
  }
}

//CHECK: define {{.*}}test_fixed_array
void test_fixed_array()
{
  //CHECK: [[JARR:%Jarr.*]] = alloca [3 x [4 x i32]],
  int Jarr[3][4];
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[JARR]], i32 0, i64 12)
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
    //CHECK: [[N:%[0-9]+]] = load i64, ptr [[NADDR]], align 8
    //CHECK: [[M:%[0-9]+]] = load i64, ptr [[MADDR]], align 8
    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 [[N]], [[M]]
    //CHECK: [[VLA:%vla.*]] = alloca i32, i64 [[ELTS]],

    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 [[N]], [[M]]
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[VLA]], i32 0, i64 [[ELTS]])
    #pragma omp parallel private(vla)
    {}
  }

  {
    int vla1[3][n];
    //CHECK: [[N:%[0-9]+]] = load i64, ptr [[NADDR]], align 8
    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 3, [[N]]
    //CHECK: [[VLA1:%vla.*]] = alloca i32, i64 [[ELTS]],

    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 3, [[N]]
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[VLA1]], i32 0, i64 [[ELTS]])
    #pragma omp parallel private(vla1)
    {}
  }

  {
    int vla2[n][3];
    //CHECK: [[N:%[0-9]+]] = load i64, ptr [[NADDR]], align 8
    //CHECK: [[VLA2:%vla.*]] = alloca [3 x i32], i64 [[N]],

    //CHECK: [[ELTS:%[0-9]+]] = mul nuw i64 [[N]], 3
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[VLA2]], i32 0, i64 [[ELTS]])
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

//CHECK: [[PTR_BASE:%ptr_base.*]] = alloca ptr, align 8
//CHECK: [[PBREF:%pbref.*]] = alloca ptr, align 8

  // Typed array sections arguments are:
  // p# %vla, <type specifier>, i# %number_of_elements, i# %offset_in_elements

//CHECK: [[AI:%.*]] = getelementptr inbounds [[YT]], ptr [[Y_ARRAY]], i64 0, i64 7
//CHECK-NEXT: [[AI1:%.*]] = getelementptr inbounds [42 x [100 x i32]], ptr [[AI]], i64 0, i64 0
//CHECK-NEXT: [[ARRAYDECAY:%.*]] = getelementptr inbounds [100 x i32], ptr [[AI1]], i64 0, i64 0
//CHECK-NEXT: [[AI2:%.*]] = getelementptr inbounds i32, ptr [[ARRAYDECAY]], i64 0
//CHECK-NEXT: [[SEC_BASE_CAST:%.*]] = ptrtoint ptr [[Y_ARRAY]] to i64
//CHECK-NEXT: [[SEC_LOWER_CAST:%.*]] = ptrtoint ptr [[AI2]] to i64
//CHECK-NEXT: [[AI3:%.*]] = getelementptr inbounds [[YT]], ptr [[Y_ARRAY]], i64 0, i64 7
//CHECK-NEXT: [[AI4:%.*]] = getelementptr inbounds [42 x [100 x i32]], ptr [[AI3]], i64 0, i64 5
//CHECK-NEXT: [[ARRAYDECAY5:%.*]] = getelementptr inbounds [100 x i32], ptr [[AI4]], i64 0, i64 0
//CHECK-NEXT: [[AI6:%.*]] = getelementptr inbounds i32, ptr [[ARRAYDECAY5]], i64 99
//CHECK-NEXT: [[SEC_UPPER_CAST:%.*]] = ptrtoint ptr [[AI6]] to i64
//CHECK-NEXT: [[TMP0:%.*]] = sub i64 [[SEC_UPPER_CAST]], [[SEC_LOWER_CAST]]
//CHECK-NEXT: [[TMP1:%.*]] = sdiv exact i64 [[TMP0]], 4
//CHECK-NEXT: [[SEC_NUMBER_OF_ELEMENTS:%.*]] = add i64 [[TMP1]], 1
//CHECK-NEXT: [[TMP2:%.*]] = sub i64 [[SEC_LOWER_CAST]], [[SEC_BASE_CAST]]
//CHECK-NEXT: [[SEC_OFFSET_IN_ELEMENTS:%.*]] = sdiv exact i64 [[TMP2]], 4
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: ptr [[Y_ARRAY]], i32 0,
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

//CHECK: [[AI:%.*]] = getelementptr inbounds [[YT]], ptr [[Y_ARRAY]], i64 0, i64 6
//CHECK: [[AI1:%.*]] = getelementptr inbounds [42 x [100 x i32]], ptr [[AI]], i64 0, i64 21
//CHECK: [[AI2:%.*]] = getelementptr inbounds [100 x i32], ptr [[AI1]], i64 0, i64 4
//CHECK: [[SEC_BASE_CAST:%sec.base.cast.*]] = ptrtoint ptr [[Y_ARRAY]] to i64
//CHECK: [[SEC_LOWER_CAST:%.*]] = ptrtoint ptr [[AI2]] to i64
//CHECK: [[TMP0:%.*]] = sub i64 [[SEC_LOWER_CAST]], [[SEC_BASE_CAST]]
//CHECK: [[SEC_OFFSET_IN_ELEMENTS:%.*]] = sdiv exact i64 [[TMP0]], 4
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: ptr [[Y_ARRAY]], i32 0, i64 1,
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
//CHECK: [[TMP15:%.*]] = load i64, ptr [[N_ADDR]], align 8
//CHECK: [[TMP17:%.*]] = mul nuw i64 1000, [[TMP15]]
//CHECK: [[VLA:%vla.*]] = alloca i16, i64 [[TMP17]], align 16
//CHECK: [[TMP20:%.*]] = mul nuw i64 10, [[TMP15]]
//CHECK-NEXT: [[TMP21:%.*]] = mul nsw i64 50, [[TMP20]]
//CHECK-NEXT: [[AI24:%.*]] = getelementptr inbounds i16, ptr [[VLA]], i64 [[TMP21]]
//CHECK-NEXT: [[TMP22:%.*]] = mul nsw i64 5, [[TMP15]]
//CHECK-NEXT: [[AI25:%.*]] = getelementptr inbounds i16, ptr [[AI24]], i64 [[TMP22]]
//CHECK-NEXT: [[AI26:%.*]] = getelementptr inbounds i16, ptr [[AI25]], i64 0
//CHECK-NEXT: [[SEC_BASE_CAST23:%sec.base.cast.*]] = ptrtoint ptr [[VLA]] to i64
//CHECK-NEXT: [[SEC_LOWER_CAST27:%sec.lower.cast.*]] = ptrtoint ptr [[AI26]] to i64
//CHECK-NEXT: [[TMP23:%.*]] = load i64, ptr [[N_ADDR]], align 8
//CHECK-NEXT: [[LB_ADD_LEN:%.*]] = add nsw i64 -1, [[TMP23]]
//CHECK-NEXT: [[TMP24:%.*]] = mul nuw i64 10, [[TMP15]]
//CHECK-NEXT: [[TMP25:%.*]] = mul nsw i64 50, [[TMP24]]
//CHECK-NEXT: [[AI28:%.*]] = getelementptr inbounds i16, ptr [[VLA]], i64 [[TMP25]]
//CHECK-NEXT: [[TMP26:%.*]] = mul nsw i64 7, [[TMP15]]
//CHECK-NEXT: [[AI29:%.*]] = getelementptr inbounds i16, ptr [[AI28]], i64 [[TMP26]]
//CHECK-NEXT: [[AI30:%.*]] = getelementptr inbounds i16, ptr [[AI29]], i64 [[LB_ADD_LEN]]
//CHECK-NEXT: [[SEC_UPPER_CAST31:%sec.upper.cast.*]] = ptrtoint ptr [[AI30]] to i64
//CHECK-NEXT: [[TMP27:%.*]] = sub i64 [[SEC_UPPER_CAST31]], [[SEC_LOWER_CAST27]]
//CHECK-NEXT: [[TMP28:%.*]] = sdiv exact i64 [[TMP27]], 2
//CHECK-NEXT: [[SEC_NUMBER_OF_ELEMENTS32:%.*]] = add i64 [[TMP28]], 1
//CHECK-NEXT: [[TMP29:%.*]] = sub i64 [[SEC_LOWER_CAST27]], [[SEC_BASE_CAST23]]
//CHECK-NEXT: [[SEC_OFFSET_IN_ELEMENTS33:%.*]] = sdiv exact i64 [[TMP29]], 2
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: ptr [[VLA]], i16 0,
//CHECK-SAME: i64 [[SEC_NUMBER_OF_ELEMENTS32]], i64 [[SEC_OFFSET_IN_ELEMENTS33]])
  #pragma omp parallel for reduction(+:A_vla[50][5:3][0:n])
  for(int i = 0; i < 10; i++);
//CHECK: "DIR.OMP.END.PARALLEL.LOOP"()

  double B_vla[11][m][42];

//CHECK: [[TMP49:%.*]] = load i64, ptr [[M_ADDR]], align 8
//CHECK: [[TMP50:%.*]] = mul nuw i64 11, [[TMP49]]
//CHECK: [[VLA2:%vla.*]] = alloca [42 x double], i64 [[TMP50]],
//CHECK: [[TMP53:%.*]] = mul nsw i64 4, [[TMP49]]
//CHECK: [[AI:%.*]] = getelementptr inbounds [42 x double], ptr [[VLA2]], i64 [[TMP53]]
//CHECK: [[AI1:%.*]] = getelementptr inbounds [42 x double], ptr [[AI]], i64 3
//CHECK: [[TMP54:%.*]] = load i64, ptr [[N_ADDR]], align 8
//CHECK: [[AI2:%.*]] = getelementptr inbounds [42 x double], ptr [[AI1]], i64 0, i64 [[TMP54]]
//CHECK: [[SEC_BASE_CAST:%sec.base.cast.*]] = ptrtoint ptr [[VLA2]] to i64
//CHECK: [[SEC_LOWER_CAST:%sec.lower.cast.*]] = ptrtoint ptr [[AI2]] to i64
//CHECK: [[TMP55:%.*]] = sub i64 [[SEC_LOWER_CAST]], [[SEC_BASE_CAST]]
//CHECK: [[SEC_OFFSET_IN_ELEMENTS:%.*]] = sdiv exact i64 [[TMP55]], 8
//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: ptr [[VLA2]], double 0.000000e+00, i64 1,
//CHECK-SAME: i64 [[SEC_OFFSET_IN_ELEMENTS]])
  #pragma omp parallel for reduction(+:B_vla[4][3][n])
  for(int i = 0; i < 10; i++);

  int *ptr_base;
  int *(&pbref) = ptr_base;

//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
//CHECK-SAME: ptr [[PTR_BASE]], i32 0
  #pragma omp parallel for reduction(+:ptr_base[3:10])
  for(int i = 0; i < 10; i++);

//CHECK: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"
//CHECK-SAME: ptr [[PBREF]], i32 0
  #pragma omp parallel for reduction(+:pbref[3:10])
  for(int i = 0; i < 10; i++);
}

//CHECK: define {{.*}}foo_teams_thread_limit
void foo_teams_thread_limit()
{
  //CHECK: [[LOCAL:%local.*]] = alloca i32,
  //CHECK: [[TMP:%omp.clause.tmp.*]] = alloca i32,
  int local = 2;

  //CHECK: load i32, ptr [[LOCAL]]
  //CHECK-NEXT: add{{.*}}2
  //CHECK-NEXT: store{{.*}}[[TMP]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LOCAL]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[TMP]], i32 0, i32 1)
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]], i32 0)
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[TMP]], i32 0)
  #pragma omp target
  #pragma omp teams num_teams(local) thread_limit(local+2)
  { }
}

//CHECK: define {{.*}}foo_linear_clauses
void foo_linear_clauses()
{
  //CHECK: [[I:%i.*]] = alloca i32, align 4
  //CHECK: [[J:%j.*]] = alloca ptr, align 8
  //CHECK: [[X:%x.*]] = alloca ptr, align 8
  //CHECK: [[XP:%xp.*]] = alloca ptr, align 8
  //CHECK: [[Y:%y.*]] = alloca ptr, align 8
  //CHECK: [[CP:%cp.*]] = alloca ptr, align 8
  //CHECK: [[Z6:%z6.*]] = alloca i32, align 4

  int i;
  int &j = i;
  int *x;
  int **xp = &x;
  int *(&y) = x;
  char *cp;

  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.LINEAR:TYPED"(ptr [[I]], i32 0, i32 1, i32 2)
  //CHECK-SAME: "QUAL.OMP.LINEAR:BYREF.TYPED"(ptr [[J]], i32 0, i32 1, i32 1)
  //CHECK-SAME: "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr [[X]], i32 0, i32 1, i32 2)
  //CHECK-SAME: "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr [[XP]], ptr null, i32 1, i32 1)
  //CHECK-SAME: "QUAL.OMP.LINEAR:BYREF.PTR_TO_PTR.TYPED"(ptr [[Y]], i32 0, i32 1, i32 1)
  //CHECK-SAME: "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr [[CP]], i8 0, i32 1, i32 3)
  //CHECK: "DIR.OMP.SIMD"()
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[Z6]], i32 0, i32 1, i32 1)
  #pragma omp parallel for simd linear(i:2) linear(j) linear(x:2) linear(xp) \
                                linear(y) linear(cp:3)
  for (int z6=0; z6 < 10; ++z6) { }

}

//CHECK: define {{.*}}foo_shared_clauses
void foo_shared_clauses(int n)
{
  //CHECK: [[Z:%z.*]] = alloca i32, align 4
  //CHECK: [[ZP:%zp.*]] = alloca ptr, align 8
  //CHECK: [[ZR:%zr.*]] = alloca ptr, align 8
  //CHECK: [[ZPR:%zpr.*]] = alloca ptr, align 8
  //CHECK: [[VLATMP:%omp.vla.tmp.*]] = alloca i64, align 8
  int z = 3;
  int *zp = &z;
  int &zr = z;
  int *(&zpr) = zp;

  // Explicit shared
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[Z]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[ZP]], ptr null, i32 1)
  //CHECK-SAME: "QUAL.OMP.SHARED:BYREF.TYPED"(ptr [[ZR]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.SHARED:BYREF.TYPED"(ptr [[ZPR]], ptr null, i32 1)
  #pragma omp parallel shared(z,zp,zr,zpr)
  {
    int i = z + *zp + zr + *zpr;
  }

  // Implicit shared, normal user variables
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[Z]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[ZP]], ptr null, i32 1)
  //CHECK-SAME: "QUAL.OMP.SHARED:BYREF.TYPED"(ptr [[ZR]], i32 0, i32 1)
  //CHECK-SAME: "QUAL.OMP.SHARED:BYREF.TYPED"(ptr [[ZPR]], ptr null, i32 1)
  #pragma omp parallel
  {
    int i = z + *zp + zr + *zpr;
  }

  // Implicit shared codegen temps
  {
    int vla[n];
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[VLATMP]], i64 0, i32 1)
    #pragma omp parallel private(vla)
    {}
  }

  // Implicit VLA.
  {
    int implicit_shared_vla[n];
    //CHECK: [[IVLA:%vla.*]] = alloca i32,
    //CHECK: store i64 [[SZ:%[0-9]+]], ptr %omp.vla.tmp{{[[0-9]*}}
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[IVLA]], i32 0, i64 [[SZ]]
    #pragma omp parallel
    {
      implicit_shared_vla[4] = 1;
    }
  }

  // Explicit VLA can be typed.
  {
    int explicit_shared_vla[n];
    //CHECK: [[EVLA:%vla.*]] = alloca i32,
    //CHECK: store i64 [[SZ:%[0-9]+]], ptr %omp.vla.tmp{{[[0-9]*}}
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[EVLA]], i32 0, i64 [[SZ]]
    #pragma omp parallel shared(explicit_shared_vla)
    {
      explicit_shared_vla[4] = 1;
    }
  }

  // An implicit fixed array is okay.
  {
    int implicit_shared_fixed[42];
    //CHECK: "DIR.OMP.PARALLEL"()
    //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %implicit_shared_fixed{{[[0-9]*}}, i32 0, i64 42
    #pragma omp parallel shared(implicit_shared_fixed)
    {
      implicit_shared_fixed[4] = 1;
    }
  }
}
// end INTEL_COLLAB
