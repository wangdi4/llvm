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

#ifndef __TBB_queuing_rw_mutex_H
#define __TBB_queuing_rw_mutex_H

#include "detail/_config.h"
#include "detail/_assert.h"

#include "profiling.h"

#include <cstring>
#include <atomic>

namespace tbb {
namespace detail {
namespace d1 {

//! Queuing reader-writer mutex with local-only spinning.
/** Adapted from Krieger, Stumm, et al. pseudocode at
    https://www.researchgate.net/publication/221083709_A_Fair_Fast_Scalable_Reader-Writer_Lock
    @ingroup synchronization */
class queuing_rw_mutex {
public:
    //! Construct unacquired mutex.
    queuing_rw_mutex() noexcept  {
        create_itt_sync(this, "tbb::queuing_rw_mutex", "");
#if TBB_USE_PROFILING_TOOLS
        internal_construct();
#endif
    }

    //! Destructor asserts if the mutex is acquired, i.e. q_tail is non-NULL
    ~queuing_rw_mutex() {
        __TBB_ASSERT( !q_tail, "destruction of an acquired mutex");
    }

    //! No Copy
    queuing_rw_mutex(const queuing_rw_mutex&) = delete;
    queuing_rw_mutex& operator=(const queuing_rw_mutex&) = delete;

    //! The scoped locking pattern
    /** It helps to avoid the common problem of forgetting to release lock.
        It also nicely provides the "node" for queuing locks. */
    class scoped_lock {
        //! Initialize fields to mean "no lock held".
        void initialize() {
            my_mutex = nullptr;
            my_internal_lock.store(0, std::memory_order_relaxed);
            my_going.store(0, std::memory_order_relaxed);
#if TBB_USE_ASSERT
            my_state = 0xFF; // Set to invalid state
            my_next.store(reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(-1)), std::memory_order_relaxed);
            my_prev.store(reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(-1)), std::memory_order_relaxed);
#endif /* TBB_USE_ASSERT */
        }

    public:
        //! Construct lock that has not acquired a mutex.
        /** Equivalent to zero-initialization of *this. */
        scoped_lock() {initialize();}

        //! Acquire lock on given mutex.
        scoped_lock( queuing_rw_mutex& m, bool write=true ) {
            initialize();
            acquire(m,write);
        }

        //! Release lock (if lock is held).
        ~scoped_lock() {
            if( my_mutex ) release();
        }

        //! No Copy
        scoped_lock(const scoped_lock&) = delete;
        scoped_lock& operator=(const scoped_lock&) = delete;

        //! Acquire lock on given mutex.
        void acquire( queuing_rw_mutex& m, bool write=true );

        //! Acquire lock on given mutex if free (i.e. non-blocking)
        bool try_acquire( queuing_rw_mutex& m, bool write=true );

        //! Release lock.
        void release();

        //! Upgrade reader to become a writer.
        /** Returns whether the upgrade happened without releasing and re-acquiring the lock */
        bool upgrade_to_writer();

        //! Downgrade writer to become a reader.
        bool downgrade_to_reader();

    private:
        //! The pointer to the mutex owned, or NULL if not holding a mutex.
        queuing_rw_mutex* my_mutex;

        //! The 'pointer' to the previous and next competitors for a mutex
        std::atomic<uintptr_t> my_prev;
        std::atomic<uintptr_t> my_next;

        using state_t = unsigned char ;

        //! State of the request: reader, writer, active reader, other service states
        std::atomic<state_t> my_state;

        //! The local spin-wait variable
        /** Corresponds to "spin" in the pseudocode but inverted for the sake of zero-initialization */
        std::atomic<unsigned char> my_going;

        //! A tiny internal lock
        std::atomic<unsigned char> my_internal_lock;

        //! Acquire the internal lock
        void acquire_internal_lock();

        //! Try to acquire the internal lock
        /** Returns true if lock was successfully acquired. */
        bool try_acquire_internal_lock();

        //! Release the internal lock
        void release_internal_lock();

        //! Wait for internal lock to be released
        void wait_for_release_of_internal_lock();

        //! A helper function
        void unblock_or_wait_on_internal_lock( uintptr_t );
    };

    void __TBB_EXPORTED_METHOD internal_construct();

    // Mutex traits
    static constexpr bool is_rw_mutex = true;
    static constexpr bool is_recursive_mutex = false;
    static constexpr bool is_fair_mutex = true;

private:
    //! The last competitor requesting the lock
    std::atomic<scoped_lock*> q_tail{nullptr};
};

} // namespace d1
} // namespace detail

inline namespace v1 {
using detail::d1::queuing_rw_mutex;
__TBB_DEFINE_PROFILING_SET_NAME(queuing_rw_mutex)
} // namespace v1
} // namespace tbb

#endif /* __TBB_queuing_rw_mutex_H */
