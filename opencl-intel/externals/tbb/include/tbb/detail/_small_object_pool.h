/*
    Copyright (c) 2020 Intel Corporation

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

#ifndef __TBB__small_object_pool_H
#define __TBB__small_object_pool_H

#include "_config.h"
#include "_assert.h"

#include <cstddef>
#include <cstdint>
#include <atomic>

namespace tbb {
namespace detail {
namespace d1 {

struct execute_data;

class small_object_pool
{
public:
    static void* __TBB_EXPORTED_FUNC allocate(small_object_pool*& allocator, std::size_t number_of_bytes,
                                              const execute_data& ed);
    static void* __TBB_EXPORTED_FUNC allocate(small_object_pool*& allocator, std::size_t number_of_bytes);
    void  __TBB_EXPORTED_METHOD deallocate(void* ptr, std::size_t number_of_bytes, const execute_data& ed);
    void  __TBB_EXPORTED_METHOD deallocate(void* ptr, std::size_t number_of_bytes);
protected:
    small_object_pool() = default;
};

class small_object_allocator
{
public:
    template <typename Type, typename... Args>
    Type* new_object(const execute_data& ed, Args&&... args) {
        void* allocated_object = small_object_pool::allocate(m_pool, sizeof(Type), ed);

        auto constructed_object = new(allocated_object) Type(std::forward<Args>(args)...);
        return constructed_object;
    }

    template <typename Type, typename... Args>
    Type* new_object(Args&&... args) {
        void* allocated_object = small_object_pool::allocate(m_pool, sizeof(Type));

        auto constructed_object = new(allocated_object) Type(std::forward<Args>(args)...);
        return constructed_object;
    }

    template <typename Type>
    void delete_object(Type* object, const execute_data& ed) {
        // Copy this since the it can be the member of the passed object and
        // unintentionally destroyed when Type destructor is called below
        small_object_allocator alloc = *this;
        object->~Type();
        alloc.deallocate(object, ed);
    }

    template <typename Type>
    void delete_object(Type* object) {
        // Copy this since the it can be the member of the passed object and
        // unintentionally destroyed when Type destructor is called below
        small_object_allocator alloc = *this;
        object->~Type();
        alloc.deallocate(object);
    }

    template <typename Type>
    void deallocate(Type* ptr, const execute_data& ed) {
        __TBB_ASSERT(m_pool != nullptr, "Pool must be valid for deallocate call");
        m_pool->deallocate(ptr, sizeof(Type), ed);
    }

    template <typename Type>
    void deallocate(Type* ptr) {
        __TBB_ASSERT(m_pool != nullptr, "Pool must be valid for deallocate call");
        m_pool->deallocate(ptr, sizeof(Type));
    }
private:
    small_object_pool* m_pool{};
};

} // namespace d1
} // namespace detail
} // namespace tbb

#endif /* __TBB__small_object_pool_H */
