// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-pc-windows-msvc -emit-dtrans-info -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK

class XObjectPtr {
public:
 explicit
 XObjectPtr() ;
 ~XObjectPtr();
};

class StylesheetExecutionContext {
public:
    virtual void
    pushVariable( const XObjectPtr val) = 0;
};

int Func(StylesheetExecutionContext& executionContext)
{
    XObjectPtr theValue;

    executionContext.pushVariable( theValue);
    return 0;
}

// CHECK: define{{.*}} @"?Func@@YAHAAVStylesheetExecutionContext@@@Z"{{.*}} !intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]
// CHECK: %[[EXEC_CTX_ADDR:.+]] = alloca ptr
// CHECK: %[[EXEC_CTX:.+]] = load ptr, ptr %[[EXEC_CTX_ADDR]]
// CHECK: %[[VTABLE:.+]] = load ptr, ptr %[[EXEC_CTX]] 
// CHECK: %[[VIRT_FUNC:.+]] = getelementptr inbounds ptr, ptr %[[VTABLE]]
// CHECK: %[[VIRT_FUNC_PTR:.+]] = load ptr, ptr %[[VIRT_FUNC]]
// CHECK: call x86_thiscallcc void %[[VIRT_FUNC_PTR]](ptr{{.*}}, ptr inalloca(<{ %"class..?AVXObjectPtr@@.XObjectPtr", [3 x i8] }>) {{.*}}), !intel_dtrans_type ![[FUNC_PTR_TYPE:[0-9]+]]


// CHECK: !intel.dtrans.types = !{![[STYLE_SHEET_TYPE:[0-9]+]], ![[XOBJ_TYPE:[0-9]+]]}

// CHECK: ![[STYLE_SHEET_TYPE]] = !{!"S", %"class..?AVStylesheetExecutionContext@@.StylesheetExecutionContext" zeroinitializer, i32 1, ![[FUNC_PTR_PTR:[0-9]+]]} 
// CHECK: ![[FUNC_PTR_PTR]] = !{![[EXEC_VARIADIC_FUNC:[0-9]+]], i32 2}
// CHECK: ![[EXEC_VARIADIC_FUNC]] = !{!"F", i1 true, i32 0, ![[INT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}

// CHECK: ![[XOBJ_TYPE]] = !{!"S", %"class..?AVXObjectPtr@@.XObjectPtr" zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}

// CHECK: ![[FUNC_MD]] = distinct !{![[EXEC_CTX_PTR:[0-9]+]]}
// CHECK: ![[EXEC_CTX_PTR]] = !{%"class..?AVStylesheetExecutionContext@@.StylesheetExecutionContext" zeroinitializer, i32 1} 

// CHECK: ![[FUNC_PTR_TYPE]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[EXEC_CTX_PTR]], ![[INALLOCA_LIT:[0-9]+]]
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[INALLOCA_LIT]] = !{!"L", i32 2, ![[XOBJ_REF:[0-9]+]], ![[PADDING_ARR:[0-9]+]]}
// CHECK: ![[PADDING_ARR]] = !{!"A", i32 3, ![[CHAR]]}
