// RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fintel-compatibility \
// RUN:  -fopenmp -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

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

// CHECK-LABEL: @_Z4oneAv
// CHECK: [[ONEA_IT1:%it1.*]] = alloca %class.GoodIter,
// CHECK: [[ONEA_IV:%.omp.iv.*]] = alloca
// CHECK: [[ONEA_I:%i.*]] = alloca i32,
void oneA() {
  GoodIter begin1, end1;

  GoodIter it1;

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.GoodIter* [[ONEA_IT1]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[ONEA_I]]
  // Initialize IV
  // CHECK: store {{.*}}[[ONEA_IV]]
  // Update it1
  // CHECK: call{{.*}} %class.GoodIter* @_ZN8GoodIterpLEi
  // CHECK-SAME: (%class.GoodIter* [[ONEA_IT1]]
  // Update i
  // CHECK: store {{.*}}[[ONEA_I]]
  // CHECK: {{.*}} call{{.*}}baz
  // Increment IV
  // CHECK: store {{.*}}[[ONEA_IV]]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  #pragma omp parallel for collapse(2)
  for (it1 = begin1; it1 < end1; ++it1)
  for (int i=0; i < 2; ++i)
  {
    baz(i);
  }
}

// CHECK-LABEL: @_Z4oneBv
// CHECK: [[ONEB_IV:%.omp.iv.*]] = alloca
// CHECK: [[ONEB_I:%i.*]] = alloca i32,
// CHECK: [[ONEB_IT1:%it1.*]] = alloca %class.GoodIter,
void oneB() {
  GoodIter begin1, end1;
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[ONEB_I]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.GoodIter* [[ONEB_IT1]])
  // Initialize IV
  // CHECK: store {{.*}}[[ONEB_IV]]
  // Update i
  // CHECK: store {{.*}}[[ONEB_I]]
  // Update it1
  // CHECK: call{{.*}} %class.GoodIter* @_ZN8GoodIterpLEi
  // CHECK-SAME: (%class.GoodIter* [[ONEB_IT1]]
  // CHECK: {{.*}} call{{.*}}baz
  // Increment IV
  // CHECK: store {{.*}}[[ONEB_IV]]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  #pragma omp parallel for collapse(2)
  for (int i=0; i < 2; ++i)
  for (GoodIter it1 = begin1; it1 < end1; ++it1)
  {
    baz(i);
  }
}

// CHECK-LABEL: @_Z3twov
// CHECK: [[TWO_I:%i.*]] = alloca i32,
// CHECK: [[TWO_IV:%.omp.iv.*]] = alloca
void two()
{
  int i;
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE"(i32* [[TWO_I]]
  // CHECK: store {{.*}}[[TWO_IV]]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  #pragma omp parallel for lastprivate(i)
  for (i=0; i < 2; ++i)
  {
  }
}

// CHECK-LABEL: @_Z4twoAv
// CHECK: [[TWOA_I:%i.*]] = alloca i32,
// CHECK: [[TWOA_IV:%.omp.iv.*]] = alloca
void twoA()
{
  int i;
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[TWOA_I]]
  // CHECK: store {{.*}}[[TWOA_IV]]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  #pragma omp parallel for private(i)
  for (i=0; i < 2; ++i)
  {
  }
}

int glob;
void bar(int *);
// CHECK-LABEL: @_Z6threeAv
// CHECK: [[THREEA_IV:%.omp.iv.*]] = alloca
void threeA()
{
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* @glob
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  #pragma omp parallel for
  for (glob=0; glob < 2; ++glob)
  {
  }
}

// CHECK-LABEL: @_Z6threeBv
// CHECK: [[THREEB_IV:%.omp.iv.*]] = alloca
void threeB()
{
  // CHECK: region.entry() [ "DIR.OMP.LOOP"
  // CHECK: store {{.*}}[[THREEB_IV]]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.LOOP"()
  #pragma omp for
  for (glob=0; glob < 2; ++glob)
  {
  }
}
