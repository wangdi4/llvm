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

#ifndef __TBB_detail__rtm_rw_mutex_H
#define __TBB_detail__rtm_rw_mutex_H

#include "_assert.h"
#include "_utils.h"
#include "../spin_rw_mutex.h"

#include <atomic>

namespace tbb {
namespace detail {
namespace d1 {

constexpr std::size_t speculation_granularity = 64;
#if _MSC_VER && !defined(__INTEL_COMPILER)
    // Suppress warning: structure was padded due to alignment specifier
    #pragma warning (push)
    #pragma warning (disable: 4324)
#endif

//! Fast, unfair, spinning speculation-enabled reader-writer lock with backoff and writer-preference
/** @ingroup synchronization */
class alignas(max_nfs_size) rtm_rw_mutex : private spin_rw_mutex {
private:
    enum class rtm_type {
        rtm_not_in_mutex,
        rtm_transacting_reader,
        rtm_transacting_writer,
        rtm_real_reader,
        rtm_real_writer
    };
public:
    //! Constructors
    rtm_rw_mutex() noexcept : write_flag(false) {
        create_itt_sync(this, "tbb::speculative_spin_rw_mutex", "");
    }

    //! Destructor
    ~rtm_rw_mutex() = default;

    //! Represents acquisition of a mutex.
    class scoped_lock {
    public:
        friend class rtm_rw_mutex;
        //! Construct lock that has not acquired a mutex.
        /** Equivalent to zero-initialization of *this. */
        constexpr scoped_lock() : m_mutex(nullptr), m_transaction_state(rtm_type::rtm_not_in_mutex) {}

        //! Acquire lock on given mutex.
        scoped_lock(rtm_rw_mutex& m, bool write = true) : m_mutex(nullptr), m_transaction_state(rtm_type::rtm_not_in_mutex) {
            acquire(m, write);
        }

        //! Release lock (if lock is held).
        ~scoped_lock() {
            if(m_transaction_state != rtm_type::rtm_not_in_mutex) {
                release();
            }
        }

        //! No Copy
        scoped_lock(const scoped_lock&) = delete;
        scoped_lock& operator=(const scoped_lock&) = delete;

        //! Acquire lock on given mutex.
        void acquire(rtm_rw_mutex& m, bool write = true) {
            __TBB_ASSERT(!m_mutex, "lock is already acquired");
            if (write) {
                m.internal_acquire_writer(*this);
            }
            else {
                m.internal_acquire_reader(*this);
            }
        }

        //! Try acquire lock on given mutex.
        bool try_acquire(rtm_rw_mutex& m, bool write = true) {
            __TBB_ASSERT(!m_mutex, "lock is already acquired");
            if (write) {
                return m.internal_try_acquire_writer(*this);
            }
            else {
                return m.internal_try_acquire_reader(*this);
            }
        }

        //! Release lock
        void release() {
            __TBB_ASSERT(m_mutex, "lock is not acquired");
            __TBB_ASSERT(m_transaction_state != rtm_type::rtm_not_in_mutex, "lock is not acquired");
            return m_mutex->internal_release(*this);
        }

        //! Upgrade reader to become a writer.
        /** Returns whether the upgrade happened without releasing and re-acquiring the lock */
        bool upgrade_to_writer() {
            __TBB_ASSERT(m_mutex, "lock is not acquired");
            if (m_transaction_state == rtm_type::rtm_transacting_writer || m_transaction_state == rtm_type::rtm_real_writer) {
                return true; // Already a writer
            }
            return m_mutex->internal_upgrade(*this);
        }

        //! Downgrade writer to become a reader.
        bool downgrade_to_reader() {
            __TBB_ASSERT(m_mutex, "lock is not acquired");
            if (m_transaction_state == rtm_type::rtm_transacting_reader || m_transaction_state == rtm_type::rtm_real_reader) {
                return true; // Already a reader
            }
            return m_mutex->internal_downgrade(*this);
        }

    private:
        rtm_rw_mutex* m_mutex;
        rtm_type m_transaction_state;
    };

    //! Mutex traits
    static constexpr bool is_rw_mutex = true;
    static constexpr bool is_recursive_mutex = false;
    static constexpr bool is_fair_mutex = false;

private:
    alignas(speculation_granularity) std::atomic<bool> write_flag;

    //! Internal acquire write lock.
    // only_speculate == true if we're doing a try_lock, else false.
    void __TBB_EXPORTED_METHOD internal_acquire_writer(rtm_rw_mutex::scoped_lock&, bool only_speculate = false);

    //! Internal acquire read lock.
    // only_speculate == true if we're doing a try_lock, else false.
    void __TBB_EXPORTED_METHOD internal_acquire_reader(rtm_rw_mutex::scoped_lock&, bool only_speculate = false);

    //! Internal upgrade reader to become a writer.
    bool __TBB_EXPORTED_METHOD internal_upgrade(rtm_rw_mutex::scoped_lock&);

    //! Internal downgrade writer to become a reader.
    bool __TBB_EXPORTED_METHOD internal_downgrade(rtm_rw_mutex::scoped_lock&);

    //! Internal try_acquire write lock.
    bool __TBB_EXPORTED_METHOD internal_try_acquire_writer(rtm_rw_mutex::scoped_lock&);

    //! Internal try_acquire read lock.
    bool __TBB_EXPORTED_METHOD internal_try_acquire_reader(rtm_rw_mutex::scoped_lock&);

    //! Internal release lock.
    void __TBB_EXPORTED_METHOD internal_release(rtm_rw_mutex::scoped_lock&);
};

#if _MSC_VER && !defined(__INTEL_COMPILER)
    #pragma warning (pop) // 4324 warning
#endif

} // namespace d1
} // namespace detail

} // namespace tbb

#endif // __TBB_detail__rtm_rw_mutex_H
