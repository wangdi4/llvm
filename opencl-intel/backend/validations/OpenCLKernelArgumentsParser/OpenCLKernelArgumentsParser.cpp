/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLKernelArgumentsParser.cpp

\*****************************************************************************/


#include "BufferDesc.h"
#include "ImageDesc.h"
#include "IMemoryObjectDesc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Type.h"
#include "OpenCLKernelArgumentsParser.h"
#include "TypeDesc.h"


using namespace llvm;
using namespace Validation;

TypeDesc OpenCLKernelArgumentsParser::forParserStruct(StructType *structTy)
{
    int numElements = structTy->getNumElements();
    TypeDesc ElemDesc(TSTRUCT,0,numElements);
    for(int i=0;i<numElements;i++)
    {
        switch(structTy->getElementType(i)->getTypeID())
        {
        case Type::FloatTyID:
            {
                ElemDesc.SetSubTypeDesc(i,TFLOAT);
                break;
            }
        case Type::DoubleTyID:
            {
                ElemDesc.SetSubTypeDesc(i,TDOUBLE);
                break;
            }
        case Type::IntegerTyID:
            {
                switch(structTy->getElementType(i)->getPrimitiveSizeInBits())
                {
                case 8:
                    {
                        ElemDesc.SetSubTypeDesc(i,TCHAR);
                        break;
                    }
                case 16:
                    {
                        ElemDesc.SetSubTypeDesc(i,TSHORT);
                        break;
                    }
                case 32:
                    {
                        ElemDesc.SetSubTypeDesc(i,TINT);
                        break;
                    }
                case 64:
                    {
                        ElemDesc.SetSubTypeDesc(i,TLONG);
                        break;
                    }
                default:
                    {
                        throw Exception::ParserBadTypeException(
                            "[OpenCLKernelArgumentsParser::forParserStruct]bad type of integer in vector");
                        break;
                    }
                }
                break;
            }
        case Type::VectorTyID:
            {
                const VectorType *vectorTy = cast<VectorType>(structTy->getElementType(i));
                std::size_t numOfElements = vectorTy->getNumElements();
                TypeDesc SubElemDesc(TVECTOR);
                SubElemDesc.SetNumberOfElements(numOfElements);
                switch ( vectorTy->getElementType()->getTypeID() )
                {
                case Type::FloatTyID:
                    {
                        SubElemDesc.SetSubTypeDesc(0,TFLOAT);
                        break;
                    }
                case Type::DoubleTyID:
                    {
                        SubElemDesc.SetSubTypeDesc(0,TDOUBLE);
                        break;
                    }
                case Type::IntegerTyID:
                    {
                        switch( vectorTy->getElementType()->getPrimitiveSizeInBits())
                        {
                        case 8:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TCHAR);
                                break;
                            }
                        case 16:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TSHORT);
                                break;
                            }
                        case 32:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TINT);
                                break;
                            }
                        case 64:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TLONG);
                                break;
                            }
                        default:
                            {
                                throw Exception::ParserBadTypeException(
                                    "[OpenCLKernelArgumentsParser::forParserStruct]bad type of integer in vector in struct");
                                break;
                            }
                        }
                        break;
                    }
                default:
                    {
                        throw Exception::ParserBadTypeException(
                            "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector in struct");
                        break;
                    }
                }
                ElemDesc.SetSubTypeDesc(i,SubElemDesc);
                break;
            }
        case Type::PointerTyID:
            {            
                TypeDesc SubElemDesc(TPOINTER);
                const PointerType *ptr = cast<PointerType>(structTy->getElementType(i));
                switch(ptr->getElementType()->getTypeID())
                {
                case Type::FloatTyID:
                    {
                        SubElemDesc.SetSubTypeDesc(0,TFLOAT);
                        break;
                    }
                case Type::DoubleTyID:
                    {
                        SubElemDesc.SetSubTypeDesc(0,TDOUBLE);
                        break;
                    }
                case Type::IntegerTyID:
                    {
                        switch(ptr->getElementType()->getPrimitiveSizeInBits())
                        {
                        case 8:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TCHAR);
                                break;
                            }
                        case 16:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TSHORT);
                                break;
                            }
                        case 32:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TINT);
                                break;
                            }
                        case 64:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TLONG);
                                break;
                            }
                        default:
                            {
                                throw Exception::ParserBadTypeException(
                                    "[OpenCLKernelArgumentsParser::forParserStruct]bad type of integer in pointer");
                                break;
                            }
                        }
                        break;
                    }


                case Type::VectorTyID:
                    {
                        const VectorType *vectorTy = cast<VectorType>(ptr->getElementType());
                        std::size_t numOfElements = vectorTy->getNumElements();
                        TypeDesc VectorSubElemDesc(TVECTOR);
                        VectorSubElemDesc.SetNumberOfElements(numOfElements);
                        switch ( vectorTy->getElementType()->getTypeID() )
                        {
                        case Type::FloatTyID:
                            {
                                VectorSubElemDesc.SetSubTypeDesc(0,TFLOAT);
                                break;
                            }
                        case Type::DoubleTyID:
                            {
                                VectorSubElemDesc.SetSubTypeDesc(0,TDOUBLE);
                                break;
                            }
                        case Type::IntegerTyID:
                            {
                                switch( vectorTy->getElementType()->getPrimitiveSizeInBits())
                                {
                                case 8:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TCHAR);
                                        break;
                                    }
                                case 16:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TSHORT);
                                        break;
                                    }
                                case 32:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TINT);
                                        break;
                                    }
                                case 64:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TLONG);
                                        break;
                                    }
                                default:
                                    {
                                        throw Exception::ParserBadTypeException(
                                            "[OpenCLKernelArgumentsParser::forParserStruct]bad type of integer in vector in pointer");
                                        break;
                                    }
                                }
                                break;
                            }
                        default:
                            {
                                throw Exception::ParserBadTypeException(
                                    "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector in pointer");
                                break;
                            }
                        }
                        SubElemDesc.SetSubTypeDesc(0,VectorSubElemDesc);
                        break;
                    }


                case Type::StructTyID:
                    {
                        TypeDesc StructDesc=forParserStruct(cast<StructType>(ptr->getElementType()));
                        SubElemDesc.SetSubTypeDesc(0,StructDesc);
                        break;
                    }
                default:
                    {
                        throw Exception::ParserBadTypeException(
                            "[OpenCLKernelArgumentsParser::forParserStruct]bad type in pointer");
                        break;
                    }
                }
                ElemDesc.SetSubTypeDesc(i,SubElemDesc);
                break;
            }
        case Type::StructTyID:
            {
                TypeDesc StructDesc=forParserStruct(cast<StructType>(structTy->getElementType(i)));
                ElemDesc.SetSubTypeDesc(i,StructDesc);
                break;
            }
        case Type::ArrayTyID:
            {
                const ArrayType *arrayTy = cast<ArrayType>(structTy->getElementType(i));
                std::size_t numOfElements = arrayTy->getNumElements();
                TypeDesc SubElemDesc(TARRAY);
                SubElemDesc.SetNumberOfElements(numOfElements);
                switch ( arrayTy->getElementType()->getTypeID() )
                {
                case Type::FloatTyID:
                    {
                        SubElemDesc.SetSubTypeDesc(0,TFLOAT);
                        break;
                    }
                case Type::DoubleTyID:
                    {
                        SubElemDesc.SetSubTypeDesc(0,TDOUBLE);
                        break;
                    }
                case Type::IntegerTyID:
                    {
                        switch( arrayTy->getElementType()->getPrimitiveSizeInBits())
                        {
                        case 8:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TCHAR);
                                break;
                            }
                        case 16:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TSHORT);
                                break;
                            }
                        case 32:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TINT);
                                break;
                            }
                        case 64:
                            {
                                SubElemDesc.SetSubTypeDesc(0,TLONG);
                                break;
                            }
                        default:
                            {
                                throw Exception::ParserBadTypeException(
                                    "[OpenCLKernelArgumentsParser::forParserStruct]bad type of integer in vector in pointer");
                                break;
                            }
                        }
                        break;
                    }
                case Type::VectorTyID:
                    {
                        const VectorType *vectorTy = cast<VectorType>(arrayTy->getElementType());
                        std::size_t numOfElements = vectorTy->getNumElements();
                        TypeDesc VectorSubElemDesc(TVECTOR);
                        VectorSubElemDesc.SetNumberOfElements(numOfElements);
                        switch ( vectorTy->getElementType()->getTypeID() )
                        {
                        case Type::FloatTyID:
                            {
                                VectorSubElemDesc.SetSubTypeDesc(0,TFLOAT);
                                break;
                            }
                        case Type::DoubleTyID:
                            {
                                VectorSubElemDesc.SetSubTypeDesc(0,TDOUBLE);
                                break;
                            }
                        case Type::IntegerTyID:
                            {
                                switch( vectorTy->getElementType()->getPrimitiveSizeInBits())
                                {
                                case 8:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TCHAR);
                                        break;
                                    }
                                case 16:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TSHORT);
                                        break;
                                    }
                                case 32:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TINT);
                                        break;
                                    }
                                case 64:
                                    {
                                        VectorSubElemDesc.SetSubTypeDesc(0,TLONG);
                                        break;
                                    }
                                default:
                                    {
                                        throw Exception::ParserBadTypeException(
                                            "[OpenCLKernelArgumentsParser::forParserStruct]bad type of integer in vector in pointer");
                                        break;
                                    }
                                }
                                break;
                            }
                        default:
                            {
                                throw Exception::ParserBadTypeException(
                                    "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector in pointer");
                                break;
                            }
                            break;
                        }
                        SubElemDesc.SetSubTypeDesc(0,VectorSubElemDesc);
                        break;
                    }

                default:
                    {
                        throw Exception::ParserBadTypeException(
                            "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector in pointer");
                        break;
                    }
                }
                ElemDesc.SetSubTypeDesc(i,SubElemDesc);
                break;
            }
        default:
            {
                throw Exception::ParserBadTypeException(
                    "[OpenCLKernelArgumentsParser::forParserStruct]bad type in vector");
                break;
            }
        }
    }
    return ElemDesc;
}


OCLKernelArgumentsList OpenCLKernelArgumentsParser::KernelArgumentsParser(const std::string& kernelName, const llvm::Module* programObject) {
    Function*         m_pKernel;        // kernel to run
    OCLKernelArgumentsList ListOfArguments;
    // Extract 'kernel' function from program
    NamedMDNode* metadata = programObject->getNamedMetadata("opencl.kernels");

    for (uint32_t k = 0, e = metadata->getNumOperands(); k != e; ++k)
    {
        // Obtain kernel function from annotation
        MDNode *elt = metadata->getOperand(k);

        Constant * globVal = mdconst::extract<Function>(elt->getOperand(0));
        m_pKernel = cast<Function>(globVal->stripPointerCasts());
        if ( m_pKernel->getName().str() != kernelName)
        {
            continue;
        }

        Function::arg_iterator arg_it;
        for (arg_it= m_pKernel->arg_begin(); arg_it != m_pKernel->arg_end(); arg_it++)
        {
            switch(arg_it->getType()->getTypeID())
            {
            case Type::FloatTyID:
                {
                    TypeDesc ElemDesc(TFLOAT);
                    BufferDesc BufDesc;
                    BufDesc.SetElementDecs(ElemDesc);
                    BufDesc.SetNumOfElements(1);
                    ListOfArguments.push_back(BufDesc);
                    break;
                } 
            case Type::DoubleTyID:
                {
                    TypeDesc ElemDesc(TDOUBLE);
                    BufferDesc BufDesc;
                    BufDesc.SetElementDecs(ElemDesc);
                    BufDesc.SetNumOfElements(1);
                    ListOfArguments.push_back(BufDesc);
                    break;
                }
            case Type::IntegerTyID:
                {
                    TypeDesc ElemDesc;
                    switch(arg_it->getType()->getPrimitiveSizeInBits())
                    {
                    case 8:
                        {
                            ElemDesc=TypeDesc(TCHAR);
                            break;
                        }
                    case 16:
                        {
                            ElemDesc=TypeDesc(TSHORT);
                            break;
                        }
                    case 32:
                        {
                            ElemDesc=TypeDesc(TINT);
                            break;
                        }
                    case 64:
                        {
                            ElemDesc=TypeDesc(TLONG);
                            break;
                        }
                    default:
                        {
                            throw Exception::ParserBadTypeException(
                                "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type of integer in vector");
                            break;
                        }
                    }
                    BufferDesc BufDesc;
                    BufDesc.SetElementDecs(ElemDesc);
                    BufDesc.SetNumOfElements(1);
                    ListOfArguments.push_back(BufDesc);
                    break;
                }
            case Type::VectorTyID:
                {
                    std::size_t numOfElements =
                        dyn_cast<VectorType>(arg_it->getType())->getNumElements();
                    TypeDesc ElemDesc(TVECTOR);
                    ElemDesc.SetNumberOfElements(numOfElements);
                    switch ( dyn_cast<VectorType> (
                        arg_it->getType())->getElementType()->getTypeID() )
                    {
                    case Type::FloatTyID:
                        {
                            ElemDesc.SetSubTypeDesc(0,TFLOAT);
                            break;
                        }
                    case Type::DoubleTyID:
                        {
                            ElemDesc.SetSubTypeDesc(0,TDOUBLE);
                            break;
                        }
                    case Type::IntegerTyID:
                        {
                            switch(dyn_cast<VectorType> (
                                arg_it->getType())->getElementType()->getPrimitiveSizeInBits())
                            {
                            case 8:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TCHAR);
                                    break;
                                }
                            case 16:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TSHORT);
                                    break;
                                }
                            case 32:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TINT);
                                    break;
                                }
                            case 64:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TLONG);
                                    break;
                                }
                            default:
                                {
                                    throw Exception::ParserBadTypeException(
                                        "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type of integer in vector");
                                    break;
                                }
                            }
                            break;
                        }
                    default:
                        {
                            throw Exception::ParserBadTypeException(
                                "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type in vector");
                            break;
                        }
                    }
                    BufferDesc BufDesc;
                    BufDesc.SetElementDecs(ElemDesc);
                    BufDesc.SetNumOfElements(1);
                    ListOfArguments.push_back(BufDesc);
                    break;
                }
            case Type::PointerTyID:
                {
                    TypeDesc ElemDesc(TPOINTER);
                    const PointerType *ptr = cast<PointerType>(arg_it->getType());
                    switch(ptr->getElementType()->getTypeID())
                    {
                    case Type::FloatTyID:
                        {
                            ElemDesc.SetSubTypeDesc(0,TFLOAT);
                            break;
                        }
                    case Type::DoubleTyID:
                        {
                            ElemDesc.SetSubTypeDesc(0,TDOUBLE);
                            break;
                        }
                    case Type::IntegerTyID:
                        {
                            switch(ptr->getElementType()->getPrimitiveSizeInBits())
                            {
                            case 8:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TCHAR);
                                    break;
                                }
                            case 16:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TSHORT);
                                    break;
                                }
                            case 32:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TINT);
                                    break;
                                }
                            case 64:
                                {
                                    ElemDesc.SetSubTypeDesc(0,TLONG);
                                    break;
                                }
                            default:
                                {
                                    throw Exception::ParserBadTypeException(
                                        "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type of integer in pointer");
                                    break;
                                }
                            }
                            break;
                        }


                    case Type::VectorTyID:
                        {
                            const VectorType *vectorTy = cast<VectorType>(ptr->getElementType());
                            std::size_t numOfElements = vectorTy->getNumElements();
                            TypeDesc SubElemDesc(TVECTOR);
                            SubElemDesc.SetNumberOfElements(numOfElements);
                            switch ( vectorTy->getElementType()->getTypeID() )
                            {
                            case Type::FloatTyID:
                                {
                                    SubElemDesc.SetSubTypeDesc(0,TFLOAT);
                                    break;
                                }
                            case Type::DoubleTyID:
                                {
                                    SubElemDesc.SetSubTypeDesc(0,TDOUBLE);
                                    break;
                                }
                            case Type::IntegerTyID:
                                {
                                    switch( vectorTy->getElementType()->getPrimitiveSizeInBits())
                                    {
                                    case 8:
                                        {
                                            SubElemDesc.SetSubTypeDesc(0,TCHAR);
                                            break;
                                        }
                                    case 16:
                                        {
                                            SubElemDesc.SetSubTypeDesc(0,TSHORT);
                                            break;
                                        }
                                    case 32:
                                        {
                                            SubElemDesc.SetSubTypeDesc(0,TINT);
                                            break;
                                        }
                                    case 64:
                                        {
                                            SubElemDesc.SetSubTypeDesc(0,TLONG);
                                            break;
                                        }
                                    default:
                                        {
                                            throw Exception::ParserBadTypeException(
                                                "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type of integer in vector in pointer");
                                            break;
                                        }
                                    }
                                    break;
                                }
                            default:
                                {
                                    throw Exception::ParserBadTypeException(
                                        "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type in vector in pointer");
                                    break;
                                }
                            }
                            ElemDesc.SetSubTypeDesc(0,SubElemDesc);
                            break;
                        }
                    case Type::StructTyID:
                        {
                            TypeDesc SubElemDesc=forParserStruct(cast<StructType>(ptr->getElementType()));
                            ElemDesc.SetSubTypeDesc(0,SubElemDesc);
                            break;
                        }
                    case Type::PointerTyID: //pointer to pointer
                        {
                            throw Exception::ParserBadTypeException(
                                "[OpenCLKernelArgumentsParser::KernelArgumentsParser]pointer to pointer");
                            break;
                        }
                    default:
                        {
                            throw Exception::ParserBadTypeException(
                                "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type in pointer");
                            break;
                        }
                    }

                    StructType *ST = dyn_cast<StructType>(ptr->getElementType());
                    if(ST && !ST->isLiteral()) {
                        const std::string &imgArg = ST->getName().str();
                        if ( std::string::npos != imgArg.find("opencl.image"))    // Image identifier was found
                        {
                            // Get dimension image type
                            if(imgArg.find("opencl.image1d_t")              !=  std::string::npos ||
                                imgArg.find("opencl.image1d_array_t")       !=  std::string::npos ||
                                imgArg.find("opencl.image1d_buffer_t")      !=  std::string::npos ||
                                imgArg.find("opencl.image2d_t")             !=  std::string::npos ||
                                imgArg.find("opencl.image2d_depth_t")       !=  std::string::npos ||
                                imgArg.find("opencl.image2d_array_t")       !=  std::string::npos ||
                                imgArg.find("opencl.image2d_array_depth_t") !=  std::string::npos ||
                                imgArg.find("opencl.image3d_t")             !=  std::string::npos)
                            {
                                ImageDesc ImgDesc;
                                ListOfArguments.push_back(ImgDesc);
                            }
                            else
                            {
                                throw Exception::InvalidArgument("[OpenCLKernelArgumentsParser] Unknown image type");
                            }
                        }
                        else {
                            BufferDesc BufDesc;
                            BufDesc.SetElementDecs(ElemDesc);
                            BufDesc.SetNumOfElements(1); // one pointer to 0 objects of type TYPE
                            // in BUFFER desc. thats not pointer desc
                            ListOfArguments.push_back(BufDesc);
                        }
                    }
                    else {
                        BufferDesc BufDesc;
                        BufDesc.SetElementDecs(ElemDesc);
                        BufDesc.SetNumOfElements(1); // one pointer to 0 objects of type TYPE
                        // in BUFFER desc. thats not pointer desc
                        ListOfArguments.push_back(BufDesc);
                    }
                    break;
                }
            case Type::StructTyID:
                {
                    TypeDesc ElemDesc=forParserStruct(cast<StructType>(arg_it->getType()));
                    BufferDesc BufDesc;
                    BufDesc.SetElementDecs(ElemDesc);
                    BufDesc.SetNumOfElements(1);
                    ListOfArguments.push_back(BufDesc);
                    break;
                }
            default:
                {
                    throw Exception::ParserBadTypeException(
                        "[OpenCLKernelArgumentsParser::KernelArgumentsParser]bad type");
                    break;
                }
            }
        }
    }
    return ListOfArguments;
}

OCLKernelArgumentsList OpenCLKernelArgumentsParser::KernelArgHeuristics(
    const OCLKernelArgumentsList &Args, const size_t* globalworksize, const uint64_t dim){
        OCLKernelArgumentsList result; //result
        BufferDesc* head; //ptr to each buffer in OCLKernelArgumentsList
        //each buffer is tree head
        uint64_t def_size = 1;//default number of pointed elems for each ptr
        uint64_t i;

        for(i = 0; i < dim; ++i)
            def_size *= globalworksize[i]; //calculate default size

        for(i = 0; i < Args.size(); ++i){
            //result.size() trees
            if(Args[i].get()->GetName() != BufferDesc::GetBufferDescName())
            {
                result.push_back(*Args[i].get());
                continue;
            }
            head = static_cast<BufferDesc*>(Args[i].get()); //set up new head
            BufferDesc Buffd;
            //replace number of pointed elements
            Buffd.SetElementDecs(RecursiveDFS((*head).GetElementDescription(), def_size)); 
            Buffd.SetNumOfElements(head->NumOfElements());//number elements in the buffer 
            //is the same
            result.push_back(Buffd);//add new buff desc to result
        }
        return result;
}
