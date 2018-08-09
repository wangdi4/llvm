// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"
#include "OpenCLProgramConfiguration.h"
#include "SATestException.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "OpenCLProgramConfiguration"
#include "llvm/Support/Debug.h"

#define TIXML_USE_STL
#include "tinyxml.h"

#include <sstream>
#include <string>
#include <stdlib.h>

using namespace std;

namespace Validation{

namespace Utils
{
    string GetDataFilePath(const string& fileName, const string& baseDirectory)
    {
        using namespace llvm;
        if( !sys::path::is_absolute(fileName) && !baseDirectory.empty())
        {
            SmallString<128> absFilePath(baseDirectory);
            sys::path::append(absFilePath, fileName);
            if(error_code EC = llvm::sys::fs::make_absolute(absFilePath))
                throw Exception::IOError("GetDataFilePath::nonexistent path created with \
                                         fileName=" + fileName +
                                         " and baseDirectory=" +  baseDirectory);
            sys::path::native(absFilePath);
            return absFilePath.str();
        }

        SmallString<128> fName(fileName);
        sys::fs::make_absolute(fName);
        return fileName;
    }
}

OpenCLKernelConfiguration::OpenCLKernelConfiguration(const TiXmlElement& root, const string& baseDirectory ):
        m_workDimension(0),
        m_inputFileType(Binary),
        m_baseDirectory(baseDirectory),
        m_generatorConfig(0)
{
    root.QueryStringAttribute( "Name", &m_kernelName);   

    stringstream value(root.FirstChildElement("WorkDimention")->GetText());
    value >> m_workDimension;
    for( int i=0 ; i<MAX_WORK_DIM ; i++)
        m_arrGlobalWorkOffset[i] = m_arrLocalWorkSize[i] = m_arrGlobalWorkSize[i] = 0;
    if ( m_workDimension < 1 || m_workDimension > 3)
        throw Exception::InvalidArgument("Work dimension is not one of: 1, 2, 3");
    root.Accept(this);
}

bool OpenCLKernelConfiguration::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute)
{
    if( element.ValueStr() == "LocalWorkSize" )
    {
        stringstream value(element.GetText());

        for (unsigned i = 0; i < m_workDimension; ++i)
        {
            value >> m_arrLocalWorkSize[i];
        }
    }

    if( element.ValueStr() == "GlobalWorkSize" )
    {
        stringstream value(element.GetText());

        for (unsigned i = 0; i < m_workDimension; ++i)
        {
            value >> m_arrGlobalWorkSize[i];
        }
    }

    if( element.ValueStr() == "GlobalWorkOffset" )
    {
        stringstream value(element.GetText());

        for (unsigned i = 0; i < m_workDimension; ++i)
        {
            value >> m_arrGlobalWorkOffset[i];
        }
    }

    if( element.ValueStr() == "InputDataFileType" )
    {
        m_inputFileType  = GetDataFileType(element.GetText());
    }

    if( element.ValueStr() == "InputDataFile" )
    {
        m_inputFilePath = Utils::GetDataFilePath(element.GetText(), m_baseDirectory);
    }

    if( element.ValueStr() == "ReferenceDataFileType" )
    {
        m_referenceFileType  = GetDataFileType(element.GetText());
    }

    if( element.ValueStr() == "ReferenceDataFile" )
    {
        m_referenceFilePath = Utils::GetDataFilePath(element.GetText(), m_baseDirectory);
    }

    if( element.ValueStr() == "NeatDataFileType" )
    {
        m_neatFileType  = GetDataFileType(element.GetText());
    }

    if( element.ValueStr() == "NeatDataFile" )
    {
        m_neatFilePath = Utils::GetDataFilePath(element.GetText(), m_baseDirectory);
    }
    if( element.ValueStr() == "OCLKernelDataGeneratorConfig")
    {
        m_generatorConfig = new OCLKernelDataGeneratorConfig(&element);
    }
    return true;
}

DataFileType OpenCLKernelConfiguration::GetDataFileType(const string& strFileType)
{
    if( strFileType == "binary" )
        return Binary;

    if( strFileType == "xml" )
        return Xml;

    if( strFileType == "random" )
        return Random;

    if( strFileType == "config" )
        return Config;

    throw Exception::InvalidArgument("[OpenCLKernelConfiguration]"+strFileType+" is unsupported file type.");
}

OpenCLIncludeDirs::OpenCLIncludeDirs(const TiXmlElement& root, const string& baseDirectory ):
    m_baseDirectory(baseDirectory)
{
    root.Accept(this);
}

bool OpenCLIncludeDirs::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute)
{
    if( element.ValueStr() == "IncludeDir" )
    {
        m_includeDirsList.push_back( Utils::GetDataFilePath(element.GetText(), m_baseDirectory) );
    }

    return true;
}

OpenCLProgramConfiguration::OpenCLProgramConfiguration(const string& configFile, const string& baseDir):
        m_useVectorizer(false),
        m_programFileType(BC),
        m_includeDirs(NULL),
        m_format(UNKNOWN)
{
    llvm::SmallString<128> configPath(configFile);
    if( !llvm::sys::path::is_absolute(configFile) )
    {
        llvm::sys::fs::make_absolute(configPath);
        if(!llvm::sys::fs::exists(configPath.str())) {
            throw Exception::IOError("Configuration file " + m_configFile + " doesn't exist");
        }
        llvm::sys::path::native(configPath);
    }

    m_programName   = llvm::sys::path::stem(llvm::StringRef(configPath));
    m_configFile    = configPath.c_str();
    m_baseDirectory = baseDir.empty() ? llvm::sys::path::parent_path(llvm::StringRef(configPath)).str()
                                      : baseDir;


    TiXmlDocument config(m_configFile);
    if (config.LoadFile())
    {
        config.Accept(this);
    }
    else
    {
        std::string errDesc(config.ErrorDesc());

        int errRowNum = config.ErrorRow();
        if ( errRowNum != 0) {
            std::string servStr;
            std::stringstream ss;
            errDesc.append(" at the row ");
            ss << errRowNum;
            ss >> servStr;
            errDesc.append(servStr);
        }

        throw Exception::IOError("Configuration file " + m_configFile + " can't be loaded : " + errDesc);
    }
}

OpenCLProgramConfiguration::~OpenCLProgramConfiguration()
{
    for (KernelConfigList::iterator it = m_kernels.begin(); it != m_kernels.end(); ++it)
    {
        delete *it;
    }
}

ProgramFileType OpenCLProgramConfiguration::GetProgramFileType(const string& strFileType)
{
    if( strFileType == "CL" )
        return CL;

    if( strFileType == "LL" )
        return LL;

    if( strFileType == "BC" )
        return BC;

    throw Exception::InvalidArgument("program file types other than CL, LL or BC are not supported");
}

bool OpenCLProgramConfiguration::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute)
{
    if( element.ValueStr() == "KernelConfiguration" )
    {
        m_kernels.push_back( new OpenCLKernelConfiguration( element, m_baseDirectory ));
    }
    else if( element.ValueStr() == "IncludeDirs" )
    {
        m_includeDirs.reset( new OpenCLIncludeDirs( element, m_baseDirectory ) );
    }

    // This code added to support old format of configuration file which supported only LLVM byte code test programs.
    // Only one instance of ByteCodeFile or ProgramFile XML node is allowed in configuration file.
    else if( element.ValueStr() == "ByteCodeFile" )
    {
        if (UNKNOWN != m_format)
        {
            throw Exception::InvalidArgument("Configuration file error: test program was already set.");
        }
        m_programFilePath = Utils::GetDataFilePath(element.GetText(), m_baseDirectory);
        m_programFileType  = BC;
        m_format = BYTECODE_ONLY;
    }

    // ProgramFile and ProgramFileType usages are not allowed with ByteCodeFile tag in one configuration file.
    else if( element.ValueStr() == "ProgramFile" )
    {
        if ((UNKNOWN != m_format) && (CL_LL_BC == m_format))
        {
            throw Exception::InvalidArgument("Configuration file error: test program was already set.");
        }
        else if ( NULL == element.GetText() )
        {
            std::stringstream stringStream;
            stringStream << "Configuration file error(";
            stringStream << "row " << element.Row() << " column "  << element.Column() << "):";
            stringStream << " no program file has been given!";
            throw Exception::InvalidArgument( stringStream.str() );
        }
        m_programFilePath = Utils::GetDataFilePath(element.GetText(), m_baseDirectory);
        const char* compilationFlags = element.Attribute("compilation_flags");
        if ( compilationFlags )
            m_compilationFlags = compilationFlags;
        m_format = CL_LL_BC;
    }

    else if( element.ValueStr() == "ProgramFileType")
    {
        if ((UNKNOWN != m_format) && (CL_LL_BC != m_format))
        {
            throw Exception::InvalidArgument("Configuration file error: ProgramFileType XML node is valid only with ProgramFile XML node.");
        }
        m_programFileType  = GetProgramFileType(element.GetText());
        m_format = CL_LL_BC;
    }

    else if( element.ValueStr() == "UseVectorizer" )
    {
        stringstream value(element.GetText());
        value >> m_useVectorizer;
    }

    else if( element.ValueStr() == "InjectObject" )
    {
        m_injectedObjectPath = Utils::GetDataFilePath(element.GetText(), m_baseDirectory);
    }

    return true;
}
} // namespace Validation
