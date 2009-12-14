// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  clang_driver.cpp
///////////////////////////////////////////////////////////

#include "stdafx.h"
#include "backend_consumer.h"
#include "clang_driver.h"
#include "clang_compiler.h"
#include "frontend_api.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/Diagnostic.h"

#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"

#include "clang/Frontend/InitHeaderSearch.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompileOptions.h"

#include "clang/Sema/ParseAST.h"

#include "logger.h"

#include <string>

using namespace clang;
using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

#pragma comment (lib, "cl_logger.lib")
#pragma comment (lib, "cl_sys_utils.lib")

#define MAX_STR_BUFF	1024
// OCL defines, incopetability between OCL and LLVM includes
#define CL_SUCCESS                                  0
#define CL_OUT_OF_RESOURCES                         -5
#define CL_OUT_OF_HOST_MEMORY                       -6

// OpenCL compiler options
// "-D", "-D=" - options
static llvm::cl::list<std::string>
D_macros("D", llvm::cl::value_desc("macro"), llvm::cl::Prefix,
       llvm::cl::desc("Predefine the specified macro"));
static llvm::cl::list<std::string>
// "-I dir" - option
I_dirs("I", llvm::cl::value_desc("directory"), llvm::cl::Prefix,
       llvm::cl::desc("Add directory to include search path"));
// "w" - no warnings
static llvm::cl::opt<bool> OptNoWarnings("w");
// "Werror" - all warnings are errors
static llvm::cl::opt<bool> OptWarnAsErrors("Werror");


llvm::OwningPtr<TargetInfo>			gTarget;
llvm::OwningPtr<SourceManager>		gSourceMgr;
static std::string					szTriple = "i686-pc-cl.1.0";
LangOptions							gLangInfo;
CompileOptions						gCompOpt;
llvm::OwningPtr<FileManager>		gFileMgr;

// Local include path
char									szOclIncPath[MAX_STR_BUFF];
char									szCurrDirrPath[MAX_STR_BUFF];

const char g_szIncludeFiles[] = "#include<cl_kernel.h>\n";

/*
// Compilation JOB queue
// We created separate thread for compilation
// The thread is waiting on event
tTaskQueue	gCompileTaskQueue;
*/

// Declare logger client
DECLARE_LOGGER_CLIENT;

int Intel::OpenCL::ClangFE::InitClangDriver()
{
	// Initiate target
	if ( gTarget )
	{
		return 0;	// Already initialized
	}

	INIT_LOGGER_CLIENT(L"ClangCompiler", LL_DEBUG);
	
	LOG_INFO("Initialize ClangCompiler - start");

	gTarget.reset(TargetInfo::CreateTargetInfo(szTriple));

	// Initialize language, always C99
	gTarget->getDefaultLangOptions(gLangInfo);

	gLangInfo.Trigraphs = 1;
	gLangInfo.BCPLComment = 1;
	gLangInfo.DollarIdents = 1;
	gLangInfo.AsmPreprocessor = 0;
	gLangInfo.GNUMode = 0;
	gLangInfo.ImplicitInt = 0;
	gLangInfo.Digraphs = 1;
	gLangInfo.HexFloats	= 1;
	gLangInfo.C99 = 1;
	gLangInfo.Microsoft = 0;
	gLangInfo.CPlusPlus = 0;
	gLangInfo.CPlusPlus0x = 0;
	gLangInfo.CXXOperatorNames = 1;
	gLangInfo.ObjC1 = 0;
	gLangInfo.ObjC2 = 0;
	gLangInfo.ObjCNonFragileABI = 0;
	gLangInfo.PascalStrings = 0;
	gLangInfo.WritableStrings = 0;
	gLangInfo.LaxVectorConversions = 1;
	gLangInfo.Exceptions = 0;
	gLangInfo.NeXTRuntime = 0;
	gLangInfo.Freestanding = 0;
	gLangInfo.NoBuiltin = 0;
	gLangInfo.ThreadsafeStatics = 0;
	gLangInfo.Blocks = 0;
	gLangInfo.EmitAllDecls = 0;
	gLangInfo.MathErrno = 0;
	gLangInfo.OpenCL = 1;
	gLangInfo.AltiVec = 1;
	gLangInfo.OverflowChecking = 0;
	gLangInfo.HeinousExtensions = 0;
	gLangInfo.Optimize = 1;
	gLangInfo.OptimizeSize = 0;
	gLangInfo.Static = 0;
	gLangInfo.PICLevel = 0;
	gLangInfo.GNUInline = 0;
	gLangInfo.NoInline = 0;
	gLangInfo.ObjCGCBitmapPrint = 0;
	gLangInfo.InstantiationDepth = 0x63;

	// Create a file manager object to provide access to and cache the filesystem.
	gFileMgr.reset(new FileManager());

	// Retrieve local OCL include directory
	size_t	envLen;
	getenv_s(&envLen, szOclIncPath, MAX_STR_BUFF, "OCL_INCLUDE_DIR");
	szOclIncPath[envLen] = 0;

	// Add current directory
	GetCurrentDirectoryA(MAX_STR_BUFF, szCurrDirrPath);

	// Initialize Source manager
	gSourceMgr.reset(new SourceManager());

	// Compiler options
	gCompOpt.InlineFunctions = 1;
	gCompOpt.UnrollLoops = 1;
	gCompOpt.OptimizationLevel = 3;

	LOG_INFO("Initialize ClangCompiler - Finish");
	return 0;
}

int Intel::OpenCL::ClangFE::CloseClangDriver()
{
	if ( !gTarget )
	{
		return -1;	// Not initialized failed
	}

	gTarget.reset();
	gSourceMgr.reset();
	gFileMgr.reset();
	LOG_INFO("Close ClangCompiler - done");

	RELEASE_LOGGER_CLIENT;

	return 0;
}

// Append a #define line to Buf for Macro.  Macro should be of the form XXX,
// in which case we emit "#define XXX 1" or "XXX=Y z W" in which case we emit
// "#define XXX Y z W".  To get a #define with no value, use "XXX=".
static void DefineBuiltinMacro(std::vector<char> &Buf, const char *Macro,
                               const char *Command = "#define ") {
  Buf.insert(Buf.end(), Command, Command+strlen(Command));
  if (const char *Equal = strchr(Macro, '=')) {
    // Turn the = into ' '.
    Buf.insert(Buf.end(), Macro, Equal);
    Buf.push_back(' ');
    Buf.insert(Buf.end(), Equal+1, Equal+strlen(Equal));
  } else {
    // Push "macroname 1".
    Buf.insert(Buf.end(), Macro, Macro+strlen(Macro));
    Buf.push_back(' ');
    Buf.push_back('1');
  }
  Buf.push_back('\n');
}

/// PickFP - This is used to pick a value based on the FP semantics of the
/// specified FP model.
template <typename T>
static T PickFP(const llvm::fltSemantics *Sem, T IEEESingleVal,
                T IEEEDoubleVal, T X87DoubleExtendedVal, T PPCDoubleDoubleVal) {
  if (Sem == &llvm::APFloat::IEEEsingle)
    return IEEESingleVal;
  if (Sem == &llvm::APFloat::IEEEdouble)
    return IEEEDoubleVal;
  if (Sem == &llvm::APFloat::x87DoubleExtended)
    return X87DoubleExtendedVal;
  assert(Sem == &llvm::APFloat::PPCDoubleDouble);
  return PPCDoubleDoubleVal;
}

static void DefineFloatMacros(std::vector<char> &Buf, const char *Prefix,
                              const llvm::fltSemantics *Sem) {
  const char *DenormMin, *Epsilon, *Max, *Min;
  DenormMin = PickFP(Sem, "1.40129846e-45F", "4.9406564584124654e-324", 
                     "3.64519953188247460253e-4951L",
                     "4.94065645841246544176568792868221e-324L");
  int Digits = PickFP(Sem, 6, 15, 18, 31);
  Epsilon = PickFP(Sem, "1.19209290e-7F", "2.2204460492503131e-16",
                   "1.08420217248550443401e-19L",
                   "4.94065645841246544176568792868221e-324L");
  int HasInifinity = 1, HasQuietNaN = 1;
  int MantissaDigits = PickFP(Sem, 24, 53, 64, 106);
  int Min10Exp = PickFP(Sem, -37, -307, -4931, -291);
  int Max10Exp = PickFP(Sem, 38, 308, 4932, 308);
  int MinExp = PickFP(Sem, -125, -1021, -16381, -968);
  int MaxExp = PickFP(Sem, 128, 1024, 16384, 1024);
  Min = PickFP(Sem, "1.17549435e-38F", "2.2250738585072014e-308",
               "3.36210314311209350626e-4932L",
               "2.00416836000897277799610805135016e-292L");
  Max = PickFP(Sem, "3.40282347e+38F", "1.7976931348623157e+308",
               "1.18973149535723176502e+4932L",
               "1.79769313486231580793728971405301e+308L");
  
  char MacroBuf[60];
  sprintf(MacroBuf, "__%s_DENORM_MIN__=%s", Prefix, DenormMin);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_DIG__=%d", Prefix, Digits);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_EPSILON__=%s", Prefix, Epsilon);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_HAS_INFINITY__=%d", Prefix, HasInifinity);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_HAS_QUIET_NAN__=%d", Prefix, HasQuietNaN);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_MANT_DIG__=%d", Prefix, MantissaDigits);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_MAX_10_EXP__=%d", Prefix, Max10Exp);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_MAX_EXP__=%d", Prefix, MaxExp);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_MAX__=%s", Prefix, Max);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_MIN_10_EXP__=(%d)", Prefix, Min10Exp);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_MIN_EXP__=(%d)", Prefix, MinExp);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_MIN__=%s", Prefix, Min);
  DefineBuiltinMacro(Buf, MacroBuf);
  sprintf(MacroBuf, "__%s_HAS_DENORM__=1", Prefix);
  DefineBuiltinMacro(Buf, MacroBuf);
}

/// DefineTypeSize - Emit a macro to the predefines buffer that declares a macro
/// named MacroName with the max value for a type with width 'TypeWidth' a
/// signedness of 'isSigned' and with a value suffix of 'ValSuffix' (e.g. LL).
static void DefineTypeSize(const char *MacroName, unsigned TypeWidth,
                           const char *ValSuffix, bool isSigned,
                           std::vector<char> &Buf) {
  char MacroBuf[60];
  long long MaxVal;
  if (isSigned)
    MaxVal = (1LL << (TypeWidth - 1)) - 1;
  else
    MaxVal = ~0LL >> (64-TypeWidth);
  
  sprintf(MacroBuf, "%s=%llu%s", MacroName, MaxVal, ValSuffix);
  DefineBuiltinMacro(Buf, MacroBuf);
}

static void DefineType(const char *MacroName, TargetInfo::IntType Ty,
                       std::vector<char> &Buf) {
  char MacroBuf[60];
  sprintf(MacroBuf, "%s=%s", MacroName, TargetInfo::getTypeName(Ty));
  DefineBuiltinMacro(Buf, MacroBuf);
}

static void InitializePredefinedMacros(const TargetInfo &TI,
                                       const LangOptions &LangOpts,
                                       std::vector<char> &Buf) {
  char MacroBuf[60];
  // Compiler version introspection macros.
  DefineBuiltinMacro(Buf, "__llvm__=1");   // LLVM Backend
  DefineBuiltinMacro(Buf, "__clang__=1");  // Clang Frontend
  
  // Currently claim to be compatible with GCC 4.2.1-5621.
  DefineBuiltinMacro(Buf, "__APPLE_CC__=5621");
  DefineBuiltinMacro(Buf, "__GNUC_MINOR__=2");
  DefineBuiltinMacro(Buf, "__GNUC_PATCHLEVEL__=1");
  DefineBuiltinMacro(Buf, "__GNUC__=4");
  DefineBuiltinMacro(Buf, "__GXX_ABI_VERSION=1002");
  
  // Initialize language-specific preprocessor defines.
  if (LangOpts.OpenCL)
    DefineBuiltinMacro(Buf, "__VERSION__=100");
  else
    DefineBuiltinMacro(Buf, "__VERSION__=\"4.2.1 Compatible Clang Compiler\"");
  
  // These should all be defined in the preprocessor according to the
  // current language configuration.
  if (!LangOpts.Microsoft)
    DefineBuiltinMacro(Buf, "__STDC__=1");
  if (LangOpts.AsmPreprocessor)
    DefineBuiltinMacro(Buf, "__ASSEMBLER__=1");
  if (LangOpts.C99 && !LangOpts.CPlusPlus)
    DefineBuiltinMacro(Buf, "__STDC_VERSION__=199901L");
  else if (0) // STDC94 ?
    DefineBuiltinMacro(Buf, "__STDC_VERSION__=199409L");

  // Standard conforming mode?
  if (!LangOpts.GNUMode)
    DefineBuiltinMacro(Buf, "__STRICT_ANSI__=1");
  
  if (LangOpts.CPlusPlus0x)
    DefineBuiltinMacro(Buf, "__GXX_EXPERIMENTAL_CXX0X__");

  if (LangOpts.Freestanding)
    DefineBuiltinMacro(Buf, "__STDC_HOSTED__=0");
  else
    DefineBuiltinMacro(Buf, "__STDC_HOSTED__=1");
  
  if (LangOpts.ObjC1) {
    DefineBuiltinMacro(Buf, "__OBJC__=1");
    if (LangOpts.ObjCNonFragileABI) {
      DefineBuiltinMacro(Buf, "__OBJC2__=1");
      DefineBuiltinMacro(Buf, "OBJC_ZEROCOST_EXCEPTIONS=1");
      DefineBuiltinMacro(Buf, "__EXCEPTIONS=1");
    }

    if (LangOpts.getGCMode() != LangOptions::NonGC)
      DefineBuiltinMacro(Buf, "__OBJC_GC__=1");
    
    if (LangOpts.NeXTRuntime)
      DefineBuiltinMacro(Buf, "__NEXT_RUNTIME__=1");
  }
  
  // darwin_constant_cfstrings controls this. This is also dependent
  // on other things like the runtime I believe.  This is set even for C code.
  DefineBuiltinMacro(Buf, "__CONSTANT_CFSTRINGS__=1");
  
  if (LangOpts.ObjC2)
    DefineBuiltinMacro(Buf, "OBJC_NEW_PROPERTIES");

  if (LangOpts.PascalStrings)
    DefineBuiltinMacro(Buf, "__PASCAL_STRINGS__");

  if (LangOpts.Blocks) {
    DefineBuiltinMacro(Buf, "__block=__attribute__((__blocks__(byref)))");
    DefineBuiltinMacro(Buf, "__BLOCKS__=1");
  }
  
  if (LangOpts.CPlusPlus) {
    DefineBuiltinMacro(Buf, "__DEPRECATED=1");
    DefineBuiltinMacro(Buf, "__EXCEPTIONS=1");
    DefineBuiltinMacro(Buf, "__GNUG__=4");
    DefineBuiltinMacro(Buf, "__GXX_WEAK__=1");
    DefineBuiltinMacro(Buf, "__cplusplus=1");
    DefineBuiltinMacro(Buf, "__private_extern__=extern");
  }
  
  // Filter out some microsoft extensions when trying to parse in ms-compat
  // mode. 
  if (LangOpts.Microsoft) {
    DefineBuiltinMacro(Buf, "_cdecl=__cdecl");
    DefineBuiltinMacro(Buf, "__int8=__INT8_TYPE__");
    DefineBuiltinMacro(Buf, "__int16=__INT16_TYPE__");
    DefineBuiltinMacro(Buf, "__int32=__INT32_TYPE__");
    DefineBuiltinMacro(Buf, "__int64=__INT64_TYPE__");
  }
  
  if (LangOpts.Optimize)
    DefineBuiltinMacro(Buf, "__OPTIMIZE__=1");
  if (LangOpts.OptimizeSize)
    DefineBuiltinMacro(Buf, "__OPTIMIZE_SIZE__=1");
    
  // Initialize target-specific preprocessor defines.
  
  // Define type sizing macros based on the target properties.
  assert(TI.getCharWidth() == 8 && "Only support 8-bit char so far");
  DefineBuiltinMacro(Buf, "__CHAR_BIT__=8");

  unsigned IntMaxWidth;
  const char *IntMaxSuffix;
  if (TI.getIntMaxType() == TargetInfo::SignedLongLong) {
    IntMaxWidth = TI.getLongLongWidth();
    IntMaxSuffix = "LL";
  } else if (TI.getIntMaxType() == TargetInfo::SignedLong) {
    IntMaxWidth = TI.getLongWidth();
    IntMaxSuffix = "L";
  } else {
    assert(TI.getIntMaxType() == TargetInfo::SignedInt);
    IntMaxWidth = TI.getIntWidth();
    IntMaxSuffix = "";
  }
  
  DefineTypeSize("__SCHAR_MAX__", TI.getCharWidth(), "", true, Buf);
  DefineTypeSize("__SHRT_MAX__", TI.getShortWidth(), "", true, Buf);
  DefineTypeSize("__INT_MAX__", TI.getIntWidth(), "", true, Buf);
  DefineTypeSize("__LONG_MAX__", TI.getLongWidth(), "L", true, Buf);
  DefineTypeSize("__LONG_LONG_MAX__", TI.getLongLongWidth(), "LL", true, Buf);
  DefineTypeSize("__WCHAR_MAX__", TI.getWCharWidth(), "", true, Buf);
  DefineTypeSize("__INTMAX_MAX__", IntMaxWidth, IntMaxSuffix, true, Buf);

  DefineType("__INTMAX_TYPE__", TI.getIntMaxType(), Buf);
  DefineType("__UINTMAX_TYPE__", TI.getUIntMaxType(), Buf);
  DefineType("__PTRDIFF_TYPE__", TI.getPtrDiffType(0), Buf);
  DefineType("__INTPTR_TYPE__", TI.getIntPtrType(), Buf);
  DefineType("__SIZE_TYPE__", TI.getSizeType(), Buf);
  DefineType("__WCHAR_TYPE__", TI.getWCharType(), Buf);
  // FIXME: TargetInfo hookize __WINT_TYPE__.
  DefineBuiltinMacro(Buf, "__WINT_TYPE__=int");
  
  DefineFloatMacros(Buf, "FLT", &TI.getFloatFormat());
  DefineFloatMacros(Buf, "DBL", &TI.getDoubleFormat());
  DefineFloatMacros(Buf, "LDBL", &TI.getLongDoubleFormat());

  // Define a __POINTER_WIDTH__ macro for stdint.h.
  sprintf(MacroBuf, "__POINTER_WIDTH__=%d", (int)TI.getPointerWidth(0));
  DefineBuiltinMacro(Buf, MacroBuf);
  
  if (!TI.isCharSigned())
    DefineBuiltinMacro(Buf, "__CHAR_UNSIGNED__");  

  // Define fixed-sized integer types for stdint.h
  assert(TI.getCharWidth() == 8 && "unsupported target types");
  assert(TI.getShortWidth() == 16 && "unsupported target types");
  DefineBuiltinMacro(Buf, "__INT8_TYPE__=char");
  DefineBuiltinMacro(Buf, "__INT16_TYPE__=short");
  
  if (TI.getIntWidth() == 32)
    DefineBuiltinMacro(Buf, "__INT32_TYPE__=int");
  else {
    assert(TI.getLongLongWidth() == 32 && "unsupported target types");
    DefineBuiltinMacro(Buf, "__INT32_TYPE__=long long");
  }
  
  // 16-bit targets doesn't necessarily have a 64-bit type.
  if (TI.getLongLongWidth() == 64)
    DefineBuiltinMacro(Buf, "__INT64_TYPE__=long long");
  
  // Add __builtin_va_list typedef.
  {
    const char *VAList = TI.getVAListDeclaration();
    Buf.insert(Buf.end(), VAList, VAList+strlen(VAList));
    Buf.push_back('\n');
  }
  
  if (const char *Prefix = TI.getUserLabelPrefix()) {
    sprintf(MacroBuf, "__USER_LABEL_PREFIX__=%s", Prefix);
    DefineBuiltinMacro(Buf, MacroBuf);
  }
  
  // Build configuration options.  FIXME: these should be controlled by
  // command line options or something.
  DefineBuiltinMacro(Buf, "__FINITE_MATH_ONLY__=0");

  if (LangOpts.Static)
    DefineBuiltinMacro(Buf, "__STATIC__=1");
  else
    DefineBuiltinMacro(Buf, "__DYNAMIC__=1");

  if (LangOpts.GNUInline)
    DefineBuiltinMacro(Buf, "__GNUC_GNU_INLINE__=1");
  else
    DefineBuiltinMacro(Buf, "__GNUC_STDC_INLINE__=1");

  if (LangOpts.NoInline)
    DefineBuiltinMacro(Buf, "__NO_INLINE__=1");

  if (unsigned PICLevel = LangOpts.PICLevel) {
    sprintf(MacroBuf, "__PIC__=%d", PICLevel);
    DefineBuiltinMacro(Buf, MacroBuf);

    sprintf(MacroBuf, "__pic__=%d", PICLevel);
    DefineBuiltinMacro(Buf, MacroBuf);
  }

  // Macros to control C99 numerics and <float.h>
  DefineBuiltinMacro(Buf, "__FLT_EVAL_METHOD__=0");
  DefineBuiltinMacro(Buf, "__FLT_RADIX__=2");
  sprintf(MacroBuf, "__DECIMAL_DIG__=%d",
          PickFP(&TI.getLongDoubleFormat(), -1/*FIXME*/, 17, 21, 33));
  DefineBuiltinMacro(Buf, MacroBuf);
  
  // Get other target #defines.
  TI.getTargetDefines(LangOpts, Buf);
}

/// InitializePreprocessor - Initialize the preprocessor getting it and the
/// environment ready to process a single file. This returns true on error.
///
static bool InitializePreprocessor(Preprocessor &PP, llvm::MemoryBuffer *SB)
{
	FileManager &FileMgr = PP.getFileManager();

	// Figure out where to get and map in the main file.
	SourceManager &SourceMgr = PP.getSourceManager();

	SourceMgr.createMainFileIDForMemBuffer(SB);
	if (SourceMgr.getMainFileID().isInvalid())
	{
		return true;
	}

	std::vector<char> PredefineBuffer;

	// Add macros from the command line.
	for(unsigned d = 0, D = D_macros.size(); d < D; ++d)
	{
		DefineBuiltinMacro(PredefineBuffer, D_macros[d].c_str());
	}

	// Install things like __POWERPC__, __GNUC__, etc into the macro table.
	InitializePredefinedMacros(PP.getTargetInfo(), PP.getLangOptions(),
		PredefineBuffer);

	// Null terminate PredefinedBuffer and add it.
	PredefineBuffer.push_back(0);
	PP.setPredefines(&PredefineBuffer[0]);

	// Once we've read this, we're done.
	return false;
}

void CompileTask::Execute()
{
	LOG_INFO("CompileTask::Execute() - Started");
	// Build single memory buffer, which will be passed to the compiler
	// Calculate total size of source buffer
	size_t stTotalSize = 0;
	for(int i=0; i<m_pTask->uiLineCount; ++i)
	{
		stTotalSize += m_pTask->pLengths[i];
	}
	stTotalSize += sizeof(g_szIncludeFiles)-1;
	// Allocate required buffer
	llvm::MemoryBuffer *SB = llvm::MemoryBuffer::getNewUninitMemBuffer(stTotalSize);
	if ( NULL == SB )
	{
		LOG_ERROR("CompileTask::Execute() - Failed to created buffer");
		m_pTask->pCallBack(m_pTask->pData, NULL, 0, CL_OUT_OF_HOST_MEMORY, NULL);
		return;
	}
	// Copy sources to the new buffer
	char*	pBegin = (char*)SB->getBufferStart();
	memcpy_s(pBegin, stTotalSize, g_szIncludeFiles, sizeof(g_szIncludeFiles));
	pBegin += sizeof(g_szIncludeFiles)-1;
	stTotalSize -= sizeof(g_szIncludeFiles)-1;
	for(int i=0; i<m_pTask->uiLineCount; ++i)
	{
		memcpy_s(pBegin, stTotalSize, m_pTask->ppsLineArray[i], m_pTask->pLengths[i]);
		stTotalSize -= m_pTask->pLengths[i];
		pBegin += m_pTask->pLengths[i];
	}

	// Parse user options
	if ( NULL != m_pTask->pszOptions)
	{
		llvm::cl::ParseCommandLineOptions(1, (char**)&m_pTask->pszOptions);
	}

	// Initialize diagnostics
	llvm::OwningPtr<DiagnosticClient>	DiagClient;
	llvm::OwningPtr<Diagnostic>			Diags;
	SmallVector<char, 1024>				Log;
	raw_svector_ostream					LogStream(Log);
	TextDiagnosticPrinter              *TDP;


	DiagClient.reset(TDP = new TextDiagnosticPrinter(LogStream));
	Diags.reset(new Diagnostic(DiagClient.get()));

	Diags->setIgnoreAllWarnings(OptNoWarnings);
	Diags->setWarningsAsErrors(OptWarnAsErrors);

    gSourceMgr->clearIDTables();

	// Init include path
	llvm::OwningPtr<HeaderSearch>	HeaderInfo(new HeaderSearch(*gFileMgr.get()));
	InitHeaderSearch init(*HeaderInfo.get());

	// Add constant path
	init.AddPath(szOclIncPath, InitHeaderSearch::Angled, false, true, false);
	init.AddPath(szCurrDirrPath, InitHeaderSearch::Angled, false, true, false);

	// Add user defined directories
	for (unsigned Iidx = 0; Iidx != I_dirs.size(); ++Iidx)
		init.AddPath(I_dirs[Iidx], InitHeaderSearch::Angled, false, true, false);

	 init.Realize();

	// Initialize preprocessor
	llvm::OwningPtr<Preprocessor> PP(new Preprocessor(*Diags, gLangInfo, *gTarget,
                                                      *gSourceMgr, *HeaderInfo, NULL));
	TDP->setLangOptions(&gLangInfo);

    if (InitializePreprocessor(*PP, SB))
	{
		LOG_ERROR("CompileTask::Execute() - InitializePreprocessor Failed");
		m_pTask->pCallBack(m_pTask->pData, NULL, 0, CL_OUT_OF_RESOURCES, NULL);
		return;
    }

	SmallVector<char, 4096>	IRbinary;

	// Create compilation engine
	llvm::OwningPtr<BackendConsumer>  BEConsumer(new BackendConsumer(*Diags, gLangInfo, gCompOpt, &IRbinary, false));
	llvm::OwningPtr<ASTContext> ContextOwner;

	ContextOwner.reset(new ASTContext(PP->getLangOptions(),
		PP->getSourceManager(),
		PP->getTargetInfo(),
		PP->getIdentifierTable(),
		PP->getSelectorTable()));

	// Process file
	LOG_DEBUG("CompileTask::Execute() - Start processing file");
    ParseAST(*PP, BEConsumer.get(), *ContextOwner.get());
	LOG_DEBUG("CompileTask::Execute() - Processing file finished");

	// Create output buffer
	void*	pOutBuff = NULL;
	// Create Log buffer
	char*	pLogBuff = NULL;
	if ( !Log.empty() )
	{
		pLogBuff = new char[Log.size()+1];
		if ( pLogBuff != NULL )
		{
			memcpy_s(pLogBuff, Log.size(), Log.begin(), Log.size());
			pLogBuff[Log.size()] = '\0';
		}
	}

	if ( !IRbinary.empty() )
	{
		ComposeBinaryContainer(IRbinary.begin(), IRbinary.size(), &pOutBuff);
		if ( NULL == pOutBuff )
		{
			delete pLogBuff;
			LOG_ERROR("CompileTask::Execute() - Failed to allocate memory for buffer");
			m_pTask->pCallBack(m_pTask->pData, NULL, 0, CL_OUT_OF_HOST_MEMORY, NULL);
			return;
		}
	}

	LOG_INFO("CompileTask::Execute() - Finished");
	m_pTask->pCallBack(m_pTask->pData, pOutBuff, IRbinary.size(), CL_SUCCESS, pLogBuff);
	// Release log and binary
	if ( NULL != pLogBuff )
	{
		delete []pLogBuff;
	}
	delete []pOutBuff;
	IRbinary.clear();

	return;
}
