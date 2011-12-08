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

File Name:  OpenCLProgramConfiguration.h

\*****************************************************************************/
#pragma once

#include "IProgramConfiguration.h"
#include "mem_utils.h"

#include "CL/cl.h"
#define TIXML_USE_STL
#include "tinyxml.h"
#include <list>

namespace Validation
{
    enum DataFileType
    {
        Binary,
        Xml
    };

    /// @brief enum for OpenCL program file type
    enum ProgramFileType
    {
        CL,
        LL,
        BC
    };

    /// @brief This class contain OpenCL kernel specific configuration
    class OpenCLKernelConfiguration: public TiXmlVisitor
    {
    public:
        /// @brief Constructor
        OpenCLKernelConfiguration(const TiXmlElement& root, const std::string& baseDirectory);

        /// @brief Returns work dimension (possible values: 1, 2, 3)
        /// @return Work dimension
        // More details about work dimension at end of file
        cl_uint GetWorkDimension() const
        {
            return m_workDimension;
        }

        /// @brief Returns global work offset
        /// @return Global work offset
        // More details about global work offset at end of file
        size_t * GetGlobalWorkOffset() const
        {
            return m_pGlobalWorkOffeset;
        }

        /// @brief Returns global work size
        /// @return Global work size
        // More details about global work size at end of file
        size_t * GetGlobalWorkSize() const
        {
            return m_pGlobalWorkSize;
        }

        /// @brief Returns local work size
        /// @return Local work size
        // More details about local work size at end of file
        size_t * GetLocalWorkSize() const
        {
            return m_pLocalWorkSize;
        }

        /// @brief Returns name of kernel to run.
        /// @return Kernel name
        const std::string& GetKernelName() const
        {
            return m_kernelName;
        }

        /// @brief Returns the type of the input file
        DataFileType GetInputFileType() const
        {
            return m_inputFileType;
        }

        /// @brief Returns the input file path
        const std::string& GetInputFilePath() const
        {
            return m_inputFilePath;
        }

        /// @brief Returns the type of the reference file
        DataFileType GetReferenceFileType() const
        {
            return m_referenceFileType;
        }

        /// @brief Returns the input file path
        const std::string& GetReferenceFilePath() const
        {
            return m_referenceFilePath;
        }

        /// @brief Returns the type of the reference file
        DataFileType GetNeatFileType() const
        {
            return m_neatFileType;
        }

        /// @brief Returns the input file path
        const std::string& GetNeatFilePath() const
        {
            return m_neatFilePath;
        }

    private:
        bool VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute);
        DataFileType GetDataFileType(const std::string& strFileType);

    private:
        // Global work offset
        // More details about global work offset at end of file
        size_t * m_pGlobalWorkOffeset;

        // Global work Size
        // More details about global work size at end of file
        size_t * m_pGlobalWorkSize;

        // Local work size
        // More details about local work size at end of file
        size_t * m_pLocalWorkSize;

        // Work dimension
        // More details about work dimension at end of file
        cl_uint m_workDimension;

        // type of the input file
        DataFileType m_inputFileType;

        // path to the input file
        std::string m_inputFilePath;

        // Name of kernel to run.
        std::string m_kernelName;

        // type of the reference file
        DataFileType m_referenceFileType;

        // path to the reference file
        std::string m_referenceFilePath;

        // type of the neat file
        DataFileType m_neatFileType;

        // path to the neat file
        std::string m_neatFilePath;

        // base directory
        std::string m_baseDirectory;
    };

    /// @brief This class contain OpenCL Include Directories
    class OpenCLIncludeDirs: public TiXmlVisitor
    {
    public:
        typedef std::list<std::string> IncludeDirsList;

        /// @brief Constructor
        OpenCLIncludeDirs(const TiXmlElement& root, const std::string& baseDirectory);

        /// @brief Return the const iterator to the include directories list start
        /// @return iterator
        IncludeDirsList::const_iterator beginIncldueDirs() const
        {
            return m_includeDirsList.begin();
        }

        /// @brief Return the const iterator to the include directories list end
        /// @return iterator
        IncludeDirsList::const_iterator endIncludeDirs() const
        {
            return m_includeDirsList.end();
        }
    private:
        bool VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute);

    private:
        // List of include directories for OpenCL
        IncludeDirsList m_includeDirsList;

        // base directory
        std::string m_baseDirectory;
    };

    /// @brief This class contain OpenCL test program information
    class OpenCLProgramConfiguration : public IProgramConfiguration, public TiXmlVisitor
    {
    public:
        typedef std::list<OpenCLKernelConfiguration*> KernelConfigList;

        /// @brief Constructor
        /// @param [IN] configFile Name of OpenCL test run configuration file
        /// @param [IN] baseDir. Base directory used for input/output file lookup
        OpenCLProgramConfiguration(const std::string& configFile, const std::string& baseDir);
        ~OpenCLProgramConfiguration();

        /// @brief Returns the program file path
        std::string GetProgramFilePath() const
        {
            return m_programFilePath;
        }

        /// @brief Returns the type of the program file
        ProgramFileType GetProgramFileType() const
        {
            return m_programFileType;
        }

        std::string GetProgramName() const
        {
            return m_programName;
        }

        /// @brief Returns indicator whether to use vectorizer
        /// @return True if vectorizer should be used, false otherwise.
        bool GetUseVectorizer() const
        {
            return m_useVectorizer;
        }

        /// @brief Return the const iterator to kernel configurations list start
        /// @return iterator
        KernelConfigList::const_iterator beginKernels() const
        {
            return m_kernels.begin();
        }

        /// @brief Return the const iterator to kernel configurations list end
        /// @return iterator
        KernelConfigList::const_iterator endKernels() const
        {
            return m_kernels.end();
        }

        /// @brief Returns OpenCLIncludeDirs
        OpenCLIncludeDirs* GetIncludeDirs() const
        {
            return m_includeDirs.get();
        }

        /// @brief Return number of kernel configurations to run.
        size_t numberOfKenelConfigurations() const
        {
            return m_kernels.size();
        }

        /// @brief Returns the base directory
        std::string GetBaseDirectory() const
        {
            return m_baseDirectory;
        }

    private:
        bool VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute);
        ProgramFileType GetProgramFileType(const std::string& strFileType);

    private:
        // Indicator whether to use vectorizer
        bool m_useVectorizer;

        // Program file path
        std::string m_programFilePath;

        // Program file type
        ProgramFileType m_programFileType;

        // List of kernel specific configurations
        KernelConfigList m_kernels;

        // Include directories for generating IR from a CL file
        auto_ptr_ex<OpenCLIncludeDirs> m_includeDirs;

        // configuration file absolute path
        std::string m_configFile;

        // base directory path - used to lookup the input/output files if specified as relative path
        std::string m_baseDirectory;

        // program name
        std::string m_programName;

        // program configuration file format
        enum PROGRAM_CONFIG_FILE_FORMAT {
            UNKNOWN,
            BYTECODE_ONLY,  // config file supports only byte code file type for test program
            CL_LL_BC        // config file supports cl, llvm test and llvm byte code file type for test program
        };
        PROGRAM_CONFIG_FILE_FORMAT m_format;
  };

        // For 2 dimensions:
        // If you divide a picture into smaller squares, say 1024 pixels each
        //   ___________________________
        //  |                           |
        //  |                           |
        //  |                           |
        //  |        ________           |
        //  |       |        |          |
        //  |       |        |          |
        //  |       |________|          |
        //  |                           |
        //  |                           |
        //  |___________________________|

        //then:
        //m_workDimension = 2
        //m_pGlobalWorkOffeset[0] = the id of the square on X radix
        //m_pGlobalWorkOffeset[1] = the id of the square on Y radix
        //m_pGlobalWorkSize[0] = total number of squares on X radix
        //m_pGlobalWorkSize[1] = total number of squares on Y radix
        //m_pLocalWorkSize[0] = size of square on X radix
        //m_pLocalWorkSize[1] = size of square on Y radix
}

