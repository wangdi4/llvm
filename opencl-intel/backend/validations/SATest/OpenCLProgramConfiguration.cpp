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

File Name:  OpenCLProgramConfiguration.cpp

\*****************************************************************************/

#include "llvm/System/Path.h"
#include "OpenCLProgramConfiguration.h"
#include "SATestException.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "OpenCLProgramConfiguration"
#include "llvm/Support/Debug.h"

#define TIXML_USE_STL
#include "tinyxml.h"

#include <string>
#include <stdlib.h>

using namespace std;

namespace Validation{

namespace Utils
{
    string GetDataFilePath(const string& fileName, const string& baseDirectory)
    {
        llvm::sys::Path filePath(fileName.c_str(), fileName.size());

        if( !filePath.isAbsolute() && !baseDirectory.empty())
        {
            llvm::sys::Path absFilePath(baseDirectory.c_str(), baseDirectory.size());
            absFilePath.appendComponent(filePath.c_str());
            return absFilePath.str();
        }

        filePath.makeAbsolute();
        return filePath.str();
    }
}

OpenCLKernelConfiguration::OpenCLKernelConfiguration(const TiXmlElement& root, const string& baseDirectory ):
        m_pGlobalWorkOffeset(NULL),
        m_pGlobalWorkSize(NULL),
        m_pLocalWorkSize(NULL),
        m_workDimension(0),
        m_inputFileType(Binary),
        m_baseDirectory(baseDirectory)
{
    root.QueryStringAttribute( "Name", &m_kernelName);   

    stringstream value(root.FirstChildElement("WorkDimention")->GetText());
    value >> m_workDimension;

    if (1 <= m_workDimension && m_workDimension <= 3)
    {
        m_pLocalWorkSize = new size_t[m_workDimension];
        m_pGlobalWorkSize = new size_t[m_workDimension];
        m_pGlobalWorkOffeset = new size_t[m_workDimension];
    }
    else
    {
        throw Exception::InvalidArgument("Work dimension is not one of: 1, 2, 3");
    }

    root.Accept(this);
}

bool OpenCLKernelConfiguration::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute)
{
    if( element.ValueStr() == "LocalWorkSize" )
    {
        stringstream value(element.GetText());

        for (unsigned i = 0; i < m_workDimension; ++i)
        {
            value >> m_pLocalWorkSize[i];
        }
    }

    if( element.ValueStr() == "GlobalWorkSize" )
    {
        stringstream value(element.GetText());

        for (unsigned i = 0; i < m_workDimension; ++i)
        {
            value >> m_pGlobalWorkSize[i];
        }
    }

    if( element.ValueStr() == "GlobalWorkOffset" )
    {
        stringstream value(element.GetText());

        for (unsigned i = 0; i < m_workDimension; ++i)
        {
            value >> m_pGlobalWorkOffeset[i];
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

    return true;
}

DataFileType OpenCLKernelConfiguration::GetDataFileType(const string& strFileType)
{
    if( strFileType == "binary" )
        return Binary;

    if( strFileType == "xml" )
        return Xml;

    throw Exception::InvalidArgument("file types other than binary or XML are not supported");
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
        m_format(UNKNOWN)
{
    llvm::sys::Path configFilePath(configFile.c_str(), configFile.size());

    if(!configFilePath.isValid())
    {
        throw Exception::IOError("Configuration file name is invalid");
    }

    if( !configFilePath.isAbsolute() )
    {
        configFilePath.makeAbsolute();
        //llvm::Path::makeAbsolute bug workaround - forces to llvm::Path to flip backslashes
        configFilePath = llvm::sys::Path(configFilePath.str().c_str(), configFilePath.str().size());
    }
    m_programName   = configFilePath.getBasename();
    m_configFile    = configFilePath.c_str();
    m_baseDirectory = baseDir.empty() ? configFilePath.getDirname().str()
                                      : baseDir;

    TiXmlDocument config(m_configFile);
    if (config.LoadFile())
    {
        config.Accept(this);
    }
    else
    {
        throw Exception::IOError("Unable to load file with run configuration.");
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
        m_includeDirs = new OpenCLIncludeDirs( element, m_baseDirectory );
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
        m_programFilePath = Utils::GetDataFilePath(element.GetText(), m_baseDirectory);
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

    else // Unrecognized configuration option
    {
        DEBUG(llvm::dbgs()<<"[OpenCL program configuration] unrecognized option is found: " << element.ValueStr() << "\n");
    }

    return true;
}
} // namespace Validation