//===--- backend_consumer.cpp - Interface to LLVM backend technologies -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#pragma once

//#include "ASTConsumers.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/CompileOptions.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/SubtargetFeature.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO.h"

using namespace clang;
using namespace llvm;

class BackendConsumer : public ASTConsumer {
    CompileOptions CompileOpts;
    bool GenerateDebugInfo;
    ASTContext *Context;
	SmallVectorImpl<char>	*OStream;
    llvm::OwningPtr<CodeGenerator> Gen;
    
    llvm::Module *TheModule;
    llvm::TargetData *TheTargetData;
    llvm::raw_ostream *AsmOutStream;

    mutable llvm::ModuleProvider *ModuleProvider;
    mutable FunctionPassManager *CodeGenPasses;
    mutable PassManager *PerModulePasses;
    mutable FunctionPassManager *PerFunctionPasses;

    FunctionPassManager *getCodeGenPasses() const;
    PassManager *getPerModulePasses() const;
    FunctionPassManager *getPerFunctionPasses() const;

    void CreatePasses();

    /// AddEmitPasses - Add passes necessary to emit assembly or LLVM
    /// IR.
    ///
    /// \return True on success. On failure \arg Error will be set to
    /// a user readable error message.
    bool AddEmitPasses(std::string &Error);

    void EmitAssembly();
    
  public:  
    BackendConsumer(Diagnostic &Diags, 
                    const LangOptions &langopts, const CompileOptions &compopts,
                    SmallVectorImpl<char> *pBinary, bool debug) :
      CompileOpts(compopts),
      GenerateDebugInfo(debug),
      Gen(CreateLLVMCodeGen(Diags, "OCL program", compopts)),
      TheModule(0), TheTargetData(0), AsmOutStream(0), ModuleProvider(0),
      CodeGenPasses(0), PerModulePasses(0), PerFunctionPasses(0),
	  OStream(pBinary) {
    }

    ~BackendConsumer() {
      delete AsmOutStream;
      delete TheTargetData;
      delete ModuleProvider;
      delete CodeGenPasses;
      delete PerModulePasses;
      delete PerFunctionPasses;
    }

    virtual void Initialize(ASTContext &Ctx) {
      Context = &Ctx;
      
      
      Gen->Initialize(Ctx);

      TheModule = Gen->GetModule();
      ModuleProvider = new ExistingModuleProvider(TheModule);
      TheTargetData = new llvm::TargetData(Ctx.Target.getTargetDescription());
      
    }
    
	ASTContext *GetContext() {return Context;};

    virtual void HandleTopLevelDecl(DeclGroupRef D) {
      PrettyStackTraceDecl CrashInfo(*D.begin(), SourceLocation(),
                                     Context->getSourceManager(),
                                     "LLVM IR generation of declaration");
      
      Gen->HandleTopLevelDecl(D);
    }
    
    virtual void HandleTranslationUnit(ASTContext &C) {
      {
        PrettyStackTraceString CrashInfo("Per-file LLVM IR generation");

        Gen->HandleTranslationUnit(C);

      }

      // EmitAssembly times and registers crash info itself.
      EmitAssembly();
      
      // Force a flush here in case we never get released.
      if (AsmOutStream)
        AsmOutStream->flush();
    }
    
    virtual void HandleTagDeclDefinition(TagDecl *D) {
      PrettyStackTraceDecl CrashInfo(D, SourceLocation(),
                                     Context->getSourceManager(),
                                     "LLVM IR generation of declaration");
      Gen->HandleTagDeclDefinition(D);
    }

    virtual void CompleteTentativeDefinition(VarDecl *D) {
      Gen->CompleteTentativeDefinition(D);
    }
  };  
