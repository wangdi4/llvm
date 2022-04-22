// INTEL_COLLAB
// expected-no-diagnostics
#ifndef HEADER
#define HEADER

///==========================================================================///
// RUN: %clang_cc1 -no-opaque-pointers -verify -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-64 --check-prefix CK0-NO-DB %s
// RUN: %clang_cc1 -no-opaque-pointers -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -emit-pch -femit-all-decls -disable-llvm-passes -o %t %s
// RUN: %clang_cc1 -no-opaque-pointers -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -std=c++11 -femit-all-decls -disable-llvm-passes -include-pch %t -verify %s -emit-llvm -o - | FileCheck --check-prefix CK0 --check-prefix CK0-64 --check-prefix CK0-NO-DB %s
// RUN: %clang_cc1 -no-opaque-pointers  -verify -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -triple i386-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-32 --check-prefix CK0-NO-DB %s
// RUN: %clang_cc1 -no-opaque-pointers  -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -std=c++11 -triple i386-unknown-unknown -emit-pch -femit-all-decls -disable-llvm-passes -o %t %s
// RUN: %clang_cc1 -no-opaque-pointers  -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -triple i386-unknown-unknown -std=c++11 -femit-all-decls -disable-llvm-passes -include-pch %t -verify %s -emit-llvm -o - | FileCheck --check-prefix CK0 --check-prefix CK0-32 --check-prefix CK0-NO-DB %s

// RUN: %clang_cc1 -no-opaque-pointers -verify -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-64 --check-prefix CK0-DB %s
// RUN: %clang_cc1 -no-opaque-pointers  -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -emit-pch -femit-all-decls -disable-llvm-passes -o %t %s
// RUN: %clang_cc1 -no-opaque-pointers -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -std=c++11 -femit-all-decls -disable-llvm-passes -include-pch %t -verify %s -emit-llvm -o - | FileCheck --check-prefix CK0 --check-prefix CK0-64 --check-prefix CK0-DB %s
// RUN: %clang_cc1 -no-opaque-pointers  -verify -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -triple i386-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-DB --check-prefix CK0-32  %s

// Mapper function code generation and runtime interface.

// CK-DB: @{{[0-9]+}} = private unnamed_addr constant [{{[0-9]+}} x i8] c";c;{{.*}}.cpp;{{[0-9]+}};{{[0-9]+}};;\00"
// CK-DB: @{{[0-9]+}} = private unnamed_addr constant [{{[0-9]+}} x i8] c";d;{{.*}}.cpp;{{[0-9]+}};{{[0-9]+}};;\00"
class C {
public:
  int a;
  double *b;
};

#pragma omp declare mapper(id: C s) map(s.a, s.b[0:2])

// CK0: define {{.*}}void [[MPRFUNC:@[.]omp_mapper[.].*C[.]id]](i8*{{.*}}, i8*{{.*}}, i8*{{.*}}, i64{{.*}}, i64{{.*}}, i8*{{.*}})
// CK0: store i8* %{{[^,]+}}, i8** [[HANDLEADDR:%[^,]+]]
// CK0: store i8* %{{[^,]+}}, i8** [[BPTRADDR:%[^,]+]]
// CK0: store i8* %{{[^,]+}}, i8** [[VPTRADDR:%[^,]+]]
// CK0: store i64 %{{[^,]+}}, i{{64|32}}* [[SIZEADDR:%[^,]+]]
// CK0: store i64 %{{[^,]+}}, i64* [[TYPEADDR:%[^,]+]]
// CK0-DAG: [[BYTESIZE:%.+]] = load i64, i64* [[SIZEADDR]]
// CK0-64-DAG: [[SIZE:%.+]] = udiv exact i64 [[BYTESIZE]], 16
// CK0-32-DAG: [[SIZE:%.+]] = udiv exact i64 [[BYTESIZE]], 8
// CK0-DAG: [[TYPE:%.+]] = load i64, i64* [[TYPEADDR]]
// CK0-DAG: [[HANDLE:%.+]] = load i8*, i8** [[HANDLEADDR]]
// CK0-DAG: [[BPTR:%.+]] = load i8*, i8** [[BPTRADDR]]
// CK0-DAG: [[BEGIN:%.+]] = load i8*, i8** [[VPTRADDR]]
// CK0-DAG: [[ISARRAY:%.+]] = icmp sgt i64 [[SIZE]], 1
// CK0-DAG: [[PTRBEGIN:%.+]] = bitcast i8* [[BEGIN]] to %class.C*
// CK0-DAG: [[PTREND:%.+]] = getelementptr %class.C, %class.C* [[PTRBEGIN]], i64 [[SIZE]]
// CK0-DAG: [[PTRSNE:%.+]] = icmp ne i8* [[BPTR]], [[BEGIN]]
// CK0-DAG: [[CMP:%.+]] = or i1 [[ISARRAY]], [[PTRSNE]]
// CK0-DAG: [[TYPEDEL:%.+]] = and i64 [[TYPE]], 8
// CK0-DAG: [[ISNOTDEL:%.+]] = icmp eq i64 [[TYPEDEL]], 0
// CK0-DAG: [[CMP1:%.+]] = and i1 [[CMP]], [[ISNOTDEL]]
// CK0: br i1 [[CMP1]], label %[[INIT:[^,]+]], label %[[LHEAD:[^,]+]]
// CK0: [[INIT]]
// CK0-64-DAG: [[ARRSIZE:%.+]] = mul nuw i64 [[SIZE]], 16
// CK0-32-DAG: [[ARRSIZE:%.+]] = mul nuw i64 [[SIZE]], 8

// Remove movement mappings and mark as implicit
// CK0-DAG: [[ITYPE:%.+]] = and i64 [[TYPE]], -4
// CK0-DAG: [[ITYPE1:%.+]] = or i64 [[ITYPE]], 512
// CK0: call void @__tgt_push_mapper_component(i8* [[HANDLE]], i8* [[BPTR]], i8* [[BEGIN]], i64 [[ARRSIZE]], i64 [[ITYPE1]], {{.*}})
// CK0: br label %[[LHEAD:[^,]+]]

// CK0: [[LHEAD]]
// CK0: [[ISEMPTY:%.+]] = icmp eq %class.C* [[PTRBEGIN]], [[PTREND]]
// CK0: br i1 [[ISEMPTY]], label %[[DONE:[^,]+]], label %[[LBODY:[^,]+]]
// CK0: [[LBODY]]
// CK0: [[PTR:%.+]] = phi %class.C* [ [[PTRBEGIN]], %{{.+}} ], [ [[PTRNEXT:%.+]], %[[LCORRECT:[^,]+]] ]
// CK0-DAG: [[ABEGIN:%.+]] = getelementptr inbounds %class.C, %class.C* [[PTR]], i32 0, i32 0
// CK0-DAG: [[BBEGIN:%.+]] = getelementptr inbounds %class.C, %class.C* [[PTR]], i32 0, i32 1
// CK0-DAG: [[BBEGIN2:%.+]] = getelementptr inbounds %class.C, %class.C* [[PTR]], i32 0, i32 1
// CK0-DAG: [[BARRBEGIN:%.+]] = load double*, double** [[BBEGIN2]]
// CK0-DAG: [[BARRBEGINGEP:%.+]] = getelementptr inbounds double, double* [[BARRBEGIN]], i[[sz:64|32]] 0
// CK0-DAG: [[BEND:%.+]] = getelementptr double*, double** [[BBEGIN]], i32 1
// CK0-DAG: [[ABEGINV:%.+]] = bitcast i32* [[ABEGIN]] to i8*
// CK0-DAG: [[BENDV:%.+]] = bitcast double** [[BEND]] to i8*
// CK0-DAG: [[ABEGINI:%.+]] = ptrtoint i8* [[ABEGINV]] to i64
// CK0-DAG: [[BENDI:%.+]] = ptrtoint i8* [[BENDV]] to i64
// CK0-DAG: [[CSIZE:%.+]] = sub i64 [[BENDI]], [[ABEGINI]]
// CK0-DAG: [[CUSIZE:%.+]] = sdiv exact i64 [[CSIZE]], ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)
// CK0-DAG: [[PRESIZE:%.+]] = call i64 @__tgt_mapper_num_components(i8* [[HANDLE]])
// CK0-DAG: [[SHIPRESIZE:%.+]] = shl i64 [[PRESIZE]], 48
// CK0-DAG: [[BPTRADDR0BC:%.+]] = bitcast %class.C* [[PTR]] to i8*
// CK0-DAG: [[PTRADDR0BC:%.+]] = bitcast i32* [[ABEGIN]] to i8*
// CK0-DAG: [[MEMBERTYPE:%.+]] = add nuw i64 0, [[SHIPRESIZE]]
// CK0-DAG: [[TYPETF:%.+]] = and i64 [[TYPE]], 3
// CK0-DAG: [[ISALLOC:%.+]] = icmp eq i64 [[TYPETF]], 0
// CK0-DAG: br i1 [[ISALLOC]], label %[[ALLOC:[^,]+]], label %[[ALLOCELSE:[^,]+]]
// CK0-DAG: [[ALLOC]]
// CK0-DAG: [[ALLOCTYPE:%.+]] = and i64 [[MEMBERTYPE]], -4
// CK0-DAG: br label %[[TYEND:[^,]+]]
// CK0-DAG: [[ALLOCELSE]]
// CK0-DAG: [[ISTO:%.+]] = icmp eq i64 [[TYPETF]], 1
// CK0-DAG: br i1 [[ISTO]], label %[[TO:[^,]+]], label %[[TOELSE:[^,]+]]
// CK0-DAG: [[TO]]
// CK0-DAG: [[TOTYPE:%.+]] = and i64 [[MEMBERTYPE]], -3
// CK0-DAG: br label %[[TYEND]]
// CK0-DAG: [[TOELSE]]
// CK0-DAG: [[ISFROM:%.+]] = icmp eq i64 [[TYPETF]], 2
// CK0-DAG: br i1 [[ISFROM]], label %[[FROM:[^,]+]], label %[[TYEND]]
// CK0-DAG: [[FROM]]
// CK0-DAG: [[FROMTYPE:%.+]] = and i64 [[MEMBERTYPE]], -2
// CK0-DAG: br label %[[TYEND]]
// CK0-DAG: [[TYEND]]
// CK0-DAG: [[PHITYPE0:%.+]] = phi i64 [ [[ALLOCTYPE]], %[[ALLOC]] ], [ [[TOTYPE]], %[[TO]] ], [ [[FROMTYPE]], %[[FROM]] ], [ [[MEMBERTYPE]], %[[TOELSE]] ]
// CK0: call void @__tgt_push_mapper_component(i8* [[HANDLE]], i8* [[BPTRADDR0BC]], i8* [[PTRADDR0BC]], i64 [[CUSIZE]], i64 [[PHITYPE0]], {{.*}})
// CK0-DAG: [[BPTRADDR1BC:%.+]] = bitcast %class.C* [[PTR]] to i8*
// CK0-DAG: [[PTRADDR1BC:%.+]] = bitcast i32* [[ABEGIN]] to i8*
// 281474976710659 == 0x1,000,000,003
// CK0-DAG: [[MEMBERTYPE:%.+]] = add nuw i64 281474976710659, [[SHIPRESIZE]]
// CK0-DAG: [[TYPETF:%.+]] = and i64 [[TYPE]], 3
// CK0-DAG: [[ISALLOC:%.+]] = icmp eq i64 [[TYPETF]], 0
// CK0-DAG: br i1 [[ISALLOC]], label %[[ALLOC:[^,]+]], label %[[ALLOCELSE:[^,]+]]
// CK0-DAG: [[ALLOC]]
// CK0-DAG: [[ALLOCTYPE:%.+]] = and i64 [[MEMBERTYPE]], -4
// CK0-DAG: br label %[[TYEND:[^,]+]]
// CK0-DAG: [[ALLOCELSE]]
// CK0-DAG: [[ISTO:%.+]] = icmp eq i64 [[TYPETF]], 1
// CK0-DAG: br i1 [[ISTO]], label %[[TO:[^,]+]], label %[[TOELSE:[^,]+]]
// CK0-DAG: [[TO]]
// CK0-DAG: [[TOTYPE:%.+]] = and i64 [[MEMBERTYPE]], -3
// CK0-DAG: br label %[[TYEND]]
// CK0-DAG: [[TOELSE]]
// CK0-DAG: [[ISFROM:%.+]] = icmp eq i64 [[TYPETF]], 2
// CK0-DAG: br i1 [[ISFROM]], label %[[FROM:[^,]+]], label %[[TYEND]]
// CK0-DAG: [[FROM]]
// CK0-DAG: [[FROMTYPE:%.+]] = and i64 [[MEMBERTYPE]], -2
// CK0-DAG: br label %[[TYEND]]
// CK0-DAG: [[TYEND]]
// CK0-DAG: [[TYPE1:%.+]] = phi i64 [ [[ALLOCTYPE]], %[[ALLOC]] ], [ [[TOTYPE]], %[[TO]] ], [ [[FROMTYPE]], %[[FROM]] ], [ [[MEMBERTYPE]], %[[TOELSE]] ]
// CK0: call void @__tgt_push_mapper_component(i8* [[HANDLE]], i8* [[BPTRADDR1BC]], i8* [[PTRADDR1BC]], i64 4, i64 [[TYPE1]], {{.*}})
// CK0-DAG: [[BPTRADDR2BC:%.+]] = bitcast double** [[BBEGIN]] to i8*
// CK0-DAG: [[PTRADDR2BC:%.+]] = bitcast double* [[BARRBEGINGEP]] to i8*
// 281474976710675 == 0x1,000,000,013
// CK0-DAG: [[MEMBERTYPE:%.+]] = add nuw i64 281474976710675, [[SHIPRESIZE]]
// CK0-DAG: [[TYPETF:%.+]] = and i64 [[TYPE]], 3
// CK0-DAG: [[ISALLOC:%.+]] = icmp eq i64 [[TYPETF]], 0
// CK0-DAG: br i1 [[ISALLOC]], label %[[ALLOC:[^,]+]], label %[[ALLOCELSE:[^,]+]]
// CK0-DAG: [[ALLOC]]
// CK0-DAG: [[ALLOCTYPE:%.+]] = and i64 [[MEMBERTYPE]], -4
// CK0-DAG: br label %[[TYEND:[^,]+]]
// CK0-DAG: [[ALLOCELSE]]
// CK0-DAG: [[ISTO:%.+]] = icmp eq i64 [[TYPETF]], 1
// CK0-DAG: br i1 [[ISTO]], label %[[TO:[^,]+]], label %[[TOELSE:[^,]+]]
// CK0-DAG: [[TO]]
// CK0-DAG: [[TOTYPE:%.+]] = and i64 [[MEMBERTYPE]], -3
// CK0-DAG: br label %[[TYEND]]
// CK0-DAG: [[TOELSE]]
// CK0-DAG: [[ISFROM:%.+]] = icmp eq i64 [[TYPETF]], 2
// CK0-DAG: br i1 [[ISFROM]], label %[[FROM:[^,]+]], label %[[TYEND]]
// CK0-DAG: [[FROM]]
// CK0-DAG: [[FROMTYPE:%.+]] = and i64 [[MEMBERTYPE]], -2
// CK0-DAG: br label %[[TYEND]]
// CK0-DAG: [[TYEND]]
// CK0-DAG: [[TYPE2:%.+]] = phi i64 [ [[ALLOCTYPE]], %[[ALLOC]] ], [ [[TOTYPE]], %[[TO]] ], [ [[FROMTYPE]], %[[FROM]] ], [ [[MEMBERTYPE]], %[[TOELSE]] ]
// CK0: call void @__tgt_push_mapper_component(i8* [[HANDLE]], i8* [[BPTRADDR2BC]], i8* [[PTRADDR2BC]], i64 16, i64 [[TYPE2]], {{.*}})
// CK0: [[PTRNEXT]] = getelementptr %class.C, %class.C* [[PTR]], i32 1
// CK0: [[ISDONE:%.+]] = icmp eq %class.C* [[PTRNEXT]], [[PTREND]]
// CK0: br i1 [[ISDONE]], label %[[LEXIT:[^,]+]], label %[[LBODY]]

// CK0: [[LEXIT]]
// CK0: [[ISARRAY:%.+]] = icmp sgt i64 [[SIZE]], 1
// CK0: [[TYPEDEL:%.+]] = and i64 [[TYPE]], 8
// CK0: [[ISNOTDEL:%.+]] = icmp ne i64 [[TYPEDEL]], 0
// CK0: [[CMP1:%.+]] = and i1 [[ISARRAY]], [[ISNOTDEL]]
// CK0: br i1 [[CMP1]], label %[[EVALDEL:[^,]+]], label %[[DONE]]
// CK0: [[EVALDEL]]
// CK0-64-DAG: [[ARRSIZE:%.+]] = mul nuw i64 [[SIZE]], 16
// CK0-32-DAG: [[ARRSIZE:%.+]] = mul nuw i64 [[SIZE]], 8

// Remove movement mappings and mark as implicit
// CK0-DAG: [[DTYPE:%.+]] = and i64 [[TYPE]], -4
// CK0-DAG: [[DTYPE1:%.+]] = or i64 [[DTYPE]], 512
// CK0: call void @__tgt_push_mapper_component(i8* [[HANDLE]], i8* [[BPTR]], i8* [[BEGIN]], i64 [[ARRSIZE]], i64 [[DTYPE1]], {{.*}})
// CK0: br label %[[DONE]]
// CK0: [[DONE]]
// CK0: ret void


// CK0-LABEL: define {{.*}}void @{{.*}}foo{{.*}}
void foo(int a){
  int i = a;
  C c;
  c.a = a;
//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CK0-NO-DB: "QUAL.OMP.MAP.TOFROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 35, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.TOFROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 35, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target map(mapper(id),tofrom: c)
  {
    ++c.a;
  }

//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]

//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TASK
//CK0-SAME: "QUAL.OMP.SHARED"(%class.C* %c)

//CK0: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CK0-NO-DB: "QUAL.OMP.MAP.TOFROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 35, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.TOFROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 35, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target map(mapper(id),tofrom: c) nowait
  {
    ++c.a;
  }
//CK0: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TASK"() ]


//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CK0-NO-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 33, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 33, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
//CK0-SAME: "QUAL.OMP.SHARED"(%class.C* %c)
  #pragma omp target teams map(mapper(id),to: c)
  {
    ++c.a;
  }
//CK0: region.exit(token [[TV1]]) [ "DIR.OMP.END.TEAMS"() ]
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]



//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TASK
//CK0-SAME: "QUAL.OMP.SHARED"(%class.C* %c)

//CK0: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CK0-NO-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 33, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 33, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
//CK-SAME: "QUAL.OMP.SHARED"(%class.C* %c)
  #pragma omp target teams map(mapper(id),to: c) nowait
  {
    ++c.a;
  }
//CK0: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
//CK0: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TASK"() ]

//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
//CK0-NO-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 1, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 1, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target enter data map(mapper(id),to: c)
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]

//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TASK
//CK0-SAME: "QUAL.OMP.SHARED"(%class.C* %c)
//CK0: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
//CK0-NO-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 1, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 1, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target enter data map(mapper(id),to: c) nowait
//CK0: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TASK"() ]



//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.EXIT.DATA
//CK0-NO-DB: "QUAL.OMP.MAP.FROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 2,  i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.FROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 2,  [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target exit data map(mapper(id),from: c)
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]

//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TASK
//CK0-SAME: "QUAL.OMP.SHARED"(%class.C* %c)
//CK0: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.EXIT.DATA
//CK0-NO-DB: "QUAL.OMP.MAP.FROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 2, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.FROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 2, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target exit data map(mapper(id),from: c) nowait
//CK0: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TASK"() ]

//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.UPDATE
//CK0-NO-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 1, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.TO"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 1, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target update to(mapper(id): c)
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET.UPDATE"() ]


//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TASK
//CK0-SAME: "QUAL.OMP.SHARED"(%class.C* %c)
//CK0: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.UPDATE
//CK0-NO-DB: "QUAL.OMP.MAP.FROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 2, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
//CK0-DB: "QUAL.OMP.MAP.FROM"(%class.C* %c, %class.C* %c, i64 {{.*}}, i64 2, [{{[0-9]+}} x i8]* @7, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1C.id
  #pragma omp target update from(mapper(id): c) nowait
//CK0: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET.UPDATE"() ]
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TASK"() ]
}

class D {
public:
  C c;
};

#pragma omp declare mapper(id1: D d) map(d.c.a, d.c.b[0:2])

// CK0-LABEL: define {{.*}}void @{{.*}}zoo{{.*}}
void zoo(int a)
{
  D d;
  d.c.a = a;
//CK0: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CK0-NO-DB: "QUAL.OMP.MAP.TOFROM"(%class.D* %d, %class.D* %d, i64 {{.*}}, i64 35, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1D.id1
//CK0-DB: "QUAL.OMP.MAP.TOFROM"(%class.D* %d, %class.D* %d, i64 {{.*}}, i64 35, [{{[0-9]+}} x i8]* @8, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1D.id1
  #pragma omp target map(mapper(id1),tofrom: d)
  {
    ++d.c.a;
    ++d.c.b;
  }
//CK0: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}

#endif // HEADER
// end INTEL_COLLAB
