// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -opaque-pointers -o - %s | FileCheck %s -check-prefix=X64
// RUN: %clang_cc1 -triple i386-unknown-linux-gnu -emit-llvm -opaque-pointers -o - %s | FileCheck %s -check-prefix=X86
extern void bar(void *, void *, void *);
int foo(int zc, void *lock) {
    static int lock2 = 2;
    static int lock3 = 3;
    if (zc)
      __notify_intrinsic((char*)"__itt_sync_create", lock);
    else
      __notify_intrinsic((char*)"__itt_sync_acquired", lock + 4);

    __notify_zc_intrinsic((char*)"__itt_sync_create", lock + zc);
    lock2 = lock2 + lock3;
    __notify_intrinsic((char*)"__itt_sync_destroy", (void*)&lock2);
    __notify_zc_intrinsic((char*)"__itt_sync_destroy", (void*)&lock3);

    bar(lock, (void*)&lock2, (void*)&lock3);
    return 0;
}

//X64:      @foo.lock2 = internal global i32 2, align 4
//X64-NEXT: @foo.lock3 = internal global i32 3, align 4
//X64-NEXT: @.str = private unnamed_addr constant [18 x i8] c"__itt_sync_create\00", align 1
//X64-NEXT: @.str.1 = private unnamed_addr constant [20 x i8] c"__itt_sync_acquired\00", align 1
//X64-NEXT: @.str.2 = private unnamed_addr constant [19 x i8] c"__itt_sync_destroy\00", align 1

//X64:      define{{.*}}i32 @foo(i32 noundef %zc, ptr noundef %lock) #0 {
//X64-NEXT: entry:
//X64-NEXT:   %zc.addr = alloca i32, align 4
//X64-NEXT:   %lock.addr = alloca ptr, align 8
//X64-NEXT:   store i32 %zc, ptr %zc.addr, align 4
//X64-NEXT:   store ptr %lock, ptr %lock.addr, align 8
//X64-NEXT:   %0 = load i32, ptr %zc.addr, align 4
//X64-NEXT:   %tobool = icmp ne i32 %0, 0
//X64-NEXT:   br i1 %tobool, label %if.then, label %if.else

//X64:      if.then:                                          ; preds = %entry
//X64-NEXT:   fence syncscope("singlethread") seq_cst
//X64-NEXT:   %1 = load ptr, ptr %lock.addr, align 8
//X64-NEXT:   call void @llvm.notify.nzc(ptr @.str, ptr %1)
//X64-NEXT:   br label %if.end

//X64:      if.else:                                          ; preds = %entry
//X64-NEXT:   fence syncscope("singlethread") seq_cst
//X64-NEXT:   %2 = load ptr, ptr %lock.addr, align 8
//X64-NEXT:   %add.ptr = getelementptr i8, ptr %2, i64 4
//X64-NEXT:   call void @llvm.notify.nzc(ptr @.str.1, ptr %add.ptr)
//X64-NEXT:   br label %if.end

//X64:      if.end:                                           ; preds = %if.else, %if.then
//X64-NEXT:   %3 = load ptr, ptr %lock.addr, align 8
//X64-NEXT:   %4 = load i32, ptr %zc.addr, align 4
//X64-NEXT:   %idx.ext = sext i32 %4 to i64
//X64-NEXT:   %add.ptr1 = getelementptr i8, ptr %3, i64 %idx.ext
//X64-NEXT:   call void @llvm.notify.zc(ptr @.str, ptr %add.ptr1)
//X64-NEXT:   %5 = load i32, ptr @foo.lock2, align 4
//X64-NEXT:   %6 = load i32, ptr @foo.lock3, align 4
//X64-NEXT:   %add = add nsw i32 %5, %6
//X64-NEXT:   store i32 %add, ptr @foo.lock2, align 4
//X64-NEXT:   fence syncscope("singlethread") seq_cst
//X64-NEXT:   call void @llvm.notify.nzc(ptr @.str.2, ptr @foo.lock2)
//X64-NEXT:   call void @llvm.notify.zc(ptr @.str.2, ptr @foo.lock3)
//X64-NEXT:   %7 = load ptr, ptr %lock.addr, align 8
//X64-NEXT:   call void @bar(ptr noundef %7, ptr noundef @foo.lock2, ptr noundef @foo.lock3)
//X64-NEXT:   ret i32 0
//X64-NEXT: }

//X64:      declare void @llvm.notify.nzc(ptr, ptr)
//X64:      declare void @llvm.notify.zc(ptr, ptr)
//X64:      declare void @bar(ptr noundef, ptr noundef, ptr noundef)

//X86:      @foo.lock2 = internal global i32 2, align 4
//X86-NEXT: @foo.lock3 = internal global i32 3, align 4
//X86-NEXT: @.str = private unnamed_addr constant [18 x i8] c"__itt_sync_create\00", align 1
//X86-NEXT: @.str.1 = private unnamed_addr constant [20 x i8] c"__itt_sync_acquired\00", align 1
//X86-NEXT: @.str.2 = private unnamed_addr constant [19 x i8] c"__itt_sync_destroy\00", align 1

//X86:      define{{.*}}i32 @foo(i32 noundef %zc, ptr noundef %lock) #0 {
//X86-NEXT: entry:
//X86-NEXT:   %zc.addr = alloca i32, align 4
//X86-NEXT:   %lock.addr = alloca ptr, align 4
//X86-NEXT:   store i32 %zc, ptr %zc.addr, align 4
//X86-NEXT:   store ptr %lock, ptr %lock.addr, align 4
//X86-NEXT:   %0 = load i32, ptr %zc.addr, align 4
//X86-NEXT:   %tobool = icmp ne i32 %0, 0
//X86-NEXT:   br i1 %tobool, label %if.then, label %if.else

//X86:      if.then:                                          ; preds = %entry
//X86-NEXT:   fence syncscope("singlethread") seq_cst
//X86-NEXT:   %1 = load ptr, ptr %lock.addr, align 4
//X86-NEXT:   call void @llvm.notify.nzc(ptr @.str, ptr %1)
//X86-NEXT:   br label %if.end

//X86:      if.else:                                          ; preds = %entry
//X86-NEXT:   fence syncscope("singlethread") seq_cst
//X86-NEXT:   %2 = load ptr, ptr %lock.addr, align 4
//X86-NEXT:   %add.ptr = getelementptr i8, ptr %2, i32 4
//X86-NEXT:   call void @llvm.notify.nzc(ptr @.str.1, ptr %add.ptr)
//X86-NEXT:   br label %if.end

//X86:      if.end:                                           ; preds = %if.else, %if.then
//X86-NEXT:   %3 = load ptr, ptr %lock.addr, align 4
//X86-NEXT:   %4 = load i32, ptr %zc.addr, align 4
//X86-NEXT:   %add.ptr1 = getelementptr i8, ptr %3, i32 %4
//X86-NEXT:   call void @llvm.notify.zc(ptr @.str, ptr %add.ptr1)
//X86-NEXT:   %5 = load i32, ptr @foo.lock2, align 4
//X86-NEXT:   %6 = load i32, ptr @foo.lock3, align 4
//X86-NEXT:   %add = add nsw i32 %5, %6
//X86-NEXT:   store i32 %add, ptr @foo.lock2, align 4
//X86-NEXT:   fence syncscope("singlethread") seq_cst
//X86-NEXT:   call void @llvm.notify.nzc(ptr @.str.2, ptr @foo.lock2)
//X86-NEXT:   call void @llvm.notify.zc(ptr @.str.2, ptr @foo.lock3)
//X86-NEXT:   %7 = load ptr, ptr %lock.addr, align 4
//X86-NEXT:   call void @bar(ptr noundef %7, ptr noundef @foo.lock2, ptr noundef @foo.lock3)
//X86-NEXT:   ret i32 0
//X86-NEXT: }

//X86:      declare void @llvm.notify.nzc(ptr, ptr)
//X86:      declare void @llvm.notify.zc(ptr, ptr)
//X86:      declare void @bar(ptr noundef, ptr noundef, ptr noundef)
