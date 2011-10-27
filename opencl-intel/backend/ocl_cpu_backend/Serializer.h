/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Serializer.h

\*****************************************************************************/
#ifndef __SERIALIZER
#define __SERIALIZER
#include "exceptions.h"
#include <malloc.h>

/**
 * This file contains utlity functions which will be used for serialization
 */
namespace Intel { namespace OpenCL { namespace DeviceBackend {

    /**
     * This class reperesents interface for output serialization stream
     */
    class IOutputStream
    {
    public:
        virtual IOutputStream& Write(const char* s, size_t count) = 0;
    };

    /**
     * This class reperesents interface for input serialization stream
     */
    class IInputStream
    {
    public:
        virtual IInputStream& Read(char* s, size_t count) = 0;
    };

    /**
     * This class represents serialization exception which will be thrown in case 
     * errors occured during serialize\deserialize
     */
    DEFINE_EXCEPTION(SerializationException)

    /**
     * This class consists from general methods used for serialization and it
     * can handle common data structures
     */
    class Serializer
    {
    public:
        template<class T>
        static void SerialPrimitive(const T* item, IOutputStream& stream)
        {
            stream.Write((const char*)item, sizeof(T));
        }

        template<class T>
        static void DeserialPrimitive(T* item, IInputStream& stream)
        {
            stream.Read((char *)item, sizeof(T));
        }

        template<class T>
        static void SerialNullTerminatedPrimitivesBuffer(const T* item, IOutputStream& stream)
        {
            int length = 0;
            const T* elemnt = (const T*)item;
            if (NULL == item)
            {
                stream.Write((const char*)(length), sizeof(int));
                return ;
            }
            
            while ((T)NULL != *elemnt) 
            {
                elemnt++;
                length++;
            }

            length++;
            stream.Write((const char*)&length, sizeof(length));
            stream.Write((const char*)item, sizeof(T) * length);
        }

        template<class T>
        static void DeserialNullTerminatedPrimitivesBuffer(T*& item, IInputStream& stream)
        {
            int length = 0;
            stream.Read((char *)&length, sizeof(int));
            if (0 == length)
            {
                item = NULL;
                return ;
            }

            item = (T*)malloc(sizeof(T) * length);
            if (NULL == item) throw Exceptions::SerializationException("Cannot Allocate Memory");

            stream.Read((char *)item, sizeof(T) * length);
        }

        template<class T>
        static void SerialPrimitivesBuffer(const T* item, int length, IOutputStream& stream)
        {
            stream.Write((const char*)length, sizeof(length));
            if(0 == length) return ;

            stream.Write((const char*)item, sizeof(T) * length);
        }

        template<class T>
        static void DeserialPrimitivesBuffer(T*& item, int& length, IInputStream& stream)
        {
            stream.Read((char *)&length, sizeof(length));
            if (0 == length)
            {
                item = NULL;
                return ;
            }

            item = (T*)malloc(sizeof(T) * length);
            if (NULL == item) throw Exceptions::SerializationException("Cannot Allocate Memory");

            stream.Read((char *)item, sizeof(T) * length);
        }

        static void SerialString(const std::string& item, IOutputStream& stream)
        {
            SerialNullTerminatedPrimitivesBuffer<char>(item.c_str() , stream);
        }

        static void DeserialString(std::string& item, IInputStream& stream)
        {
            char* temp = NULL;
            DeserialNullTerminatedPrimitivesBuffer<char>(temp, stream);

            try
            {
                item = std::string(temp);
            }
            catch( std::bad_alloc& )
            {
                free(temp);
                throw Exceptions::SerializationException("Cannot Allocate Memory");
            }
            free(temp);
        }

        static void SerialPointerHint(const void** item, IOutputStream& stream)
        {
            bool isNullPointer = (NULL == item);
            SerialPrimitive<bool>(&isNullPointer, stream);
        }

        static void DeserialPointerHint(void** item, IInputStream& stream)
        {
            bool isNullPointer = false;
            DeserialPrimitive<bool>(&isNullPointer, stream);
            *item = (void*)(isNullPointer ? NULL : !NULL);
        }

    };

}}} // namespace

#endif // __SERIALIZER
