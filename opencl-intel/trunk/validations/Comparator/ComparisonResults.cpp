/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ComparisonResults.cpp

\*****************************************************************************/
#include "ComparisonResults.h"

using namespace Validation;

void ComparisonResults::AddMismatch(const MismatchedVal& in_Val)
{
    /// Check if mismatchedVal with this index has been added
    /// to mismatch values previously
    uint32_t bufN = in_Val.GetBufIdx();
    CompStatMap::iterator mit = m_statMap.find(bufN);
    // if not exist add with zero initialized
    if(mit == m_statMap.end())
    {
        CompStatistics cs(in_Val.GetDesc()); 
        cs.numMismatches = 0;
        mit = m_statMap.insert(std::pair<uint32_t, CompStatistics>(bufN, cs)).first;
    }

    // increase counter of mismatches
    (mit->second).numMismatches++;
    (mit->second).maxDiff = std::max(in_Val.ComputeDeviation(), mit->second.maxDiff);
    if((mit->second).numMismatches < MAX_MISMATCHES)
        mismatches.push_back(in_Val);

}

ComparisonResults::MismatchedVal ComparisonResults::GetMismatch(size_t index)
{
    return mismatches[index];
}

size_t ComparisonResults::GetMismatchCount()
{
    return mismatches.size();
}

void ComparisonResults::ReportDetail()
{
    if(!mismatches.empty())
    {
        std::cout<<"Detailed statistics:"<<std::endl;
        std::cout<<mismatches.size() <<" mismatches:" << std::endl;
        for(uint32_t i = 0; i<mismatches.size(); i++)
        {
            std::cout<<mismatches[i].ToString()<<std::endl;
        }
        std::cout<<"------"<<std::endl;
    }
    std::cout<< GetStatistics()->ToString() << std::endl;
}

void ComparisonResults::Clear()
{
    mismatches.clear();
}

void ComparisonResults::Report()
{
    if(!m_statMap.empty())
    {
        std::cout << "Kernel " << m_kernelName << "\n";
        std::cout << "BufferContainer[0] mismatch statistics\n";
        for(CompStatMap::iterator e = m_statMap.end(),
            it = m_statMap.begin();
            it!=e;++it)
        {
            const IMemoryObjectDesc *pDesc = (it->second.pDesc).get();
            
            if(BufferDesc::GetBufferDescName() == pDesc->GetName())
            {
                const BufferDesc* pBufDesc = static_cast<const BufferDesc*>(pDesc);
                std::string typeStr = pBufDesc->GetElementDescription().TypeToString();
                std::cout << "Mismatches in Buffer[" << it->first << "] : " << (it->second).numMismatches << std::endl;
                std::cout << "     Type: "<< typeStr << std::endl;
                std::cout << "     Length: " << pBufDesc->NumOfElements() << std::endl;
                std::cout << "     Maximal mismatch deviation: " << (it->second).maxDiff;
                bool isFloat;
                isFloat = pBufDesc->GetElementDescription().IsFloatingPoint();
                if(isFloat)
                {
                    std::cout<< " ulps";
                }
                std::cout<<std::endl;
            }
            else if(ImageDesc::GetImageDescName() == pDesc->GetName())
            {
                const ImageDesc* pImgDesc = static_cast<const ImageDesc*>(pDesc);
                const std::string typeStr = pImgDesc->DataTypeToString();
                const std::string orderStr = pImgDesc->OrderToString();
                std::cout << "Mismatches in Image[" << it->first << "] : " << (it->second).numMismatches << std::endl;
                std::cout << 
                    "     Datatype  " << pImgDesc->DataTypeToString() <<
                    " " << "Channel order " << pImgDesc->OrderToString() << std::endl << 
                    "     Sizes[WxHxD]: " << pImgDesc->GetSizes().width << 
                    " " << pImgDesc->GetSizes().height << 
                    " " << pImgDesc->GetSizes().depth << std::endl;
                std::cout << "     Maximal mismatch deviation: " << (it->second).maxDiff << std::endl;
            }
            else 
            {
                throw Exception::InvalidArgument("Not supported IMemObjectDesc");
            }

        }
    }
}

ComparisonResults::~ComparisonResults()
{
}
