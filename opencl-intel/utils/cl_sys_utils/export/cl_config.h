/////////////////////////////////////////////////////////////////////////
// cl_config.h
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel's prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel's suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////
#pragma once

#include "cl_types.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cl_env.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <fstream>
#endif

using std::string;
using std::vector;

namespace Intel { namespace OpenCL { namespace Utils {

	/**********************************************************************************************
	* Class name:	ConfigFile
	*
	* Description:	represents an ConfigFile object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class ConfigFile
	{
	protected:
		std::map<string,string> m_mapContents;  // extracted keys and values

		typedef std::map<string,string>::iterator mapi;
		typedef std::map<string,string>::const_iterator mapci;

		string m_sDelimiter;  // separator between key and value
		string m_sComment;    // separator between value and comments
		string m_sSentry;     // optional string to signal end of file

	public:
		/******************************************************************************************
		* Function: 	ConfigFile
		* Description:	The ConfigFile class constructor
		* Arguments:	sfilename [in]		local configuration file name
		*				sDelimiter [in]		the configuration parameter's delimiter - separator
		*									between key and value
		*				sComment [in]		configuration parameter's comments identifier -
		*									separator between value and comments
		*				sSentry [in]		represents end of file - optional string to signal end
		*									of file
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ConfigFile(const string& strFileName, string strDelimiter = "=", string strComment = "#", string strSentry = "EndConfigFile" );

		/******************************************************************************************
		* Function: 	ConfigFile
		* Description:	The ConfigFile class constructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ConfigFile();

		/******************************************************************************************
		* Function: 	Read
		* Description:	Search for key and read value or optional default value
		*				call as Read<T>(key, default_value)
		* Arguments:	const string& key [in]		reference to the string that represnts the key
		*				const T& value [in]			reference to the default value
		* Return value:	class T - the value which assign to the key
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		template<class T>
		T Read( const string& strKey, const T& value ) const;

		/******************************************************************************************
		* Function: 	ReadInto
		* Description:	Search for key and read value into variable
		*				call as ReadInto<T>(var, key)
		* Arguments:	T& var [in] 			refernce to the variable to which the value will
		*										be assigned
		*				const string& key [in]	reference to the string that represnts the key
		* Return value:	True -	if the value exists and was assign succesfully to the variable
		*				False - the value doesn't exists or can't assig value to the variable
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		template<class T>
		bool ReadInto( T& var, const string& key ) const;

		/******************************************************************************************
		* Function: 	ReadInto
		* Description:	Search for key and read value or optional default value into variable
		*				call as ReadInto<T>(var, key, default_value)
		* Arguments:	T& var-				refernce to the variable to which the value will
		*										be assigned
		*				const T& value [in]-	reference to the default value
		*				const string& key [in]-	reference to the string that represnts the key
		* Return value:	True -	if the value exists and was assign succesfully to the variable
		*				False - the value doesn't exists or can't assig value to the variable
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		template<class T>
		bool ReadInto( T& var, const string& key, const T& value ) const;

		/******************************************************************************************
		* Function: 	Add
		* Description:	Modify keys and values or add new configuration item
		*				call as Add<T>(key, value)
		* Arguments:	string key [in] -		represents the key of the configuration item
		*				const T& value [in] -	reference to the value of the configuration item
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		template<class T>
		void Add( string key, const T& value );

		/******************************************************************************************
		* Function: 	Remove
		* Description:	remove configuration item
		*				call as Remove(key)
		* Arguments:	const string& key [in] -	represents the key of the configuration item
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void Remove( const string& key );

		/******************************************************************************************
		* Function: 	KeyExists
		* Description:	Check whether key exists in configuration
		*				call as KeyExists(key)
		* Arguments:	const string& key [in] -	represents the key of the configuration item
		* Return value:	True -	the key exists in configuration
		*				False -	the key doesn't exist in configuration
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		bool KeyExists( const string& key ) const;

		/******************************************************************************************
		* Function: 	GetDelimiter
		* Description:	get delimiter of configuration syntax
		*				call as delimiter = GetDelimiter()
		* Arguments:
		* Return value:	delimiter of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		string GetDelimiter() const { return m_sDelimiter; }

		/******************************************************************************************
		* Function: 	GetComment
		* Description:	get comment of configuration syntax
		*				call as comment = GetComment()
		* Arguments:
		* Return value:	comment of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		string GetComment() const { return m_sComment; }

		/******************************************************************************************
		* Function: 	GetSentry
		* Description:	Get value of the sentry from the configuration syntax
		*				call as sentry = GetSentry()
		* Arguments:
		* Return value:	sentry of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		string GetSentry() const { return m_sSentry; }

		/******************************************************************************************
		* Function: 	SetDelimiter
		* Description:	set new delimiter value to the configuration syntax
		*				call as old_delimiter = SetDelimiter(new_delimiter)
		* Arguments:	const string& s [in] -	new delimiter
		* Return value:	previous value of delimiter of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		string SetDelimiter(const string& strDelimiter)
		{
			string strPrefDelimiter = m_sDelimiter;
			m_sDelimiter = strPrefDelimiter;
			return strPrefDelimiter;
		}

		/******************************************************************************************
		* Function: 	SetComment
		* Description:	set new comment value to the configuration syntax
		*				call as old_commens = SetComment(new_comment)
		* Arguments:	const string& s [in] -	new comment
		* Return value:	previous value of comment of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		string SetComment(const string& strComment)
		{
			string strOldComment = m_sComment;
			m_sComment = strComment;
			return strOldComment;
		}

		/******************************************************************************************
		* Function: 	ReadFile
		* Description:	read configuration file into ConfigFile object
		* Arguments:	string fileName [in] -	full path of configuration file
		*				ConfigFile& cf [in] -	reference to configuration file object
		* Return value:	CL_SUCCESS - file was read successfully
		*				CL_ERR_FILE_NOT_EXISTS - file name doesn't exists
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		static cl_err_code ReadFile(const string& fileName, ConfigFile& cf);

		/******************************************************************************************
		* Function: 	WriteFile
		* Description:	write ConfigFile object into configuration file
		* Arguments:	string fileName [in] -	full path of configuration file
		*				ConfigFile& cf [in] -	reference to configuration file object
		* Return value:	CL_SUCCESS - file was read successfully
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		static cl_err_code WriteFile(string fileName, ConfigFile& cf);

		/******************************************************************************************
		* Function: 	tokenize
		* Description:	create substrings vector from a tring sentence. using the ';' or ',' or '|'
		*				character as seperators
		* Arguments:	string sin [in] -				input string
		*				vector<string> & tokens [in] -	output substrings vector
		* Return value:	number of substrings in vector
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		static int tokenize(const string & sin, std::vector<string> & tokens);

		/******************************************************************************************
		* Function: 	T_as_string
		* Description:	convert class T to string
		* Arguments:	const T& t [in] -	class T (Type T must support << operator)
		* Return value:	string whihc represents the class
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		template<class T> static string ConvertTypeToString( const T& t );

		/******************************************************************************************
		* Function: 	string_as_T
		* Description:	convert string to class T (Type T must support << operator)
		* Arguments:	const string& s [in] -	input string
		* Return value:	class T
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		template<class T> static T ConvertStringToType( const string& str );

        /******************************************************************************************
		* Function: 	GetRegistryOrEtcValue
		* Description:	Get a value from registry on Windows (in key SOFTWARE\Intel\OpenCL) or from a file in /etc/OpenCL/vendors/Intel/
		* Arguments:	const string& name [in] - the name of the registry value or etc file
        *               const T& defaultVal     - default value that is returned in any case of error
		* Return value:	the value stored in the registry 
		* Author:		Aharon Abramson
		* Date:			October 2013
		******************************************************************************************/
        template<class T> static T GetRegistryOrEtcValue(const string& name, const T& defaultVal);

	protected:

        /******************************************************************************************
		* Function: 	trim
		* Description:	trim operation - remove unnecessary empty strings
		* Arguments:	string& s [in] -	reference to input string
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		static void trim( string& str );

	};

	// Convert Type T to a string
	template<class T>
	string ConfigFile::ConvertTypeToString( const T& t )
	{
		std::ostringstream ostOutput;
		ostOutput << t;
		return ostOutput.str();
	}

	// Convert string argument into a specific Type
	template<class T>
	T ConfigFile::ConvertStringToType( const string& str )
	{
		T returned_type;
		std::istringstream istInput(str);
		istInput >> returned_type;
		return returned_type;
	}

	// Convert string argument to a string type
	template<>
	inline string ConfigFile::ConvertStringToType<string>( const string& str )
	{
		return str;
	}

	// Convert string argument to a bool type
	// False cobnsidered as one of the followings: {"0", "false", "F", "no", "n"}
	// True considered as one of the followings: {"1", "True", "true", "T", "yes", "y", "-1", all others}
	template<>
	inline bool ConfigFile::ConvertStringToType<bool>( const string& str )
	{
		string strInput = str;
		string::iterator it = strInput.begin();
		while ( it != strInput.end() )
		{
			// convert all string chars to upper case
			*it = toupper(*it);
			++it;
		}
		if( strInput==string("0")     ||
			strInput==string("FALSE") ||
			strInput==string("NO")    ||
			strInput==string("F")     ||
			strInput==string("N")     ||
			strInput==string("NONE"))
		{
			return false;
		}
		return true;
	}

	template<class T>
	T ConfigFile::Read( const string& key, const T& value ) const
	{
		// search first for environment variable
		string strEnv;
		cl_err_code clErr = Intel::OpenCL::Utils::GetEnvVar(strEnv, key);
		if (CL_SUCCEEDED(clErr))
		{
			return ConvertStringToType<T>( strEnv );
		}

		// Return the value corresponding to key or given default value
		// if key is not found
		mapci p = m_mapContents.find(key);
		if ( p == m_mapContents.end() )
		{
			return value;
		}
		return ConvertStringToType<T>( p->second );
	}


	template<class T>
	bool ConfigFile::ReadInto( T& returnedVar, const string& strKey ) const
	{
		// search first for environment variable
		bool bFound = true;
		string strEnv;
		cl_err_code clErr = Intel::OpenCL::Utils::GetEnvVar(strEnv, strKey);
		if (CL_SUCCEEDED(clErr))
		{
			returnedVar = ConvertStringToType<T>( strEnv );
			return bFound;
		}

		// if the environment value doesn't exists get the value corresponding to the input key and store it in var
		std::map<string,string>::const_iterator it = m_mapContents.find(strKey);
		bFound = ( it != m_mapContents.end() );
		if( bFound )
		{
			returnedVar = ConvertStringToType<T>( it->second );
		}
		return bFound;
	}


	template<class T>
	bool ConfigFile::ReadInto( T& returnVar, const string& strKey, const T& defultValue ) const
	{
		// search first for environment variable
		bool bFound = true;
		string strEnv;
		cl_err_code clErr = Intel::OpenCL::Utils::GetEnvVar(strEnv, strKey);
		if (CL_SUCCEEDED(clErr))
		{
			returnVar = ConvertStringToType<T>( strEnv );
			return bFound;
		}

		// if the environment value doesn't exists get the value corresponding to the input key and store it in var
		std::map<string,string>::const_iterator it = m_mapContents.find(strKey);
		bFound = ( it != m_mapContents.end() );
		if( bFound )
		{
			returnVar = ConvertStringToType<T>( it->second );
		}
		else
		{
			returnVar = defultValue;
		}
		return bFound;
	}


	template<class T>
	void ConfigFile::Add( string strKey, const T& value )
	{
		// Add a key with given value
		string strValue = ConvertTypeToString( value );
		trim(strKey);
		trim(strValue);
		m_mapContents[strKey] = strValue;
		return;
	}

#ifdef _WIN32

template<typename T>
T GetRegistryValue(HKEY key, const string& valName, const T& defaultVal)
{
    T regVal;
    DWORD regValSize = sizeof(regVal);
    LONG res = RegQueryValueExA(key, valName.c_str(), NULL, NULL, (BYTE*)&regVal, &regValSize);
    if (ERROR_SUCCESS != res)
    {
        return defaultVal;
    }
    return regVal;
}

template<typename T>
T GetRegistryKeyValue(const string& keyName, const string& valName, T defaultVal)
{
    HKEY key = NULL;
	LONG res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName.c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key);

    if (ERROR_SUCCESS != res)
    {
        return defaultVal;
    }
    else
    {
        T regVal = GetRegistryValue(key, valName, defaultVal);
        RegCloseKey(key);
        return regVal;
    }    
}
#endif

#ifndef DEVICE_NATIVE
    template<class T>
    T ConfigFile::GetRegistryOrEtcValue(const string& name, const T& defaultVal)
    {        
#ifdef _WIN32
        return GetRegistryKeyValue("SOFTWARE\\Intel\\OpenCL", name, defaultVal);
#else
        T regVal;        
        std::ifstream ifs(("/etc/OpenCL/vendors/Intel/" + name).c_str());
        if (!ifs.good())
        {
            return defaultVal;
        }        
        ifs >> regVal;
        if (!ifs.good())
        {
            return defaultVal;
        }
        ifs.close();
        return regVal;
#endif        
    }
#endif

    enum OPENCL_VERSION
    {
        OPENCL_VERSION_UNKNOWN =    0,
        OPENCL_VERSION_1_2 =        1,
        OPENCL_VERSION_2_0 =        2,
        OPENCL_VERSION_2_1 =        3,
        OPENCL_VERSION_2_2 =        4
    };

    OPENCL_VERSION GetOpenclVerByCpuModel();

    /**
     * This is the base class to all config wrappers.
     */
    class BasicCLConfigWrapper
    {
    public:
        BasicCLConfigWrapper()
            : m_pConfigFile(NULL)
        {}

        ~BasicCLConfigWrapper()
        {
            Release();
        }

        cl_err_code Initialize(std::string filename)
        {
	        m_pConfigFile = new ConfigFile(filename);
	        return CL_SUCCESS;
        }

        void Release()
        {
	        if (NULL != m_pConfigFile)
	        {
		        delete m_pConfigFile;
		        m_pConfigFile = NULL;
	        }
        }

        /**
         * @return the dynamically detected OpenCL version (according to registry in Windows and /etc/ in Linux)
         */        
        OPENCL_VERSION GetOpenCLVersion() const;
        bool DisableStackDump() const {
#ifndef NDEBUG
            return m_pConfigFile->Read<bool>("CL_DISABLE_STACK_TRACE", false );
#else
            return false;
#endif
        }
        bool UseRelaxedMath() const { return m_pConfigFile->Read<bool>("CL_CONFIG_USE_FAST_RELAXED_MATH", false); }
        int  RTLoopUnrollFactor() const { return m_pConfigFile->Read<int>("CL_CONFIG_CPU_RT_LOOP_UNROLL_FACTOR", 1); }

        unsigned long GetForcedLocalMemSize() const
        {
            std::string strForcedSize;
            if (!m_pConfigFile->ReadInto(strForcedSize, "CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE"))
            {
                return 0;
            }

            return ParseStringToSize(strForcedSize);
        }
	
	private:
		BasicCLConfigWrapper(const BasicCLConfigWrapper&);
		BasicCLConfigWrapper& operator=(const BasicCLConfigWrapper&);
    
	protected:
        ConfigFile * m_pConfigFile;

        unsigned long ParseStringToSize(const std::string& userStr) const;
    };

}}}
