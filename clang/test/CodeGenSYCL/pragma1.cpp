//RUN: %clang_cc1 -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -internal-isystem %S/Inputs -fsycl-is-device -O0 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

#include "sycl.hpp"

SYCL_EXTERNAL void bar(int i);
SYCL_EXTERNAL int ibar(int i);

//CHECK-LABEL: define{{.*}}foo_unroll
void foo_unroll()
{
  //CHECK: br{{.*}}!llvm.loop [[UNROLL1:![0-9]+]]
  #pragma unroll
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[UNROLL2:![0-9]+]]
  #pragma unroll 4
  for (int i=0;i<32;++i) { bar(i); }
}

struct SIVDep {
  int A[32];
};

struct SIVDep2 {
  SIVDep X[8][16];
};

//CHECK-LABEL: foo_ivdep
void foo_ivdep(int select, SIVDep &SV, SIVDep2 &SV2, SIVDep2 *Sv2p)
{
  //CHECK: br{{.*}}!llvm.loop [[IVDEP1:![0-9]+]]
  [[intel::ivdep()]]
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[IVDEP2:![0-9]+]]
  [[intel::ivdep(4)]]
  for (int i=0;i<32;++i) { bar(i); }

  int myArray[32];
  //CHECK: load i32, i32 addrspace(4)* %i8.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %8 to i64
  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %[[IDXPROM]], !llvm.index.group [[IVDEP3:![0-9]+]]
  //CHECK: store i32 %call, i32 addrspace(4)* %[[ARRAYIDX]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP4:![0-9]+]]
  [[intel::ivdep(myArray)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i15.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %12 to i64
  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %[[IDXPROM]], !llvm.index.group [[IVDEP5:![0-9]+]]
  //CHECK: store i32 %call19, i32 addrspace(4)* %[[ARRAYIDX]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP6:![0-9]+]]
  [[intel::ivdep(myArray, 8)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i25.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %16 to i64
  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %[[IDXPROM]], !llvm.index.group [[IVDEP7:![0-9]+]]
  //CHECK: store i32 %call29, i32 addrspace(4)* %[[ARRAYIDX]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP8:![0-9]+]]
  #pragma unroll 4
  [[intel::ivdep(myArray, 8)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i35.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %21 to i64
  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %A, i64 0, i64 %[[IDXPROM]], !llvm.index.group [[IVDEP9:![0-9]+]]
  //CHECK: store i32 %call39, i32 addrspace(4)* %[[ARRAYIDX]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP10:![0-9]+]]
  [[intel::ivdep(SV.A)]]
  for (int i=0;i<32;++i) { SV.A[i] = ibar(i); }

  //CHECK: %[[PTRLOAD2:.+]] = load %struct.{{.*}}SIVDep2 addrspace(4)*, %struct.{{.*}}SIVDep2 addrspace(4)* addrspace(4)* %Sv2p.addr.ascast, align 8
  //CHECK: %[[PTRIDX:.+]] = getelementptr inbounds %struct.{{.*}}SIVDep2, %struct.{{.*}}SIVDep2 addrspace(4)* %[[PTRLOAD2]], i32 0, i32 0
  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [8 x [16 x %struct.{{.*}}SIVDep]], [8 x [16 x %struct.{{.*}}SIVDep]] addrspace(4)* %[[PTRIDX]], i64 0, i64 2
  //CHECK: %[[ARRAYIDX1:.+]] = getelementptr inbounds [16 x %struct.{{.*}}SIVDep], [16 x %struct.{{.*}}SIVDep] addrspace(4)* %[[ARRAYIDX]], i64 0, i64 3
  //CHECK: %[[PTRIDX1:.+]] = getelementptr inbounds %struct.{{.*}}SIVDep, %struct.{{.*}}SIVDep addrspace(4)* %[[ARRAYIDX1]], i32 0, i32 0
  //CHECK: %[[PTRLOAD3:.+]] = load i32, i32 addrspace(4)* %i45.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %[[PTRLOAD3]] to i64
  //CHECK: %[[ARRAYIDX2:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %[[PTRIDX1]], i64 0, i64 %[[IDXPROM]], !llvm.index.group [[IVDEP11:![0-9]+]]
  //CHECK: store i32 %call49, i32 addrspace(4)* %[[ARRAYIDX2]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP12:![0-9]+]]
  [[intel::ivdep(Sv2p->X[2][3].A)]]
  for (int i=0;i<32;++i) { Sv2p->X[2][3].A[i] = ibar(i); }

  int myArray2[32];
  int *ptr = select ? myArray : myArray2;
  //CHECK: %[[PTRLOAD1:.+]] = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.ascast, align 8
  //CHECK: %[[PTRLOAD2:.+]] = load i32, i32 addrspace(4)* %i58.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %[[PTRLOAD2]] to i64
  //CHECK: %[[PTRIDX:.+]] = getelementptr inbounds i32, i32 addrspace(4)* %[[PTRLOAD1]], i64 %[[IDXPROM]], !llvm.index.group [[IVDEP13:![0-9]+]]
  //CHECK: store i32 %call62, i32 addrspace(4)* %[[PTRIDX]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP14:![0-9]+]]
  [[intel::ivdep(ptr)]]
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 16
  //CHECK: store i32 addrspace(4)* %[[ARRAYIDX]], i32 addrspace(4)* addrspace(4)* %ptr.ascast, align 8
  //CHECK: store i32 0, i32 addrspace(4)* %i69.ascast, align 4
  ptr = &myArray[16];

  //CHECK: %[[PTRLOAD:.+]] = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.ascast, align 8
  //CHECK: %[[IDXLOAD:.+]] = load i32, i32 addrspace(4)* %i69.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %[[IDXLOAD]] to i64
  //CHECK: %[[PTRIDX:.+]] = getelementptr inbounds i32, i32 addrspace(4)* %[[PTRLOAD]], i64 %[[IDXPROM]], !llvm.index.group [[IVDEP15:![0-9]+]]
  //CHECK: store i32 %call73, i32 addrspace(4)* %[[PTRIDX]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP16:![0-9]+]]
  [[intel::ivdep(ptr)]]
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i79.ascast, align 4
  //CHECK: %[[IDXPROM:.+]] = sext i32 %41 to i64
  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %[[IDXPROM]], !llvm.index.group [[IVDEP17:![0-9]+]]
  //CHECK: store i32 %call83, i32 addrspace(4)* %[[ARRAYIDX]], align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP18:![0-9]+]]
  [[intel::ivdep(myArray2)]]
  [[intel::ivdep(myArray)]]
  [[intel::ivdep(8)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }
}

int main() {
  sycl::handler h;
  h.single_task([]() {
    foo_unroll();
    SIVDep SV;
    SIVDep2 SV2;
    foo_ivdep(1, SV, SV2, &SV2);
  });
}
//CHECK: [[UNROLL1]] = distinct !{[[UNROLL1]], ![[LOOP_MUSTPROGRESS:[0-9]+]], [[UNROLL1A:![0-9]+]]}
//CHECK: [[UNROLL1A]] = !{!"llvm.loop.unroll.enable"}
//CHECK: [[UNROLL2]] = distinct !{[[UNROLL2]], ![[LOOP_MUSTPROGRESS]], [[UNROLL2A:![0-9]+]]}
//CHECK: [[UNROLL2A]] = !{!"llvm.loop.unroll.count", i32 4}
//CHECK: [[IVDEP1]] = distinct !{[[IVDEP1]], ![[LOOP_MUSTPROGRESS]], [[IVDEP1A:![0-9]+]]}
//CHECK: [[IVDEP1A]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[IVDEP2]] = distinct !{[[IVDEP2]], ![[LOOP_MUSTPROGRESS]], [[IVDEP2A:![0-9]+]]}
//CHECK: [[IVDEP2A]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[IVDEP3]] = distinct !{}
//CHECK: [[IVDEP4]] =  distinct !{[[IVDEP4]], ![[LOOP_MUSTPROGRESS]], [[IVDEP4A:![0-9]+]]}
//CHECK: [[IVDEP4A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP3]]}
//CHECK: [[IVDEP5]] = distinct !{}
//CHECK: [[IVDEP6]] =  distinct !{[[IVDEP6]], ![[LOOP_MUSTPROGRESS]], [[IVDEP6A:![0-9]+]]}
//CHECK: [[IVDEP6A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP5]], i32 8}
//CHECK: [[IVDEP7]] = distinct !{}
//CHECK: [[IVDEP8]] = distinct !{[[IVDEP8]], ![[LOOP_MUSTPROGRESS]], [[IVDEP8A:![0-9]+]], [[UNROLL2A]]}
//CHECK: [[IVDEP8A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP7]], i32 8}
//CHECK: [[IVDEP9]] = distinct !{}
//CHECK: [[IVDEP10]] = distinct !{[[IVDEP10]], ![[LOOP_MUSTPROGRESS]], [[IVDEP10A:![0-9]+]]}
//CHECK: [[IVDEP10A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP9]]}
//CHECK: [[IVDEP11]] = distinct !{}
//CHECK: [[IVDEP12]] =  distinct !{[[IVDEP12]], ![[LOOP_MUSTPROGRESS]], [[IVDEP12A:![0-9]+]]}
//CHECK: [[IVDEP12A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP11]]}
//CHECK: [[IVDEP13]] = distinct !{}
//CHECK: [[IVDEP14]] =  distinct !{[[IVDEP14]], ![[LOOP_MUSTPROGRESS]], [[IVDEP14A:![0-9]+]]}
//CHECK: [[IVDEP14A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP13]]}
//CHECK: [[IVDEP15]] = distinct !{}
//CHECK: [[IVDEP16]] =  distinct !{[[IVDEP16]], ![[LOOP_MUSTPROGRESS]], [[IVDEP16A:![0-9]+]]}
//CHECK: [[IVDEP16A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP15]]}
//CHECK: [[IVDEP17]] = distinct !{}
//CHECK: [[IVDEP18]] =  distinct !{[[IVDEP18]], ![[LOOP_MUSTPROGRESS]], [[IVDEP18A:![0-9]+]], [[IVDEP18B:![0-9]+]]}
//CHECK: [[IVDEP18A]] = !{!"llvm.loop.ivdep.safelen", i32 8}
//CHECK: [[IVDEP18B]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP18B1:![0-9]+]], [[IVDEP17]]}
//CHECK: [[IVDEP18B1]] = distinct !{}
