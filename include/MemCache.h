/**
 * MemCache.h.h
 *
 * Abstract:
 * Defines a rudimentary override of operators new and delete in order to cache
 * memory allocations at a rate of CACHE_RATE above the objects in use of a
 * the inheriting class.  For example, the default CACHE_RATE of 50 will keep
 * a cache of up to 50% of the number of in-use allocations.  100 objects
 * allocated?  That will keep up to 50 cached memory allocations.
 *
 * @copyright Copyright(C) Working Bits Systems, LLC 2023
 */
#ifndef _MEMCACHE_H_
#define _MEMCACHE_H_
#include <vector>
#include <mutex>

// The Working Bits Systems namespace
namespace wbs
{
    /**
     * Base class used to provide a performance boost for frequently allocated
     * and freed objects.  By keeping memory allocations in this cache, the
     * system overhead of memory allocations is reduced.  CACHE_RATE
     * determines the size of the cache as a percentage of the currently
     * allocated objects.  The template parameter for MemCache must be the
     * inheriting class, allowing for the required static variables on a per-
     * inheriting-class basis.
     *
     * Usage: Publicly inherit this class.  E.g.:
     *        class Foo: public Memcache< Foo >
     *        {
     *        }
     *
     * @note Array allocation operators are ignored for this implementation.
     */
    template <typename T>
    class MemCache
    {
    public:
        /**
         * Allocates memory of the provided size.  "new" calls this method with
         * the size of the object needed for the class in question automatically.
         *
         * @param size - the size of the memory blck to be allocated
         *
         * @return a pointer to the memory block or nullptr if no memory can be
         *         allocated.
         */
        void* operator new( size_t size )
        {
            void* ret = nullptr;

            // Lock access to the cache
            sm_lock.lock();
            if ( 0 < sm_cache.size() )
            {
                ret = sm_cache.back();
                sm_cache.pop_back();
            }
            else
            {
                // Just need a buffer for the number of bytes.
                ret = ::new char[ size ];
            }
            ++sm_inUse;
            sm_lock.unlock();

            return ret;
        }

        /**
         * Frees memory previously allocated by operator new.  May place memory
         * into the internal cache for reuse.
         *
         * @param mem - the pointer to the memory to be deleted (or cached).
         */
        void operator delete( void* mem )
        {
            sm_lock.lock();

            // Just put it on the cache, and if we end up needing to delete it,
            // we will do so below.  In the rare case, this could cause an
            // unnecessary reallocation of memory behind the vector, but the
            // simplified code is worth it.
            sm_cache.push_back( mem );
            int size = sm_cache.size();

            // This object no longer in use.
            --sm_inUse;

            // Delete cache entries until we're down to our % of in use objects.
            int targetCacheSize = sm_inUse * CACHE_RATE / 100;
            while ( ( 0 != size ) && ( size > targetCacheSize ) )
            {
                ::delete( (char*) sm_cache.back() );
                sm_cache.pop_back();
                --size;
            }

            sm_lock.unlock();
        }

    private:
        // The number of objects currently in use (new'd, but not yet deleted)
        static inline int sm_inUse = 0;

        // The set of cached objects of type T.  We use a vector<void*> so the
        // internal allocation will grow to some (assumed) steady-state size.
        // This may slow initial delete operations as more object are added,
        // but we expect the application to get to a steady state over time.
        static inline std::vector< void * > sm_cache;

        // A lock for vector access to protect in a multithreaded environment.
        static inline std::mutex sm_lock;

        static constexpr int CACHE_RATE = 50;
    };
}

#endif // _MEMCACHE_H_
