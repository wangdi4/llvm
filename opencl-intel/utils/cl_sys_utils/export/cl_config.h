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
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////
#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
//  cl_config.h
//  Implementation of the configuration file class
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
//
//  Typical usage
//  -------------
//  Given a configuration file "settings.inf":
//     atoms  = 25
//     length = 8.0  # nanometers
//     name = Reece Surcher
// 
//  Named values are read in various ways, with or without default values:
//     ConfigFile config( "settings.inp" );
//     int atoms = config.read<int>( "atoms" );
//     double length = config.read( "length", 10.0 );
//     string author, title;
//     config.readInto( author, "name" );
//     config.readInto( title, "title", string("Untitled") );
// 
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_types.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cl_env.h"

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
		ConfigFile( string sfilename,
			string sDelimiter = "=",
			string sComment = "#",
			string sSentry = "EndConfigFile" );

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
		T Read( const string& key, const T& value ) const;

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
		string GetDelimiter() const
		{ 
			return m_sDelimiter;
		}

		/******************************************************************************************
		* Function: 	GetComment    
		* Description:	get comment of configuration syntax
		*				call as comment = GetComment()
		* Arguments:	
		* Return value:	comment of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		string GetComment() const
		{
			return m_sComment; 
		}

		/******************************************************************************************
		* Function: 	GetSentry    
		* Description:	Get value of the sentry from the configuration syntax
		*				call as sentry = GetSentry()
		* Arguments:	
		* Return value:	sentry of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		string GetSentry() const 
		{ 
			return m_sSentry; 
		}

		/******************************************************************************************
		* Function: 	SetDelimiter    
		* Description:	set new delimiter value to the configuration syntax
		*				call as old_delimiter = SetDelimiter(new_delimiter)
		* Arguments:	const string& s [in] -	new delimiter
		* Return value:	previous value of delimiter of configuration syntax
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		string SetDelimiter(const string& s)
		{ 
			string old = m_sDelimiter;
			m_sDelimiter = s;  
			return old; 
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
		string SetComment(const string& s)
		{ 
			string old = m_sComment;
			m_sComment = s;
			return old;
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
		static cl_err_code ReadFile(string fileName, ConfigFile& cf);

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

	protected:

		/******************************************************************************************
		* Function: 	T_as_string    
		* Description:	convert class T to string
		* Arguments:	const T& t [in] -	class T
		* Return value:	string whihc represents the class
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		template<class T> static string T_as_string( const T& t );

		/******************************************************************************************
		* Function: 	string_as_T    
		* Description:	convert string to class T
		* Arguments:	const string& s [in] -	input string
		* Return value:	class T
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		template<class T> static T string_as_T( const string& s );

		/******************************************************************************************
		* Function: 	trim
		* Description:	trim operation - remove unnecessary empty strings
		* Arguments:	string& s [in] -	reference to input string
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		static void trim( string& s );

	};

	/* static */
	template<class T>
	string ConfigFile::T_as_string( const T& t )
	{
		// Convert from a T to a string
		// Type T must support << operator
		std::ostringstream ost;
		ost << t;
		return ost.str();
	}

	/* static */
	template<class T>
	T ConfigFile::string_as_T( const string& s )
	{
		// Convert from a string to a T
		// Type T must support >> operator
		T t;
		std::istringstream ist(s);
		ist >> t;
		return t;
	}

	/* static */
	template<>
	inline string ConfigFile::string_as_T<string>( const string& s )
	{
		// Convert from a string to a string
		// In other words, do nothing
		return s;
	}

	/* static */
	template<>
	inline bool ConfigFile::string_as_T<bool>( const string& s )
	{
		// Convert from a string to a bool
		// Interpret "false", "F", "no", "n", "0" as false
		// Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true
		bool b = true;
		string sup = s;
		for( string::iterator p = sup.begin(); p != sup.end(); ++p )
			*p = toupper(*p);  // make string all caps
		if( sup==string("FALSE") || sup==string("F") ||
			sup==string("NO") || sup==string("N") ||
			sup==string("0") || sup==string("NONE") )
			b = false;
		return b;
	}

	template<class T>
	T ConfigFile::Read( const string& key, const T& value ) const
	{
		// search first for environment variable
		string strEnv;
		cl_err_code clErr = Intel::OpenCL::Utils::GetEnvVar(strEnv, key);
		if (CL_SUCCEEDED(clErr))
		{
			return string_as_T<T>( strEnv );
		}

		// Return the value corresponding to key or given default value
		// if key is not found
		mapci p = m_mapContents.find(key);
		if ( p == m_mapContents.end() )
		{
			return value;
		}
		return string_as_T<T>( p->second );
	}


	template<class T>
	bool ConfigFile::ReadInto( T& var, const string& key ) const
	{
		// search first for environment variable
		string strEnv;
		cl_err_code clErr = Intel::OpenCL::Utils::GetEnvVar(strEnv, key);
		if (CL_SUCCEEDED(clErr))
		{
			var = string_as_T<T>( strEnv );
			return found;
		}

		// Get the value corresponding to key and store in var
		// Return true if key is found
		// Otherwise leave var untouched
		mapci p = m_mapContents.find(key);
		found = ( p != m_mapContents.end() );
		if( found )
		{
			var = string_as_T<T>( p->second );
		}
		return found;
	}


	template<class T>
	bool ConfigFile::ReadInto( T& var, const string& key, const T& value ) const
	{
		// search first for environment variable
		string strEnv;
		cl_err_code clErr = Intel::OpenCL::Utils::GetEnvVar(strEnv, key);
		if (CL_SUCCEEDED(clErr))
		{
			var = string_as_T<T>( strEnv );
			return found;
		}

		// Get the value corresponding to key and store in var
		// Return true if key is found
		// Otherwise set var to given default
		mapci p = m_mapContents.find(key);
		found = ( p != m_mapContents.end() );
		if( found )
		{
			var = string_as_T<T>( p->second );
		}
		else
		{
			var = value;
		}
		return found;
	}


	template<class T>
	void ConfigFile::Add( string key, const T& value )
	{
		// Add a key with given value
		string v = T_as_string( value );
		trim(key);
		trim(v);
		m_mapContents[key] = v;
		return;
	}
}}}
