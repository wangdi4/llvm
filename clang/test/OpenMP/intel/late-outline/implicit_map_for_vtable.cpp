// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -O0 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN: -fintel-compatibility -fopenmp-late-outline \
// RUN: -fopenmp-targets=spir64 -emit-llvm %s -o - \
// RUN:  | FileCheck %s --check-prefix HOST
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s --check-prefix TARG
//

class Base {
public:
  int x = 16;
  Base() { x = 16;}
#pragma omp declare target
  virtual void m1() {}
#pragma omp end declare target
};

class Derived : public Base {
public:
  int j = 32;
  Derived() { j = 32;}
#pragma omp declare target
  void m1() override {}
#pragma omp end declare target
};
#pragma omp declare target
void zoo(Base *D) {
  D->m1();
}
void bar(Derived *X) {
  X->m1();
}
#pragma omp end declare target


struct A {
#pragma omp declare target
  virtual void f(){ }
#pragma omp end declare target
};
struct B : virtual public A { int i;};
struct C : virtual public A {int j;};
struct D : public B, public C {
#pragma omp declare target
  void f() override {}
#pragma omp end declare target
};

#pragma omp declare target
void xoo(A *X) {
  X->f();
}
#pragma omp end declare target


int main() {
  Base X;
//expected-warning@+1 {{Type 'Base' is not trivially copyable and not guaranteed to be mapped correctly}}
#pragma omp target map(tofrom: X)
  {
    zoo(&X);
  }
  Derived dobj;
//expected-warning@+1 {{Type 'Derived' is not trivially copyable and not guaranteed to be mapped correctly}}
#pragma omp target map(tofrom: dobj)
  {
    bar(&dobj);
  }
  D d;
//expected-warning@+1 {{Type 'D' is not trivially copyable and not guaranteed to be mapped correctly}}
#pragma omp target map(tofrom:d)
  {
    xoo(&d);
  }
#pragma omp target map(tofrom:d.i)
  {
    xoo(&d);
  }

#pragma omp target firstprivate(d)
  {
    xoo(&d);
  }
//expected-warning@+1 {{Type 'Base' is not trivially copyable and not guaranteed to be mapped correctly}}
#pragma omp target update to(X)
}
//HOST: @_ZTV4Base = linkonce_odr target_declare unnamed_addr constant
//HOST: @_ZTV7Derived = linkonce_odr target_declare unnamed_addr constant
//HOST: @_ZTV1D = linkonce_odr target_declare unnamed_addr constant
//HOST: define dso_local{{.*}}main
//HOST: [[X:%X]] = alloca %class.Base
//HOST: [[dobj:%dobj]] = alloca %class.Derived
//HOST: [[d:%d]] = alloca %struct.D
//HOST: [[VT:%vtable]] = load ptr, ptr [[X]]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME: "QUAL.OMP.MAP.TOFROM"
//HOST-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[X]], ptr [[VT]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[VT1:%vtable1]] = load ptr, ptr [[dobj]]
//HOST: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME: "QUAL.OMP.MAP.TOFROM"
//HOST-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[dobj]], ptr [[VT1]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[VT2:%[^,]+]] = load ptr, ptr [[d]]
//HOST: [[VT3:%[^,]+]] = load ptr, ptr [[d]]
//HOST: [[ADD:%[^,]+]] = getelementptr inbounds i8, ptr [[d]]
//HOST: [[VT4:%[^,]+]] = load ptr, ptr [[ADD]]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME:"QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT2]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST-SAME:"QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT3]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT4]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[I:%i]] = getelementptr inbounds %struct.B, ptr [[d]],
//HOST: [[VT7:%[^,]+]] = load ptr, ptr [[d]]
//HOST: [[VT8:%[^,]+]] = load ptr, ptr [[d]]
//HOST: [[ADD1:%[^,]+]] = getelementptr inbounds i8, ptr [[d]]
//HOST: [[VT10:%[^,]+]] = load ptr, ptr [[ADD1]]
//HOST: [[DIV:%[0-9]+]] = sdiv
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[d]], ptr [[d]], i64 [[DIV]], i64 32, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[d]], ptr [[I]], i64 4, i64 281474976710659, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT7]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT8]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT10]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[VT18:%[^,]+]] = load ptr, ptr [[d]]
//HOST: [[VT19:%[^,]+]] = load ptr, ptr [[d]]
//HOST: [[ADD2:%[^,]+]] = getelementptr inbounds i8, ptr  [[d]]
//HOST: [[VT21:%[^,]+]] = load ptr, ptr [[ADD2]]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(ptr %d, ptr @_ZTS1D.omp.copy_constr, ptr @_ZTS1D.omp.destr)
//HOST-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[d]], ptr %d, i64 %30, i64 32, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT18]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT19]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr [[d]], ptr [[VT21]], i32 8, i64 281474976710673, ptr null, ptr null)
//HOST: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[VT29:%[^,]+]] = load ptr, ptr [[X]]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.UPDATE
//HOST-SAME:  "QUAL.OMP.MAP.TO:CHAIN"(ptr [[X]], ptr [[VT29]], i32 8, i64 281474976710673, ptr null, ptr null)
//region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET.UPDATE"()]
//HOST: ret i32

//HOST: !omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6, !7}
//HOST: !5 = !{i32 1, !"_ZTV4Base", i32 0, i32 5, ptr @_ZTV4Base}
//HOST: !6 = !{i32 1, !"_ZTV1D", i32 0, i32 7, ptr @_ZTV1D}
//HOST: !7 = !{i32 1, !"_ZTV7Derived", i32 0, i32 6, ptr @_ZTV7Derived}

//TARG: @_ZTV4Base = linkonce_odr target_declare unnamed_addr addrspace(1) constant
//TARG: @_ZTV7Derived = linkonce_odr target_declare unnamed_addr addrspace(1) constant
//TARG: define {{.*}}main
//TARG: [[X:%X]] = alloca %class.Base
//TARG: [[dobj:%dobj]] = alloca %class.Derived
//TARG: [[d:%d]] = alloca %struct.D
//TARG: [[XA:%X.ascast]] = addrspacecast ptr [[X]] to ptr addrspace(4)
//TARG: [[dobja:%dobj.ascast]] = addrspacecast ptr [[dobj]] to ptr addrspace(4)
//TARG: [[da:%d.ascast]] = addrspacecast ptr [[d]] to ptr addrspace(4)
//TARG: [[VT:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[XA]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[XA]], ptr addrspace(4) [[VT]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[VT1:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[dobja]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[dobja]], ptr addrspace(4) [[VT1]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[VT2:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[da]]
//TARG: [[VT3:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[da]]
//TARG: [[ADD:%[^,]+]] = getelementptr inbounds i8, ptr addrspace(4) [[da]]
//TARG: [[VT4:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[ADD]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[da]], ptr addrspace(4) [[VT2]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[da]], ptr addrspace(4) [[VT3]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[da]], ptr addrspace(4) [[VT4]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) [[da]], ptr addrspace(4) [[da]]
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[VT18:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[da]]
//TARG: [[VT19:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[da]]
//TARG: [[ADD:%[^,]+]] = getelementptr inbounds i8, ptr addrspace(4) [[da]]
//TARG: [[VT21:%[^,]+]] = load ptr addrspace(4), ptr addrspace(4) [[ADD]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(ptr addrspace(4) [[da]], ptr @_ZTS1D.omp.copy_constr, ptr @_ZTS1D.omp.destr)
//TARG-SAME: "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) [[da]], ptr addrspace(4) [[da]], i64 %40, i64 32, ptr null, ptr null)
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[da]], ptr addrspace(4) [[VT18]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[da]], ptr addrspace(4) [[VT19]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[da]], ptr addrspace(4) [[VT21]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[VT18:%vtable.*]] = load ptr addrspace(4), ptr addrspace(4) [[X]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.UPDATE
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr addrspace(4) [[XA]], ptr addrspace(4) [[VT18]], i32 8, i64 281474976710673, ptr null, ptr null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET.UPDATE"() ]
//TARG: ret i32

//TARG: !omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6, !7}
//TARG: !5 = !{i32 1, !"_ZTV4Base", i32 0, i32 5, ptr addrspace(1) @_ZTV4Base}
//TARG: !6 = !{i32 1, !"_ZTV1D", i32 0, i32 7, ptr addrspace(1) @_ZTV1D}
//TARG: !7 = !{i32 1, !"_ZTV7Derived", i32 0, i32 6, ptr addrspace(1) @_ZTV7Derived}

// INTEL_COLLAB
