// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: implicit_maps_double_complex{{.*}}(
void implicit_maps_double_complex (int a){
  double _Complex dc = (double)a;

// CHECK: [[A_A:%a.addr]] = alloca i32,
// CHECK: [[DC:%dc]] = alloca { double, double },
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.ALLOC:SCALAR"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[DC]], ptr [[DC]], i64 16, i64 544
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(alloc:scalar)
  {
   dc *= dc;
  }
}

// CHECK-LABEL: implicit_maps_double_complex_1{{.*}}(
void implicit_maps_double_complex_1 (int a){
  double _Complex dc = (double)a;

// CHECK: [[A_A:%a.addr]] = alloca i32,
// CHECK: [[DC:%dc]] = alloca { double, double },
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TO:SCALAR"()
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[DC]], ptr [[DC]], i64 16, i64 545
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(to:scalar)
  {
   dc *= dc;
  }
}


// CHECK-LABEL: implicit_maps_double_complex_2{{.*}}(
void implicit_maps_double_complex_2 (int a){
  double _Complex dc = (double)a;

// CHECK: [[A_A:%a.addr]] = alloca i32,
// CHECK: [[DC:%dc]] = alloca { double, double },
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.FROM:SCALAR"()
// CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr [[DC]], ptr [[DC]], i64 16, i64 546
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(from:scalar)
  {
   dc *= dc;
  }
}


// CHECK-LABEL: implicit_maps_double{{.*}}(
void implicit_maps_double (int a){
  double d = (double)a;
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.FIRSTPRIVATE:SCALAR"()
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %d
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(firstprivate:scalar)
  {
    d += 1.0;
  }
}

// CHECK-LABEL: implicit_maps_double_1{{.*}}(
void implicit_maps_double_1 (int a){
  double d = (double)a;
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.DEFAULT:SCALAR"()
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %d
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(default: scalar)
  {
    d += 1.0;
  }
}

// CHECK-LABEL: implicit_maps_double_2{{.*}}(
void implicit_maps_double_2 (int a){
  double d = (double)a;
// CHECK: [[D:%d]] = alloca double
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TO:SCALAR"()
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[D]], ptr [[D]], i64 8, i64 545
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(to: scalar)
  {
    d += 1.0;
  }
}

// CHECK-LABEL: implicit_maps_array{{.*}}(
void implicit_maps_array (int a){
  double darr[2] = {(double)a, (double)a};
// CHECK: [[DA:%darr]] = alloca [2 x double]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.ALLOC:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[DA]], ptr [[DA]], i64 16, i64 544
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(alloc: aggregate)
  {
    darr[0] += 1.0;
    darr[1] += 1.0;
  }
}

// CHECK-LABEL: implicit_maps_array_1{{.*}}(
void implicit_maps_array_1 (int a){
  double darr[2] = {(double)a, (double)a};
// CHECK: [[DA:%darr]] = alloca [2 x double]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TO:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[DA]], ptr [[DA]], i64 16, i64 545
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(to: aggregate)
  {
    darr[0] += 1.0;
    darr[1] += 1.0;
  }
}


// CHECK-LABEL: implicit_maps_array_2{{.*}}(
void implicit_maps_array_2 (int a){
  double darr[2] = {(double)a, (double)a};
// CHECK: [[DA:%darr]] = alloca [2 x double]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.FROM:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr [[DA]], ptr [[DA]], i64 16, i64 546
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(from: aggregate)
  {
    darr[0] += 1.0;
    darr[1] += 1.0;
  }
}

// CHECK-LABEL: implicit_maps_array_3{{.*}}(
void implicit_maps_array_3 (int a){
  double darr[2] = {(double)a, (double)a};
// CHECK: [[DA:%darr]] = alloca [2 x double]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TOFROM:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[DA]], ptr [[DA]], i64 16, i64 547
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(tofrom: aggregate)
  {
    darr[0] += 1.0;
    darr[1] += 1.0;
  }
}


// CHECK-LABEL: zero_size_section_and_private_maps{{.*}}(
void zero_size_section_and_private_maps (int ii){
  int pvtArr[10];
// CHECK: [[PA:%pvtArr]] = alloca [10 x i32]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.FIRSTPRIVATE:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[PA]], ptr [[PA]], i64 40, i64 673
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(firstprivate:aggregate)
  {
    pvtArr[5]++;
  }
}

// CHECK-LABEL: explicit_maps_single{{.*}}(
void explicit_maps_single (){
  int *pa;

// CHECK: [[PA:%pa]] = alloca  ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.ALLOC:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[PA]], ptr [[PA]], i64 8, i64 544
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(alloc: pointer)
  {
    pa[50]++;
  }
}


// CHECK-LABEL: explicit_maps_single_1{{.*}}(
void explicit_maps_single_1 (){
  int *pa;

// CHECK: [[PA:%pa]] = alloca  ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TO:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[PA]], ptr [[PA]], i64 8, i64 545
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(to: pointer)
  {
    pa[50]++;
  }
}


// CHECK-LABEL: explicit_maps_single_2{{.*}}(
void explicit_maps_single_2 (){
  int *pa;

// CHECK: [[PA:%pa]] = alloca  ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.FROM:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr [[PA]], ptr [[PA]], i64 8, i64 546
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(from: pointer)
  {
    pa[50]++;
  }
}

// CHECK-LABEL: explicit_maps_single_3{{.*}} {
void explicit_maps_single_3 (){
  int *pa;

// CHECK: [[PA:%pa]] = alloca  ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TOFROM:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[PA]], ptr [[PA]], i64 8, i64 547
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(tofrom: pointer)
  {
    pa[50]++;
  }
}


// CHECK-LABEL: explicit_maps_single_4{{.*}}(
void explicit_maps_single_4 (){
  int *pa;

// CHECK: [[PA:%pa]] = alloca  ptr,
// CHECK: [[PA_MAP:%pa.map.ptr.tmp]] = alloca ptr,
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[PA]],
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.FIRSTPRIVATE:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L]], ptr [[L]], i64 0, i64 544
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(firstprivate: pointer)
  {
    pa[50]++;
  }
}

// CHECK-LABEL: implicit_maps_variable_length_array{{.*}}(
void implicit_maps_variable_length_array (int a){
  double vla[2][a];
// CHECK: [[A_ADDR:%a.addr]] = alloca i32,
// CHECK: [[L:%[0-9]+]] =  load i32, ptr [[A_ADDR]],
// CHECK: [[VLA:%vla]] = alloca double,
// CHECK: [[L4:%[0-9]+]] = mul nuw i64 2,
// CHECK: [[L5:%[0-9]+]] = mul nuw i64 [[L4]],
// CHECK: [[ARI:%arrayidx]] = getelementptr inbounds double, ptr [[VLA]],
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.ALLOC:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM:VARLEN"(ptr [[VLA]], ptr [[ARI]], i64 [[L5]], i64 544
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(alloc: aggregate)
  {
    vla[1][3] += 1.0;
  }
}


class SSS {
public:
  int a;
  double b;
};

// CHECK-LABEL: implicit_maps_struct{{.*}}(
void implicit_maps_struct (int a){
  SSS s = {a, (double)a};
// CHECK: [[S:%s]] =  alloca %class.SSS,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.ALLOC:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[S]], ptr [[S]], i64 16, i64 544
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(alloc: aggregate)
  {
    s.a += 1;
    s.b += 1.0;
  }
}


// CHECK-LABEL: implicit_maps_struct_1{{.*}}(
void implicit_maps_struct_1 (int a){
  SSS s = {a, (double)a};

// CHECK: [[S:%s]] =  alloca %class.SSS,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TO:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[S]], ptr [[S]], i64 16, i64 545
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(to: aggregate)
  {
    s.a += 1;
    s.b += 1.0;
  }
}


// CHECK-LABEL: implicit_maps_struct_2{{.*}}(
void implicit_maps_struct_2 (int a){
  SSS s = {a, (double)a};

// CHECK: [[S:%s]] =  alloca %class.SSS,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.FROM:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr [[S]], ptr [[S]], i64 16, i64 546
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(from: aggregate)
  {
    s.a += 1;
    s.b += 1.0;
  }
}


// CHECK-LABEL: implicit_maps_struct_3{{.*}}(
void implicit_maps_struct_3 (int a){
  SSS s = {a, (double)a};

// CHECK: [[S:%s]] =  alloca %class.SSS,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.TOFROM:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[S]], ptr [[S]], i64 16, i64 547
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(tofrom: aggregate)
  {
    s.a += 1;
    s.b += 1.0;
  }
}

// CHECK-LABEL: implicit_maps_pointer{{.*}}(
void implicit_maps_pointer (){
  double *ddyn;

// CHECK: [[DDY:%ddyn]] =  alloca ptr,
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[DDY]],
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.DEFAULT:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L]], ptr [[L]], i64 0, i64 544
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(default: pointer)
  {
    ddyn[0] += 1.0;
    ddyn[1] += 1.0;
  }
}

double *g;

// CHECK-LABEL: @_Z3foo{{.*}}(
template<typename T>
void foo(float *&lr, T *&tr) {
  float *l;
  T *t;

// CHECK: [[LR:%lr.addr]] =  alloca ptr
// CHECK: [[LL:%l]] = alloca ptr 
// CHECK: [[T:%t]] = alloca ptr
// CHECK: [[L:%[0-9]+]] = load ptr, ptr @g,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:POINTER"()
// CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[L]]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target is_device_ptr(g) defaultmap(none:pointer)
  {
    ++g;
  }
// CHECK: [[L3:%[0-9]+]] = load ptr, ptr [[LL]]
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:POINTER"()
// CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[L3]]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target is_device_ptr(l) defaultmap(none:pointer)
  {
    ++l;
  }
// CHECK: [[L6:%[0-9]+]] = load ptr, ptr [[T]]
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L6]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target is_device_ptr(t) defaultmap(none:pointer)
  {
    ++t;
  }
// CHECK: [[L9:%[0-9]+]] = load ptr, ptr [[LR]]
// CHECK: [[L10:%[0-9]+]] = load ptr, ptr [[LR]]
// CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr [[L9]])
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L10]]
// CHECK: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target is_device_ptr(lr) defaultmap(none:pointer)
  {
    ++lr;
  }

// CHECK: [[L13:%[0-9]+]] = load ptr, ptr %tr.addr
// CHECK: [[L14:%[0-9]+]] = load ptr, ptr %tr.addr
// CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAMe: "QUAL.OMP.LIVEIN"(ptr [[L13]])
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L14]]
// CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target is_device_ptr(tr) defaultmap(none:pointer)
  {
    ++tr;
  }

// CHECK: [[L17:%[0-9]+]] = load ptr, ptr %tr.addr
// CHECK: [[L18:%[0-9]+]] = load ptr, ptr %lr.addr
// CHECK: [[L19:%[0-9]+]] = load ptr, ptr %tr.addr
// CHECK: [[TV6:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr [[L17]])
// CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr [[L18]])
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L19]]
// CHECK: region.exit(token [[TV6]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target is_device_ptr(tr,lr) defaultmap(none:pointer)
  {
    ++tr;
  }
}

void bar(float *&a, int *&b) {
  foo<int>(a,b);
}


// CHECK-LABEL: explicit_maps_single_5{{.*}}(
void explicit_maps_single_5 (int ii){
  // Map of a scalar.
  int a = ii;

// CHECK: [[A:%a]] = alloca i32,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:SCALAR"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CLOSE"(ptr [[A]], ptr [[A]], i64 4, i64 1059
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(close, tofrom: a) defaultmap(none:scalar)
  {
   a++;
  }


// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:SCALAR"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM:ALWAYS.CLOSE"(ptr [[A]], ptr [[A]], i64 4, i64 1063
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(always close tofrom: a) defaultmap(none:scalar)
  {
   a++;
  }

// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE"()
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(none)
  {
   int loc = 1;
  }
}

extern int x;
#pragma omp declare target to(x)

void declare_target_to()
{
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:SCALAR"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @x, ptr @x, i64 4, i64 547
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target defaultmap(none : scalar)
  {
    x++;
  }
}


float Vector[1024];
#pragma omp declare target link(Vector)

extern int a;
#pragma omp declare target link(a)

double *ptr;
#pragma omp declare target link(ptr)

void declare_target_link()
{
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:SCALAR"()
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:AGGREGATE"()
// CHECK-SAME: "QUAL.OMP.DEFAULTMAP.NONE:POINTER"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @Vector_decl_tgt_ref_ptr, ptr @Vector, i64 4096, i64 531
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @a_decl_tgt_ref_ptr, ptr @a, i64 4, i64 531
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @ptr_decl_tgt_ref_ptr, ptr @ptr, i64 8, i64 531
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target defaultmap(none:scalar) defaultmap(none:aggregate) defaultmap(none:pointer)
  {

    Vector[a]++;
    ptr++;
  }
}
// end INTEL_COLLAB
