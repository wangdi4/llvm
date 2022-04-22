// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -O0 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN: -fintel-compatibility -fopenmp-late-outline \
// RUN: -fopenmp-targets=spir64 -emit-llvm %s -o - \
// RUN:  | FileCheck %s --check-prefix HOST
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple spir64 -fopenmp \
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
//HOST: [[L0:%[0-9]+]] = bitcast %class.Base* [[X]] to i8***
//HOST: [[VT:%vtable]] = load i8**, i8*** [[L0]]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME: "QUAL.OMP.MAP.TOFROM"
//HOST-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%class.Base* [[X]], i8** [[VT]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[L9:%[0-9]+]] = bitcast %class.Derived* [[dobj]] to i8***
//HOST: [[VT1:%vtable1]] = load i8**, i8*** [[L9]]
//HOST: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME: "QUAL.OMP.MAP.TOFROM"
//HOST-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%class.Derived* [[dobj]], i8** [[VT1]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[L18:%[0-9]+]] = bitcast %struct.D* [[d]] to i8***
//HOST: [[VT2:%vtable2]] = load i8**, i8*** [[L18]]
//HOST: [[L19:%[0-9]+]] = bitcast %struct.D* [[d]] to i8***
//HOST: [[VT3:%vtable3]] = load i8**, i8*** [[L19]]
//HOST: [[L20:%[0-9]+]] = bitcast %struct.D* [[d]] to i8*
//HOST: [[ADD:%add.ptr]] = getelementptr inbounds i8, i8* [[L20]]
//HOST: [[L21:%[0-9]+]] =  bitcast i8* [[ADD]] to i8***
//HOST: [[VT4:%vtable4]] = load i8**, i8*** [[L21]]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME:"QUAL.OMP.MAP.TO:CHAIN"(%struct.D* [[d]], i8** [[VT2]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST-SAME:"QUAL.OMP.MAP.TO:CHAIN"(%struct.D* [[d]], i8** [[VT3]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.D* [[d]], i8** [[VT4]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[L35:%[0-9]+]] = bitcast %struct.D* %d to %struct.B*
//HOST: [[I:%i]] = getelementptr inbounds %struct.B, %struct.B* [[L35]],
//HOST: [[L36:%[0-9]+]] = bitcast %struct.D* %d to i8***
//HOST: [[VT7:%vtable7]] = load i8**, i8*** [[L36]]
//HOST: [[L37:%[0-9]+]] = bitcast %struct.D* %d to i8***
//HOST: [[VT8:%vtable8]] = load i8**, i8*** [[L37]]
//HOST: [[L38:%[0-9]+]] = bitcast %struct.D* [[d]] to i8*
//HOST: [[ADD1:%add.ptr9]] = getelementptr inbounds i8, i8* [[L38]]
//HOST: [[L39:%[0-9]+]] = bitcast i8* [[ADD1]]
//HOST: [[VT10:%vtable10]] = load i8**, i8*** [[L39]]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//HOST-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.D* [[d]], %struct.D* [[d]], i64 %46, i64 32, i8* null, i8* null)
//HOST-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.D* [[d]], i32* [[I]], i64 4, i64 281474976710659, i8* null, i8* null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.D* [[d]], i8** [[VT7]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.D* [[d]], i8** [[VT8]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.D* [[d]], i8** [[VT10]], i32 8, i64 281474976710673, i8* null, i8* null)
//HOST: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//HOST: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.UPDATE
//HOST-SAME:  "QUAL.OMP.MAP.TO:CHAIN"(%class.Base* %X, i8** %vtable18, i32 8, i64 281474976710673, i8* null, i8* null)
//region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET.UPDATE"()]
//HOST: ret i32

//HOST: !omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6}
//HOST: !4 = !{i32 1, !"_ZTV4Base", i32 0, i32 4, { [3 x i8*] }* @_ZTV4Base}
//HOST: !5 = !{i32 1, !"_ZTV1D", i32 0, i32 6, { [5 x i8*], [5 x i8*] }* @_ZTV1D}
//HOST: !6 = !{i32 1, !"_ZTV7Derived", i32 0, i32 5, { [3 x i8*] }* @_ZTV7Derived}

//TARG: @_ZTV4Base = linkonce_odr target_declare unnamed_addr addrspace(1) constant
//TARG: @_ZTV7Derived = linkonce_odr target_declare unnamed_addr addrspace(1) constant
//TARG: define {{.*}}main
//TARG: [[X:%X]] = alloca %class.Base
//TARG: [[dobj:%dobj]] = alloca %class.Derived
//TARG: [[d:%d]] = alloca %struct.D
//TARG: [[XA:%X.ascast]] = addrspacecast %class.Base* [[X]] to %class.Base addrspace(4)*
//TARG: [[dobja:%dobj.ascast]] = addrspacecast %class.Derived* [[dobj]] to %class.Derived addrspace(4)*
//TARG: [[da:%d.ascast]] = addrspacecast %struct.D* [[d]] to %struct.D addrspace(4)*
//TARG: [[L0:%[0-9]+]] =  bitcast %class.Base addrspace(4)* [[XA]] to i8 addrspace(4)* addrspace(4)* addrspace(4)*
//TARG: [[VT:%vtable]] = load i8 addrspace(4)* addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspace(4)* [[L0]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%class.Base addrspace(4)* [[XA]], i8 addrspace(4)* addrspace(4)* [[VT]], i32 8, i64 281474976710673, i8* null, i8* null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[L9:%[0-9]+]] = bitcast %class.Derived addrspace(4)* [[dobja]] to i8 addrspace(4)* addrspace(4)* addrspace(4)*
//TARG: [[VT1:%vtable1]] = load i8 addrspace(4)* addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspace(4)* [[L9]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%class.Derived addrspace(4)* [[dobja]], i8 addrspace(4)* addrspace(4)* [[VT1]], i32 8, i64 281474976710673, i8* null, i8* null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[L18:%[0-9]+]] =  bitcast %struct.D addrspace(4)* [[da]] to i8 addrspace(4)* addrspace(4)* addrspace(4)*
//TARG: [[VT2:%vtable2]] = load i8 addrspace(4)* addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspace(4)* [[L18]]
//TARG: [[L19:%[0-9]+]] = bitcast %struct.D addrspace(4)* [[da]] to i8 addrspace(4)* addrspace(4)* addrspace(4)*
//TARG: [[VT3:%vtable3]] = load i8 addrspace(4)* addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspace(4)* [[L19]]
//TARG: [[L20:%[0-9]+]] = bitcast %struct.D addrspace(4)* [[da]]
//TARG: [[ADD:%add.ptr]] = getelementptr inbounds i8, i8 addrspace(4)* [[L20]]
//TARG: [[L21:%[0-9]+]] =  bitcast i8 addrspace(4)* [[ADD]]
//TARG: [[VT4:%vtable4]] = load i8 addrspace(4)* addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspace(4)* [[L21]]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.D addrspace(4)* [[da]], i8 addrspace(4)* addrspace(4)* [[VT2]], i32 8, i64 281474976710673, i8* null, i8* null)
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.D addrspace(4)* [[da]], i8 addrspace(4)* addrspace(4)* [[VT3]], i32 8, i64 281474976710673, i8* null, i8* null)
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.D addrspace(4)* [[da]], i8 addrspace(4)* addrspace(4)* [[VT4]], i32 8, i64 281474976710673, i8* null, i8* null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//TARG-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.D addrspace(4)* [[da]], %struct.D addrspace(4)* [[da]]
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
//TARG: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.UPDATE
//TARG-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%class.Base addrspace(4)* %X.ascast, i8 addrspace(4)* addrspace(4)* %vtable18, i32 8, i64 281474976710673, i8* null, i8* null)
//TARG: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET.UPDATE"() ]
//TARG: ret i32

//TARG: !omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6}
//TARG: !4 = !{i32 1, !"_ZTV4Base", i32 0, i32 4, { [3 x i8 addrspace(4)*] } addrspace(1)* @_ZTV4Base}
//TARG: !5 = !{i32 1, !"_ZTV1D", i32 0, i32 6, { [5 x i8 addrspace(4)*], [5 x i8 addrspace(4)*] } addrspace(1)* @_ZTV1D}
//TARG: !6 = !{i32 1, !"_ZTV7Derived", i32 0, i32 5, { [3 x i8 addrspace(4)*] } addrspace(1)* @_ZTV7Derived}

// INTEL_COLLAB
