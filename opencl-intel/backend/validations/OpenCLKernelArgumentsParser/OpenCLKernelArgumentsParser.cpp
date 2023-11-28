// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "OpenCLKernelArgumentsParser.h"
#include "BufferDesc.h"
#include "IMemoryObjectDesc.h"
#include "ImageDesc.h"
#include "TypeDesc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace Validation;

TypeDesc OpenCLKernelArgumentsParser::forParserStruct(StructType *structTy) {
  int numElements = structTy->getNumElements();
  TypeDesc ElemDesc(TSTRUCT, 0, numElements);
  for (int i = 0; i < numElements; i++) {
    switch (structTy->getElementType(i)->getTypeID()) {
    case Type::FloatTyID: {
      ElemDesc.SetSubTypeDesc(i, TFLOAT);
      break;
    }
    case Type::DoubleTyID: {
      ElemDesc.SetSubTypeDesc(i, TDOUBLE);
      break;
    }
    case Type::IntegerTyID: {
      switch (structTy->getElementType(i)->getPrimitiveSizeInBits()) {
      case 8: {
        ElemDesc.SetSubTypeDesc(i, TCHAR);
        break;
      }
      case 16: {
        ElemDesc.SetSubTypeDesc(i, TSHORT);
        break;
      }
      case 32: {
        ElemDesc.SetSubTypeDesc(i, TINT);
        break;
      }
      case 64: {
        ElemDesc.SetSubTypeDesc(i, TLONG);
        break;
      }
      default: {
        throw Exception::ParserBadTypeException(
            "[OpenCLKernelArgumentsParser::forParserStruct]bad type of integer "
            "in vector");
        break;
      }
      }
      break;
    }
    case Type::FixedVectorTyID:
    case Type::ScalableVectorTyID: {
      const FixedVectorType *vectorTy =
          cast<FixedVectorType>(structTy->getElementType(i));
      std::size_t numOfElements = vectorTy->getNumElements();
      TypeDesc SubElemDesc(TVECTOR);
      SubElemDesc.SetNumberOfElements(numOfElements);
      switch (vectorTy->getElementType()->getTypeID()) {
      case Type::FloatTyID: {
        SubElemDesc.SetSubTypeDesc(0, TFLOAT);
        break;
      }
      case Type::DoubleTyID: {
        SubElemDesc.SetSubTypeDesc(0, TDOUBLE);
        break;
      }
      case Type::IntegerTyID: {
        switch (vectorTy->getElementType()->getPrimitiveSizeInBits()) {
        case 8: {
          SubElemDesc.SetSubTypeDesc(0, TCHAR);
          break;
        }
        case 16: {
          SubElemDesc.SetSubTypeDesc(0, TSHORT);
          break;
        }
        case 32: {
          SubElemDesc.SetSubTypeDesc(0, TINT);
          break;
        }
        case 64: {
          SubElemDesc.SetSubTypeDesc(0, TLONG);
          break;
        }
        default: {
          throw Exception::ParserBadTypeException(
              "[OpenCLKernelArgumentsParser::forParserStruct]bad type of "
              "integer in vector in struct");
          break;
        }
        }
        break;
      }
      default: {
        throw Exception::ParserBadTypeException(
            "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector "
            "in struct");
        break;
      }
      }
      ElemDesc.SetSubTypeDesc(i, SubElemDesc);
      break;
    }
    case Type::PointerTyID: {
      ElemDesc.SetSubTypeDesc(i, TPOINTER);
      break;
    }
    case Type::StructTyID: {
      TypeDesc StructDesc =
          forParserStruct(cast<StructType>(structTy->getElementType(i)));
      ElemDesc.SetSubTypeDesc(i, StructDesc);
      break;
    }
    case Type::ArrayTyID: {
      const ArrayType *arrayTy = cast<ArrayType>(structTy->getElementType(i));
      std::size_t numOfElements = arrayTy->getNumElements();
      TypeDesc SubElemDesc(TARRAY);
      SubElemDesc.SetNumberOfElements(numOfElements);
      switch (arrayTy->getElementType()->getTypeID()) {
      case Type::FloatTyID: {
        SubElemDesc.SetSubTypeDesc(0, TFLOAT);
        break;
      }
      case Type::DoubleTyID: {
        SubElemDesc.SetSubTypeDesc(0, TDOUBLE);
        break;
      }
      case Type::IntegerTyID: {
        switch (arrayTy->getElementType()->getPrimitiveSizeInBits()) {
        case 8: {
          SubElemDesc.SetSubTypeDesc(0, TCHAR);
          break;
        }
        case 16: {
          SubElemDesc.SetSubTypeDesc(0, TSHORT);
          break;
        }
        case 32: {
          SubElemDesc.SetSubTypeDesc(0, TINT);
          break;
        }
        case 64: {
          SubElemDesc.SetSubTypeDesc(0, TLONG);
          break;
        }
        default: {
          throw Exception::ParserBadTypeException(
              "[OpenCLKernelArgumentsParser::forParserStruct]bad type of "
              "integer in vector in pointer");
          break;
        }
        }
        break;
      }
      case Type::FixedVectorTyID:
      case Type::ScalableVectorTyID: {
        const FixedVectorType *vectorTy =
            cast<FixedVectorType>(arrayTy->getElementType());
        std::size_t numOfElements = vectorTy->getNumElements();
        TypeDesc VectorSubElemDesc(TVECTOR);
        VectorSubElemDesc.SetNumberOfElements(numOfElements);
        switch (vectorTy->getElementType()->getTypeID()) {
        case Type::FloatTyID: {
          VectorSubElemDesc.SetSubTypeDesc(0, TFLOAT);
          break;
        }
        case Type::DoubleTyID: {
          VectorSubElemDesc.SetSubTypeDesc(0, TDOUBLE);
          break;
        }
        case Type::IntegerTyID: {
          switch (vectorTy->getElementType()->getPrimitiveSizeInBits()) {
          case 8: {
            VectorSubElemDesc.SetSubTypeDesc(0, TCHAR);
            break;
          }
          case 16: {
            VectorSubElemDesc.SetSubTypeDesc(0, TSHORT);
            break;
          }
          case 32: {
            VectorSubElemDesc.SetSubTypeDesc(0, TINT);
            break;
          }
          case 64: {
            VectorSubElemDesc.SetSubTypeDesc(0, TLONG);
            break;
          }
          default: {
            throw Exception::ParserBadTypeException(
                "[OpenCLKernelArgumentsParser::forParserStruct]bad type of "
                "integer in vector in pointer");
            break;
          }
          }
          break;
        }
        default: {
          throw Exception::ParserBadTypeException(
              "[OpenCLKernelArgumentsParser::forParserStruct]bad type in "
              "vector in pointer");
          break;
        } break;
        }
        SubElemDesc.SetSubTypeDesc(0, VectorSubElemDesc);
        break;
      }

      default: {
        throw Exception::ParserBadTypeException(
            "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector "
            "in pointer");
        break;
      }
      }
      ElemDesc.SetSubTypeDesc(i, SubElemDesc);
      break;
    }
    default: {
      throw Exception::ParserBadTypeException(
          "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector");
      break;
    }
    }
  }
  return ElemDesc;
}

OCLKernelArgumentsList OpenCLKernelArgumentsParser::KernelArgumentsParser(
    const std::string &kernelName, const llvm::Module *M) {
  OCLKernelArgumentsList ListOfArguments;

  // Extract 'kernel' function from program
  Function *m_pKernel = M->getFunction(kernelName);
  if (!m_pKernel)
    return ListOfArguments;

  assert(m_pKernel->getCallingConv() == llvm::CallingConv::SPIR_KERNEL &&
         "Given function isn't a kernel");

  SYCLKernelMetadataAPI::KernelMetadataAPI KMD(m_pKernel);
  assert(KMD.ArgBaseTypeList.hasValue() && "expect kernel_arg_type");

  for (const auto &[Idx, Arg] : llvm::enumerate(m_pKernel->args())) {
    switch (Arg.getType()->getTypeID()) {
    case Type::FloatTyID: {
      TypeDesc ElemDesc(TFLOAT);
      BufferDesc BufDesc;
      BufDesc.SetElementDecs(ElemDesc);
      BufDesc.SetNumOfElements(1);
      ListOfArguments.push_back(BufDesc);
      break;
    }
    case Type::DoubleTyID: {
      TypeDesc ElemDesc(TDOUBLE);
      BufferDesc BufDesc;
      BufDesc.SetElementDecs(ElemDesc);
      BufDesc.SetNumOfElements(1);
      ListOfArguments.push_back(BufDesc);
      break;
    }
    case Type::IntegerTyID: {
      TypeDesc ElemDesc;
      switch (Arg.getType()->getPrimitiveSizeInBits()) {
      case 8: {
        ElemDesc = TypeDesc(TCHAR);
        break;
      }
      case 16: {
        ElemDesc = TypeDesc(TSHORT);
        break;
      }
      case 32: {
        ElemDesc = TypeDesc(TINT);
        break;
      }
      case 64: {
        ElemDesc = TypeDesc(TLONG);
        break;
      }
      default: {
        throw Exception::ParserBadTypeException(
            "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type of "
            "integer in vector");
        break;
      }
      }
      BufferDesc BufDesc;
      BufDesc.SetElementDecs(ElemDesc);
      BufDesc.SetNumOfElements(1);
      ListOfArguments.push_back(BufDesc);
      break;
    }
    case Type::FixedVectorTyID:
    case Type::ScalableVectorTyID: {
      std::size_t numOfElements =
          dyn_cast<FixedVectorType>(Arg.getType())->getNumElements();
      TypeDesc ElemDesc(TVECTOR);
      ElemDesc.SetNumberOfElements(numOfElements);
      switch (
          dyn_cast<VectorType>(Arg.getType())->getElementType()->getTypeID()) {
      case Type::FloatTyID: {
        ElemDesc.SetSubTypeDesc(0, TFLOAT);
        break;
      }
      case Type::DoubleTyID: {
        ElemDesc.SetSubTypeDesc(0, TDOUBLE);
        break;
      }
      case Type::IntegerTyID: {
        switch (dyn_cast<VectorType>(Arg.getType())
                    ->getElementType()
                    ->getPrimitiveSizeInBits()) {
        case 8: {
          ElemDesc.SetSubTypeDesc(0, TCHAR);
          break;
        }
        case 16: {
          ElemDesc.SetSubTypeDesc(0, TSHORT);
          break;
        }
        case 32: {
          ElemDesc.SetSubTypeDesc(0, TINT);
          break;
        }
        case 64: {
          ElemDesc.SetSubTypeDesc(0, TLONG);
          break;
        }
        default: {
          throw Exception::ParserBadTypeException(
              "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type of "
              "integer in vector");
          break;
        }
        }
        break;
      }
      default: {
        throw Exception::ParserBadTypeException(
            "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type in "
            "vector");
        break;
      }
      }
      BufferDesc BufDesc;
      BufDesc.SetElementDecs(ElemDesc);
      BufDesc.SetNumOfElements(1);
      ListOfArguments.push_back(BufDesc);
      break;
    }
    case Type::PointerTyID: {
      TypeDesc ElemDesc(TPOINTER);
      StringRef ArgTypeName(KMD.ArgBaseTypeList.getItem(Idx));
      ArgTypeName = ArgTypeName.drop_back();
      auto SetSubType = [](TypeDesc &Desc, StringRef TypeName) {
        if (TypeName == "float") {
          Desc.SetSubTypeDesc(0, TFLOAT);
        } else if (TypeName == "double") {
          Desc.SetSubTypeDesc(0, TDOUBLE);
        } else if (TypeName.contains("char")) {
          Desc.SetSubTypeDesc(0, TCHAR);
        } else if (TypeName.contains("short")) {
          Desc.SetSubTypeDesc(0, TSHORT);
        } else if (TypeName.contains("int")) {
          Desc.SetSubTypeDesc(0, TINT);
        } else if (TypeName.contains("long")) {
          Desc.SetSubTypeDesc(0, TLONG);
        } else {
          return false;
        }
        return true;
      };
      if (!SetSubType(ElemDesc, ArgTypeName)) {
        constexpr StringRef ExtVector = "__attribute__((ext_vector_type(";
        if (ArgTypeName.contains(ExtVector)) {
          TypeDesc SubElemDesc(TVECTOR);
          if (!SetSubType(SubElemDesc,
                          ArgTypeName.substr(0, ArgTypeName.find(' '))))
            throw Exception::ParserBadTypeException(
                "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type "
                "of integer in vector in pointer");
          ArgTypeName = ArgTypeName.drop_back(3);
          StringRef Num = ArgTypeName.substr(ArgTypeName.find(ExtVector) +
                                             ExtVector.size());
          unsigned NumOfElements;
          [[maybe_unused]] bool Fail = Num.getAsInteger(10, NumOfElements);
          assert(!Fail && "fail to get number of elements");
          SubElemDesc.SetNumberOfElements(NumOfElements);
          ElemDesc.SetSubTypeDesc(0, SubElemDesc);
        } else if (ArgTypeName.consume_front("struct ")) {
          auto *STy = StructType::getTypeByName(
              M->getContext(), (Twine("struct.") + ArgTypeName).str());
          assert(STy && "struct type not found in module");
          TypeDesc SubElemDesc = forParserStruct(STy);
          ElemDesc.SetSubTypeDesc(0, SubElemDesc);
        } else if (ArgTypeName.contains("*")) {
          throw Exception::ParserBadTypeException(
              "[OpenCLKernelArgumentsParser::KernelArgumentsParser]pointer to "
              "pointer");
        } else
          throw Exception::ParserBadTypeException(
              "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type in "
              "pointer");
      }
      StructType *ST = dyn_cast_or_null<StructType>(Arg.getParamByValType());
      if ((ST && !ST->isLiteral()) || ArgTypeName.startswith("opencl")) {
        const StringRef ImgArg = ST ? ST->getName() : ArgTypeName;
        if (ImgArg.startswith("opencl.image")) {
          // Image identifier was found.
          // Get dimension image type
          if (ImgArg.startswith("opencl.image1d_t") ||
              ImgArg.startswith("opencl.image1d_array_t") ||
              ImgArg.startswith("opencl.image1d_buffer_t") ||
              ImgArg.startswith("opencl.image2d_t") ||
              ImgArg.startswith("opencl.image2d_depth_t") ||
              ImgArg.startswith("opencl.image2d_array_t") ||
              ImgArg.startswith("opencl.image2d_array_depth_t") ||
              ImgArg.startswith("opencl.image3d_t")) {
            ImageDesc ImgDesc;
            ListOfArguments.push_back(ImgDesc);
          } else {
            throw Exception::InvalidArgument(
                "[OpenCLKernelArgumentsParser] Unknown image type");
          }
        } else {
          BufferDesc BufDesc;
          BufDesc.SetElementDecs(ElemDesc);
          BufDesc.SetNumOfElements(1); // one pointer to 0 objects of type TYPE
          // in BUFFER desc. thats not pointer desc
          ListOfArguments.push_back(BufDesc);
        }
      } else {
        BufferDesc BufDesc;
        BufDesc.SetElementDecs(ElemDesc);
        BufDesc.SetNumOfElements(1); // one pointer to 0 objects of type TYPE
        // in BUFFER desc. thats not pointer desc
        ListOfArguments.push_back(BufDesc);
      }
      break;
    }
    case Type::StructTyID: {
      TypeDesc ElemDesc = forParserStruct(cast<StructType>(Arg.getType()));
      BufferDesc BufDesc;
      BufDesc.SetElementDecs(ElemDesc);
      BufDesc.SetNumOfElements(1);
      ListOfArguments.push_back(BufDesc);
      break;
    }
    default: {
      throw Exception::ParserBadTypeException(
          "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type");
      break;
    }
    }
  }
  return ListOfArguments;
}

OCLKernelArgumentsList OpenCLKernelArgumentsParser::KernelArgHeuristics(
    const OCLKernelArgumentsList &Args, const size_t *globalworksize,
    const uint64_t dim) {
  OCLKernelArgumentsList result; // result
  BufferDesc *head;              // ptr to each buffer in OCLKernelArgumentsList
  // each buffer is tree head
  uint64_t def_size = 1; // default number of pointed elems for each ptr
  uint64_t i;

  for (i = 0; i < dim; ++i)
    def_size *= globalworksize[i]; // calculate default size

  for (i = 0; i < Args.size(); ++i) {
    // result.size() trees
    if (Args[i].get()->GetName() != BufferDesc::GetBufferDescName()) {
      result.push_back(*Args[i].get());
      continue;
    }
    head = static_cast<BufferDesc *>(Args[i].get()); // set up new head
    BufferDesc Buffd;
    // replace number of pointed elements
    Buffd.SetElementDecs(
        RecursiveDFS((*head).GetElementDescription(), def_size));
    Buffd.SetNumOfElements(
        head->NumOfElements()); // number elements in the buffer
    // is the same
    result.push_back(Buffd); // add new buff desc to result
  }
  return result;
}
