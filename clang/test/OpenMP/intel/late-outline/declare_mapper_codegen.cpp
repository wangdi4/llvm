// INTEL_COLLAB
// expected-no-diagnostics
#ifndef HEADER
#define HEADER

///==========================================================================///
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-NO-DB %s
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -emit-pch -femit-all-decls -disable-llvm-passes -o %t %s
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -std=c++11 -femit-all-decls -disable-llvm-passes -include-pch %t -verify %s -emit-llvm -o - | FileCheck --check-prefix CK0 --check-prefix CK0-NO-DB %s
// RUN: %clang_cc1  -verify -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -triple i386-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-NO-DB %s
// RUN: %clang_cc1  -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -std=c++11 -triple i386-unknown-unknown -emit-pch -femit-all-decls -disable-llvm-passes -o %t %s
// RUN: %clang_cc1  -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -triple i386-unknown-unknown -std=c++11 -femit-all-decls -disable-llvm-passes -include-pch %t -verify %s -emit-llvm -o - | FileCheck --check-prefix CK0 --check-prefix CK0-NO-DB %s

// RUN: %clang_cc1 -verify -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-DB %s
// RUN: %clang_cc1  -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -emit-pch -femit-all-decls -disable-llvm-passes -o %t %s
// RUN: %clang_cc1 -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=powerpc64le-ibm-linux-gnu -x c++ -triple powerpc64le-unknown-unknown -std=c++11 -femit-all-decls -disable-llvm-passes -include-pch %t -verify %s -emit-llvm -o - | FileCheck --check-prefix CK0 --check-prefix CK0-DB %s
// RUN: %clang_cc1  -verify -debug-info-kind=limited -fopenmp -fopenmp-late-outline -fopenmp-targets=i386-pc-linux-gnu -x c++ -triple i386-unknown-unknown -emit-llvm -femit-all-decls -disable-llvm-passes %s -o - | FileCheck --check-prefix CK0 --check-prefix CK0-DB  %s

// Mapper function code generation and runtime interface.

// CK-DB: @{{[0-9]+}} = private unnamed_addr constant [{{[0-9]+}} x i8] c";c;{{.*}}.cpp;{{[0-9]+}};{{[0-9]+}};;\00"
// CK-DB: @{{[0-9]+}} = private unnamed_addr constant [{{[0-9]+}} x i8] c";d;{{.*}}.cpp;{{[0-9]+}};{{[0-9]+}};;\00"
class C {
public:
  int a;
  double *b;
};

#pragma omp declare mapper(id: C s) map(s.a, s.b[0:2])

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
