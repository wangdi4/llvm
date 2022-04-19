//RUN: %clang_cc1 -triple spir64-unknown-unknown-intelfpga -O0 -cl-std=CL2.0 -emit-llvm -opaque-pointers -o - %s | FileCheck --check-prefixes=CHECK,SPIR %s
//RUN: %clang_cc1 -triple spir64-unknown-unknown-intelfpga -O0 -cl-std=CL1.2 -emit-llvm -opaque-pointers -o - %s | FileCheck --check-prefixes=CHECK,BOTH %s
//RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -O0 -cl-std=CL2.0 -emit-llvm -opaque-pointers -o - %s | FileCheck --check-prefixes=CHECK,BOTH %s
//RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -O0 -cl-std=CL1.2 -emit-llvm -opaque-pointers -o - %s | FileCheck --check-prefixes=CHECK,BOTH %s

// This test file is a copy of CodeGenIntelHLS/pragmas.cpp adopted for OpenCL C
// Main changes:
//   * declarations of SV, SV2 and Sv2p variables moved into foo_ivdep function
//     to workaround OpenCL C 1.2 restriction: program scope variables must
//     reside in constant address space
//   * udpated some CHECK-lines to take into account address spaces

void bar(int i);
int ibar(int i);

//CHECK-LABEL: foo_unroll
void foo_unroll()
{
  //CHECK: br{{.*}}!llvm.loop [[UNROLL1:![0-9]+]]
  #pragma unroll
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[UNROLL2:![0-9]+]]
  #pragma unroll 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_coalesce
void foo_coalesce()
{
  //CHECK: br{{.*}}!llvm.loop [[COAL1:![0-9]+]]
  #pragma loop_coalesce
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[COAL2:![0-9]+]]
  #pragma loop_coalesce 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_ii
void foo_ii()
{
  //CHECK: br{{.*}}!llvm.loop [[II1:![0-9]+]]
  #pragma ii 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_max_concurrency
void foo_max_concurrency()
{
  //CHECK: br{{.*}}!llvm.loop [[MAXC1:![0-9]+]]
  #pragma max_concurrency 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_max_interleaving
void foo_max_interleaving()
{
  //CHECK: br{{.*}}!llvm.loop [[MAXI1:![0-9]+]]
  for (int j=0;j<32;++j) {
    #pragma max_interleaving 1
    for (int i=0;i<32;++i) { bar(i); }
  }
}

//CHECK-LABEL: foo_ivdep
void foo_ivdep(int select)
{
  struct SIVDep {
    int A[32];
  } SV;

  struct SIVDep2 {
    struct SIVDep X[8][16];
  } SV2, *Sv2p = &SV2;

  //CHECK: br{{.*}}!llvm.loop [[IVDEP1:![0-9]+]]
  #pragma ivdep
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[IVDEP2:![0-9]+]]
  #pragma ivdep safelen(4)
  for (int i=0;i<32;++i) { bar(i); }

  int myArray[32];

  //CHECK: load i32, ptr %i8, align 4
  //CHECK: %idxprom = sext i32 %8 to i64
  //CHECK: %arrayidx = getelementptr inbounds [32 x i32], ptr %myArray, i64 0, i64 %idxprom, !llvm.index.group [[IVDEP3:![0-9]+]]
  //CHECK: store i32 %call, ptr %arrayidx, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP4:![0-9]+]]
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, ptr %i15, align 4
  //CHECK: %idxprom20 = sext i32 %12 to i64
  //CHECK: %arrayidx21 = getelementptr inbounds [32 x i32], ptr %myArray, i64 0, i64 %idxprom20, !llvm.index.group [[IVDEP5:![0-9]+]]
  //CHECK: store i32 %call19, ptr %arrayidx21, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP6:![0-9]+]]
  #pragma ivdep safelen(8) array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, ptr %i25, align 4
  //CHECK: %idxprom30 = sext i32 %16 to i64
  //CHECK: %arrayidx31 = getelementptr inbounds [32 x i32], ptr %myArray, i64 0, i64 %idxprom30, !llvm.index.group [[IVDEP7:![0-9]+]]
  //CHECK: store i32 %call29, ptr %arrayidx31, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP8:![0-9]+]]
  #pragma unroll 4
  #pragma ivdep safelen(8) array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, ptr %i35, align 4
  //CHECK: %idxprom40 = sext i32 %20 to i64
  //CHECK: %arrayidx41 = getelementptr inbounds [32 x i32], ptr %myArray, i64 0, i64 %idxprom40, !llvm.index.group [[IVDEP9:![0-9]+]]
  //CHECK: store i32 %call39, ptr %arrayidx41, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP10:![0-9]+]]
  #pragma ivdep safelen(8) array(myArray)
  #pragma unroll 4
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: getelementptr inbounds %struct.SIVDep, ptr %SV, i32 0, i32 0
  //CHECK: load i32, ptr %i45, align 4
  //CHECK: %idxprom50 = sext i32 %24 to i64
  //CHECK: %arrayidx51 = getelementptr inbounds [32 x i32], ptr %A, i64 0, i64 %idxprom50, !llvm.index.group [[IVDEP11:![0-9]+]]
  //CHECK: store i32 %call49, ptr %arrayidx51, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP12:![0-9]+]]
  #pragma ivdep array(SV.A)
  for (int i=0;i<32;++i) { SV.A[i] = ibar(i); }

  //SPIR: %28 = load ptr addrspace(4), ptr %Sv2p, align 8
  //SPIR: %X = getelementptr inbounds %struct.SIVDep2, ptr addrspace(4) %28, i32 0, i32 0
  //SPIR: %arrayidx60 = getelementptr inbounds [8 x [16 x %struct.SIVDep]], ptr addrspace(4) %X, i64 0, i64 2
  //SPIR: %arrayidx61 = getelementptr inbounds [16 x %struct.SIVDep], ptr addrspace(4) %arrayidx60, i64 0, i64 3
  //SPIR: %A62 = getelementptr inbounds %struct.SIVDep, ptr addrspace(4) %arrayidx61, i32 0, i32 0
  //SPIR: %29 = load i32, ptr %i55, align 4
  //SIPR: %idxprom63 = sext i32 %29 to i64
  //SPIR: %arrayidx64 = getelementptr inbounds [32 x i32], ptr addrspace(4) %A62, i64 0, i64 %idxprom63
  //SPIR: store i32 %call59, ptr addrspace(4) %arrayidx64, align 4

  //BOTH: load ptr, ptr %Sv2p, align 8
  //BOTH: %X = getelementptr inbounds %struct.SIVDep2, ptr %28, i32 0, i32 0
  //BOTH: %arrayidx60 = getelementptr inbounds [8 x [16 x %struct.SIVDep]], ptr %X, i64 0, i64 2
  //BOTH: %arrayidx61 = getelementptr inbounds [16 x %struct.SIVDep], ptr %arrayidx60, i64 0, i64 3
  //BOTH: %A62 = getelementptr inbounds %struct.SIVDep, ptr %arrayidx61, i32 0, i32 0
  //BOTH: %29 = load i32, ptr %i55, align 4
  //BOTH: %idxprom63 = sext i32 %29 to i64
  //BOTH: %arrayidx64 = getelementptr inbounds [32 x i32], ptr %A62, i64 0, i64 %idxprom63
  //BOTH: store i32 %call59, ptr %arrayidx64, align 4
  #pragma ivdep array(Sv2p->X[2][3].A)
  for (int i=0;i<32;++i) { Sv2p->X[2][3].A[i] = ibar(i); }

  int myArray2[32];
  //CHECK: getelementptr inbounds [32 x i32], ptr %myArray, i64 0, i64 0
  //CHECK: getelementptr inbounds [32 x i32], ptr %myArray2, i64 0, i64 0
  int *ptr = select ? myArray : myArray2;

  //SPIR: %34 = load ptr addrspace(4), ptr %ptr, align 8
  //SPIR: %35 = load i32, ptr %i69, align 4
  //SPIR: %idxprom74 = sext i32 %35 to i64
  //SPIR: %arrayidx75 = getelementptr inbounds i32, ptr addrspace(4) %34, i64 %idxprom74, !llvm.index.group [[IVDEP13:![0-9]+]]
  //SPIR: store i32 %call73, ptr addrspace(4) %arrayidx75, align 4
  //SPIR: br{{.*}}!llvm.loop [[IVDEP14:![0-9]+]]

  //BOTH: load ptr, ptr %ptr, align 8
  //BOTH: %35 = load i32, ptr %i69, align 4
  //BOTH: %idxprom74 = sext i32 %35 to i64
  //BOTH: %arrayidx75 = getelementptr inbounds i32, ptr %34, i64 %idxprom74, !llvm.index.group [[IVDEP13:![0-9]+]]
  //BOTH: store i32 %call73, ptr %arrayidx75, align 4
  //BOTH: br{{.*}}!llvm.loop [[IVDEP14:![0-9]+]]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  ptr = &myArray[16];

  //SPIR: %39 = load ptr addrspace(4), ptr %ptr, align 8
  //SPIR: %40 = load i32, ptr %i80, align 4
  //SPIR: %idxprom85 = sext i32 %40 to i64
  //SPIR: %arrayidx86 = getelementptr inbounds i32, ptr addrspace(4) %39, i64 %idxprom85, !llvm.index.group [[IVDEP15:![0-9]+]]
  //SPIR: store i32 %call84, ptr addrspace(4) %arrayidx86, align 4
  //SPIR: br{{.*}}!llvm.loop [[IVDEP16:![0-9]+]]

  //BOTH: load ptr, ptr %ptr, align 8
  //BOTH: %40 = load i32, ptr %i80, align 4
  //BOTH: %idxprom85 = sext i32 %40 to i64
  //BOTH: %arrayidx86 = getelementptr inbounds i32, ptr %39, i64 %idxprom85, !llvm.index.group [[IVDEP15:![0-9]+]]
  //BOTH: store i32 %call84, ptr %arrayidx86, align 4
  //BOTH: br{{.*}}!llvm.loop [[IVDEP16:![0-9]+]]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }
}

//CHECK: [[UNROLL1]] = distinct !{[[UNROLL1]], [[UNROLL1A:![0-9]+]]}
//CHECK: [[UNROLL1A]] = !{!"llvm.loop.unroll.enable"}
//CHECK: [[UNROLL2]] = distinct !{[[UNROLL2]], [[UNROLL2A:![0-9]+]]}
//CHECK: [[UNROLL2A]] = !{!"llvm.loop.unroll.count", i32 4}
//CHECK: [[COAL1]] = distinct !{[[COAL1]], [[COAL1A:![0-9]+]]}
//CHECK: [[COAL1A]] = !{!"llvm.loop.coalesce.enable"}
//CHECK: [[COAL2]] = distinct !{[[COAL2]], [[COAL2A:![0-9]+]]}
//CHECK: [[COAL2A]] = !{!"llvm.loop.coalesce.count", i32 4}
//CHECK: [[II1]] = distinct !{[[II1]], [[II1A:![0-9]+]]}
//CHECK: [[II1A]] = !{!"llvm.loop.ii.count", i32 4}
//CHECK: [[MAXC1]] = distinct !{[[MAXC1]], [[MAXC1A:![0-9]+]]}
//CHECK: [[MAXC1A]] = !{!"llvm.loop.max_concurrency.count", i32 4}
//CHECK: [[MAXI1]] = distinct !{[[MAXI1]], [[MAXI1A:![0-9]+]]}
//CHECK: [[MAXI1A]] = !{!"llvm.loop.max_interleaving.count", i32 1}
//CHECK: [[IVDEP1]] = distinct !{[[IVDEP1]], [[IVDEP1A:![0-9]+]]}
//CHECK: [[IVDEP1A]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[IVDEP2]] = distinct !{[[IVDEP2]], [[IVDEP2A:![0-9]+]]}
//CHECK: [[IVDEP2A]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[IVDEP3]] = distinct !{}
//CHECK: [[IVDEP4]] = distinct !{[[IVDEP4]], [[IVDEP4A:![0-9]+]]}
//CHECK: [[IVDEP4A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP3]]}
//CHECK: [[IVDEP5]] = distinct !{}
//CHECK: [[IVDEP6]] = distinct !{[[IVDEP6]], [[IVDEP6A:![0-9]+]]}
//CHECK: [[IVDEP6A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP5]], i32 8}
//CHECK: [[IVDEP7]] = distinct !{}
//CHECK: [[IVDEP8]] = distinct !{[[IVDEP8]], [[IVDEP8A:![0-9]+]], [[UNROLL2A]]}
//CHECK: [[IVDEP8A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP7]], i32 8}
//CHECK: [[IVDEP9]] = distinct !{}
//CHECK: [[IVDEP10]] = distinct !{[[IVDEP10]], [[IVDEP10A:![0-9]+]], [[UNROLL2A]]}
//CHECK: [[IVDEP10A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP9]], i32 8}
//CHECK: [[IVDEP11]] = distinct !{}
//CHECK: [[IVDEP12]] = distinct !{[[IVDEP12]], [[IVDEP12A:![0-9]+]]}
//CHECK: [[IVDEP12A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP11]]}
//CHECK: [[IVDEP13]] = distinct !{}
//CHECK: [[IVDEP14]] = distinct !{[[IVDEP14]], [[IVDEP14A:![0-9]+]]}
//CHECK: [[IVDEP14A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP13]]}
//CHECK: [[IVDEP15]] = distinct !{}
//CHECK: [[IVDEP16]] = distinct !{[[IVDEP16]], [[IVDEP16A:![0-9]+]]}
//CHECK: [[IVDEP16A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP15]]}

