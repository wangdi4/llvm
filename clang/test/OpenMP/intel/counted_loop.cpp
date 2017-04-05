// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-1
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-2
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=REG-1
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=REG-2


namespace std {
  struct random_access_iterator_tag { };
  template <class Iter> struct iterator_traits {
    typedef typename Iter::difference_type difference_type;
    typedef typename Iter::iterator_category iterator_category;
  };
  template <class Iter>
  typename iterator_traits<Iter>::difference_type
  distance(Iter first, Iter last) { return first - last; }
}
class Iter0 {
  public:
    Iter0() { }
    Iter0(const Iter0 &) { }
    Iter0 operator ++() { return *this; }
    Iter0 operator --() { return *this; }
    Iter0 operator + (int delta) { return *this; }
    bool operator <(Iter0 a) { return true; }
};
// expected-note@+1 2 {{candidate function not viable: no known conversion from 'Iter1' to 'Iter0' for 1st argument}}
int operator -(Iter0 a, Iter0 b) { return 0; }
class Iter1 {
  public:
    Iter1(float f=0.0f, double d=0.0) { }
    Iter1(const Iter1 &) { }
    Iter1 operator ++() { return *this; }
    Iter1 operator --() { return *this; }
    bool operator <(Iter1 a) { return true; }
    bool operator >=(Iter1 a) { return false; }
};
class GoodIter {
  public:
    GoodIter() { }
    GoodIter(const GoodIter &) { }
    GoodIter(int fst, int snd) { }
    GoodIter &operator =(const GoodIter &that) { return *this; }
    GoodIter &operator =(const Iter0 &that) { return *this; }
    GoodIter &operator +=(int x) { return *this; }
    explicit GoodIter(void *) { }
    GoodIter operator ++() { return *this; }
    GoodIter operator --() { return *this; }
    bool operator !() { return true; }
    bool operator <(GoodIter a) { return true; }
    bool operator <=(GoodIter a) { return true; }
    bool operator >=(GoodIter a) { return false; }
    typedef int difference_type;
    typedef std::random_access_iterator_tag iterator_category;
};
GoodIter operator -(int v, GoodIter a) { return GoodIter(); }
GoodIter operator +(int v, GoodIter a) { return GoodIter(); }
GoodIter operator -(GoodIter a, int v) { return GoodIter(); }
GoodIter operator +(GoodIter a, int v) { return GoodIter(); }
int operator -(GoodIter a, GoodIter b) { return 0; }

struct A {
  A();
  A(const A&);
  A& operator=(const A&);
  ~A();
  int *ip;
} obj;

void baz(int);

//
// A counted loop will be generated when any loop index (counter) is:
//   1) struct/class type (i.e. a c++ iterator)
//   2) Is specified lastprivate or private
//   3) Is global/address taken (i.e. NOT a local variable w/o address taken)
//

// CHECK-LABEL: @_Z4oneAv
// CHECK-1: [[ONEA_IT1:%it1.*]] = alloca %class.GoodIter,
// CHECK-2: [[ONEA_IT1:%it1.*]] = alloca %class.GoodIter,
// REG-1: [[ONEA_IT1:%it1.*]] = alloca %class.GoodIter,
// REG-2: [[ONEA_IT1:%it1.*]] = alloca %class.GoodIter,
// CHECK-1: [[ONEA_IV:%.omp.iv.*]] = alloca
// CHECK-2: [[ONEA_IV:%.omp.iv.*]] = alloca
// REG-1: [[ONEA_IV:%.omp.iv.*]] = alloca
// REG-2: [[ONEA_IV:%.omp.iv.*]] = alloca
// CHECK-1: [[ONEA_I:%i.*]] = alloca i32,
// CHECK-2: [[ONEA_I:%i.*]] = alloca i32,
// REG-1: [[ONEA_I:%i.*]] = alloca i32,
// REG-2: [[ONEA_I:%i.*]] = alloca i32,
void oneA() {
  GoodIter begin1, end1;
  
  GoodIter it1;
  
  // CHECK-1: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  // CHECK-2: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  // CHECK-1: (metadata !"QUAL.OMP.PRIVATE", i32* [[ONEA_I]])
  // CHECK-2: (metadata !"QUAL.OMP.PRIVATE", %class.GoodIter* [[ONEA_IT1]])
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-2: directive(metadata !"DIR.QUAL.LIST.END")
  // REG-1: region.entry() [ "DIR.OMP.PARALLEL.LOOP"{{.*}}"QUAL.OMP.PRIVATE"(i32* [[ONEA_I]]
  // REG-2: region.entry() [ "DIR.OMP.PARALLEL.LOOP"{{.*}}"QUAL.OMP.PRIVATE"(%class.GoodIter* [[ONEA_IT1]]
  // Initialize IV
  // CHECK-1: store {{.*}}[[ONEA_IV]]
  // CHECK-2: store {{.*}}[[ONEA_IV]]
  // REG-1: store {{.*}}[[ONEA_IV]]
  // REG-2: store {{.*}}[[ONEA_IV]]
  // Update it1
  // CHECK-1: {{.*}} call{{.*}} %class.GoodIter* @_ZN8GoodIterpLEi(%class.GoodIter* [[ONEA_IT1]]
  // CHECK-2: {{.*}} call{{.*}} %class.GoodIter* @_ZN8GoodIterpLEi(%class.GoodIter* [[ONEA_IT1]]
  // Update i
  // CHECK-1: store {{.*}}[[ONEA_I]]
  // CHECK-2: store {{.*}}[[ONEA_I]]
  // CHECK-1: {{.*}} call{{.*}}baz 
  // CHECK-2: {{.*}} call{{.*}}baz 
  // Increment IV
  // CHECK-1: store {{.*}}[[ONEA_IV]]
  // CHECK-2: store {{.*}}[[ONEA_IV]]
  // CHECK-1: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  // CHECK-2: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-2: directive(metadata !"DIR.QUAL.LIST.END")
  #pragma omp parallel for collapse(2)
  for (it1 = begin1; it1 < end1; ++it1)
  for (int i=0; i < 2; ++i)
  {
    baz(i);
  }
}

// CHECK-LABEL: @_Z4oneBv
// CHECK-1: [[ONEB_IV:%.omp.iv.*]] = alloca
// CHECK-2: [[ONEB_IV:%.omp.iv.*]] = alloca
// CHECK-1: [[ONEB_I:%i.*]] = alloca i32,
// CHECK-2: [[ONEB_I:%i.*]] = alloca i32,
// CHECK-1: [[ONEB_IT1:%it1.*]] = alloca %class.GoodIter,
// CHECK-2: [[ONEB_IT1:%it1.*]] = alloca %class.GoodIter,
// REG-1: [[ONEB_I:%i.*]] = alloca i32,
// REG-1: [[ONEB_IT1:%it1.*]] = alloca %class.GoodIter,
// REG-2: [[ONEB_I:%i.*]] = alloca i32,
// REG-2: [[ONEB_IT1:%it1.*]] = alloca %class.GoodIter,
void oneB() {
  GoodIter begin1, end1;
  // CHECK-1: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  // CHECK-2: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  // CHECK-1: (metadata !"QUAL.OMP.PRIVATE", i32* [[ONEB_I]])
  // CHECK-2: (metadata !"QUAL.OMP.PRIVATE", %class.GoodIter* [[ONEB_IT1]])
  // REG-1: region.entry() [ "DIR.OMP.PARALLEL.LOOP"{{.*}}"QUAL.OMP.PRIVATE"(i32* [[ONEB_I]])
  // REG-2: region.entry() [ "DIR.OMP.PARALLEL.LOOP"{{.*}}"QUAL.OMP.PRIVATE"(%class.GoodIter* [[ONEB_IT1]])
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-2: directive(metadata !"DIR.QUAL.LIST.END")
  // Initialize IV
  // CHECK-1: store {{.*}}[[ONEB_IV]]
  // CHECK-2: store {{.*}}[[ONEB_IV]]
  // Update i
  // CHECK-1: store {{.*}}[[ONEB_I]]
  // CHECK-2: store {{.*}}[[ONEB_I]]
  // Update it1
  // CHECK-1: {{.*}} call{{.*}} %class.GoodIter* @_ZN8GoodIterpLEi(%class.GoodIter* [[ONEB_IT1]]
  // CHECK-2: {{.*}} call{{.*}} %class.GoodIter* @_ZN8GoodIterpLEi(%class.GoodIter* [[ONEB_IT1]]
  // CHECK-1: {{.*}} call{{.*}}baz 
  // CHECK-2: {{.*}} call{{.*}}baz 
  // Increment IV
  // CHECK-1: store {{.*}}[[ONEB_IV]]
  // CHECK-2: store {{.*}}[[ONEB_IV]]
  // CHECK-1: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  // CHECK-2: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-2: directive(metadata !"DIR.QUAL.LIST.END")
  #pragma omp parallel for collapse(2)
  for (int i=0; i < 2; ++i)
  for (GoodIter it1 = begin1; it1 < end1; ++it1)
  {
    baz(i);
  }
}

// CHECK-LABEL: @_Z3twov
// CHECK-1: [[TWO_I:%i.*]] = alloca i32,
// CHECK-1: [[TWO_IV:%.omp.iv.*]] = alloca
// REG-1: [[TWO_I:%i.*]] = alloca i32,
// REG-1: [[TWO_IV:%.omp.iv.*]] = alloca
void two()
{
  int i;
  // CHECK-1: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  // CHECK-1: (metadata !"QUAL.OMP.LASTPRIVATE", i32* [[TWO_I]])
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-1: store {{.*}}[[TWO_IV]]
  // CHECK-1: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // REG-1: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.LASTPRIVATE"(i32* [[TWO_I]]
  // REG-1: store {{.*}}[[TWO_IV]]
  #pragma omp parallel for lastprivate(i)
  for (i=0; i < 2; ++i)
  {
  }
}

// CHECK-LABEL: @_Z4twoAv
// CHECK-1: [[TWOA_I:%i.*]] = alloca i32,
// CHECK-1: [[TWOA_IV:%.omp.iv.*]] = alloca
// REG-1: [[TWOA_I:%i.*]] = alloca i32,
// REG-1: [[TWOA_IV:%.omp.iv.*]] = alloca
void twoA()
{
  int i;
  // CHECK-1: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  // CHECK-1: (metadata !"QUAL.OMP.PRIVATE", i32* [[TWOA_I]])
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-1: store {{.*}}[[TWOA_IV]]
  // CHECK-1: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // REG-1: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE"(i32* [[TWOA_I]]
  // REG-1: store {{.*}}[[TWOA_IV]]
  #pragma omp parallel for private(i)
  for (i=0; i < 2; ++i)
  {
  }
}

int glob;
void bar(int *);
// CHECK-LABEL: @_Z6threeAv
// CHECK-1: [[THREEA_IV:%.omp.iv.*]] = alloca
// REG-1: [[THREEA_IV:%.omp.iv.*]] = alloca
void threeA()
{
  // CHECK-1: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  // CHECK-1: (metadata !"QUAL.OMP.PRIVATE", i32* @glob)
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-1: store {{.*}}[[THREEA_IV]]
  // CHECK-1: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // REG-1: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(){{.*}}"QUAL.OMP.PRIVATE"(i32* @glob
  #pragma omp parallel for
  for (glob=0; glob < 2; ++glob)
  {
  }
}

// CHECK-LABEL: @_Z6threeBv
// CHECK-1: [[THREEB_IV:%.omp.iv.*]] = alloca
// REG-1: [[THREEB_IV:%.omp.iv.*]] = alloca
void threeB()
{
  // CHECK-1: directive(metadata !"DIR.OMP.LOOP")
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // CHECK-1: store {{.*}}[[THREEB_IV]]
  // CHECK-1: directive(metadata !"DIR.OMP.END.LOOP")
  // CHECK-1: directive(metadata !"DIR.QUAL.LIST.END")
  // REG-1: region.entry() [ "DIR.OMP.LOOP"
  #pragma omp for
  for (glob=0; glob < 2; ++glob)
  {
  }
}

