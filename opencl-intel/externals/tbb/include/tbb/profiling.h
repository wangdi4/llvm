/*
    Copyright (c) 2005-2020 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef __TBB_profiling_H
#define __TBB_profiling_H

#include "detail/_config.h"
#include <cstdint>

namespace tbb {
namespace detail {
inline namespace d0 {
    enum byte : unsigned char {};
    // include list of index names
    #define TBB_STRING_RESOURCE(index_name,str) index_name,
    enum string_resource_index : std::uintptr_t {
        #include "detail/_string_resource.h"
        NUM_STRINGS
    };
    #undef TBB_STRING_RESOURCE

    enum itt_relation
    {
    __itt_relation_is_unknown = 0,
    __itt_relation_is_dependent_on,         /**< "A is dependent on B" means that A cannot start until B completes */
    __itt_relation_is_sibling_of,           /**< "A is sibling of B" means that A and B were created as a group */
    __itt_relation_is_parent_of,            /**< "A is parent of B" means that A created B */
    __itt_relation_is_continuation_of,      /**< "A is continuation of B" means that A assumes the dependencies of B */
    __itt_relation_is_child_of,             /**< "A is child of B" means that A was created by B (inverse of is_parent_of) */
    __itt_relation_is_continued_by,         /**< "A is continued by B" means that B assumes the dependencies of A (inverse of is_continuation_of) */
    __itt_relation_is_predecessor_to        /**< "A is predecessor to B" means that B cannot start until A completes (inverse of is_dependent_on) */
    };

//! Unicode support
#if (_WIN32||_WIN64) && !__MINGW32__
    //! Unicode character type. Always wchar_t on Windows.
    using tchar = wchar_t;
#else /* !WIN */
    using tchar = char;
#endif /* !WIN */

} // namespace d0
} // namespace detail
} // namespace tbb

// Check if the tools support is enabled
#if (_WIN32||_WIN64||__linux__) && !__MINGW32__ && TBB_USE_PROFILING_TOOLS

#if _WIN32||_WIN64
#include <stdlib.h>  /* mbstowcs_s */
#endif

namespace tbb {
namespace detail {
namespace d1 {

#if _WIN32||_WIN64
    void __TBB_EXPORTED_FUNC itt_set_sync_name( void *obj, const wchar_t* name );
    inline std::size_t multibyte_to_widechar( wchar_t* wcs, const char* mbs, std::size_t bufsize) {
#if _MSC_VER>=1400
        std::size_t len;
        mbstowcs_s( &len, wcs, bufsize, mbs, _TRUNCATE );
        return len;   // mbstowcs_s counts null terminator
#else
        std::size_t len = mbstowcs( wcs, mbs, bufsize );
        if(wcs && len!=size_t(-1) )
            wcs[len<bufsize-1? len: bufsize-1] = wchar_t('\0');
        return len+1; // mbstowcs does not count null terminator
#endif
    }
#else
    void __TBB_EXPORTED_FUNC itt_set_sync_name( void *obj, const char* name );
#endif
} // namespace d1
} // namespace detail
} // namespace tbb

//! Macro __TBB_DEFINE_PROFILING_SET_NAME(T) defines "set_name" methods for sync objects of type T
/** Should be used in the "tbb" namespace only.
    Don't place semicolon after it to avoid compiler warnings. **/
#if _WIN32||_WIN64
    #define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)                       \
        namespace profiling {                                                       \
            inline void set_name( sync_object_type& obj, const wchar_t* name ) {    \
                tbb::detail::d1::itt_set_sync_name( &obj, name );                  \
            }                                                                       \
            inline void set_name( sync_object_type& obj, const char* name ) {       \
                std::size_t len = tbb::detail::d1::multibyte_to_widechar(nullptr, name, 0);   \
                wchar_t *wname = new wchar_t[len];                                  \
                tbb::detail::d1::multibyte_to_widechar(wname, name, len);             \
                set_name( obj, wname );                                             \
                delete[] wname;                                                     \
            }                                                                       \
        }
#else /* !WIN */
    #define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)                       \
        namespace profiling {                                                       \
            inline void set_name( sync_object_type& obj, const char* name ) {       \
                tbb::detail::d1::itt_set_sync_name( &obj, name );                  \
            }                                                                       \
        }
#endif /* !WIN */

#else /* no tools support */

#if _WIN32||_WIN64
    #define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)               \
        namespace profiling {                                               \
            inline void set_name( sync_object_type&, const wchar_t* ) {}    \
            inline void set_name( sync_object_type&, const char* ) {}       \
        }
#else /* !WIN */
    #define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)               \
        namespace profiling {                                               \
            inline void set_name( sync_object_type&, const char* ) {}       \
        }
#endif /* !WIN */

#endif /* no tools support */

#include <atomic>

// Need these to work regardless of tools support
namespace tbb {
namespace detail {
namespace d1 {

    enum notify_type {prepare=0, cancel, acquired, releasing};

    void __TBB_EXPORTED_FUNC call_itt_notify_impl(int t, void *ptr);
    void __TBB_EXPORTED_FUNC create_itt_sync_impl(void *ptr, const tchar *objtype, const tchar *objname);

    enum itt_domain_enum { ITT_DOMAIN_FLOW=0, ITT_DOMAIN_MAIN=1, ITT_DOMAIN_ALGO=2, ITT_NUM_DOMAINS };
    void __TBB_EXPORTED_FUNC itt_make_task_group_impl(itt_domain_enum domain, void* group, unsigned long long group_extra,
        void* parent, unsigned long long parent_extra, string_resource_index name_index);
    void __TBB_EXPORTED_FUNC itt_task_begin_impl(itt_domain_enum domain, void* task, unsigned long long task_extra,
        void* parent, unsigned long long parent_extra, string_resource_index name_index);
    void __TBB_EXPORTED_FUNC itt_task_end_impl(itt_domain_enum domain);

#if TBB_USE_PROFILING_TOOLS
    inline void create_itt_sync(void *ptr, const char *objtype, const char *objname) {
#if (_WIN32||_WIN64) && !__MINGW32__
        std::size_t len_type = multibyte_to_widechar(nullptr, objtype, 0);
        wchar_t *type = new wchar_t[len_type];
        multibyte_to_widechar(type, objtype, len_type);
        std::size_t len_name = multibyte_to_widechar(nullptr, objname, 0);
        wchar_t *name = new wchar_t[len_name];
        multibyte_to_widechar(name, objname, len_name);
#else
        const char *type = objtype;
        const char *name = objname;
#endif
        create_itt_sync_impl(ptr, type, name);

#if (_WIN32||_WIN64) && !__MINGW32__
        delete[] type;
        delete[] name;
#endif
    }

    inline void call_itt_notify(notify_type t, void *ptr) {
        call_itt_notify_impl((int)t, ptr);
    }

    inline void itt_make_task_group(itt_domain_enum domain, void* group, unsigned long long group_extra,
        void* parent, unsigned long long parent_extra, string_resource_index name_index) {
        itt_make_task_group_impl(domain, group, group_extra, parent, parent_extra, name_index);
    }
#else
    inline void create_itt_sync(void* /*ptr*/, const char* /*objtype*/, const char* /*objname*/) {}
    inline void call_itt_notify(notify_type /*t*/, void* /*ptr*/) {}
    inline void itt_make_task_group(itt_domain_enum /*domain*/, void* /*group*/, unsigned long long /*group_extra*/,
        void* /*parent*/, unsigned long long /*parent_extra*/, string_resource_index /*name_index*/) {}

#endif // TBB_USE_PROFILING_TOOLS


    template <typename T>
    inline void store_with_release_itt(std::atomic<T>& dst, T src) {
        call_itt_notify(releasing, &dst);
        dst.store(src, std::memory_order_release);
    }

    template <typename T>
    inline T load_with_acquire_itt(const std::atomic<T>& src) {
        call_itt_notify(acquired, &src);
        return src.load(std::memory_order_acquire);
    }
} // namespace d1
} // namespace detail
} // namespace tbb


#endif /* __TBB_profiling_H */
