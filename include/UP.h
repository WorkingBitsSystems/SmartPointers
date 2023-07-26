/**
 * UP.h
 *
 * Abstract:
 * Definition of a template class that is a smart pointer class that maintains
 * an "unique pointer" to a memory location of the passed type.  Upon
 * destruction of this object, the memory it points to is freed to the system.
 *
 * This class is designed to replace the C++ unique_pointer, which is overly
 * verbose in name and function.
 *
 * @see TP.h
 *
 * @copyright Copyright(C) Working Bits Systems, LLC 2023
 */
#ifndef _UP_H_
#define _UP_H_
#include <memory>

/**
 * The Working Bits Systems namespace.
 */
namespace wbs
{

    // Forward declare SP for friend.
    template< typename TYPESP > class SP;

    /**
     * @class UP
     * (Unique Pointer implementation with MUCH shorter name)
     *
     * This class manages ownership of an object of the type passed as the
     * template parameter.  It forces the semantics of the unique owning pointer
     * such that memory will not be leaked.
     */
    template< typename TYPEUP > class UP
    {
    public:
        // Constructors, destructor, and Assignment operator -------------------
        /**
         * Default Constructor
         */
        UP()
        {
            m_ptr = nullptr;
        }

        /**
         * Move semantics constructor.
         */
        UP( UP< TYPEUP >&& other )
        {
             m_ptr = other.m_ptr;
             other.m_ptr = nullptr;
        }

        /**
         * Take over ownership from a std::unique_pointer.
         */
        UP( std::unique_ptr< TYPEUP >& other )
        {
             m_ptr = other.release();
        }

        /**
         * Take over ownership from a raw c-style pointer.
         */
        UP( TYPEUP* p )
        {
             m_ptr = p;
        }

        /**
         * Destructor
         * deletes the referenced memory block if the pointer is initialized.
         */
        ~UP()
        {
            if ( nullptr != m_ptr )
            {
                delete m_ptr;
            }
        }

        /**
         * Move semantics assignment operator.
         */
        UP< TYPEUP >& operator=( UP< TYPEUP >&& other )
        {
            if ( nullptr != m_ptr )
            {
                delete m_ptr;
            }
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;

            return *this;
        }

        /**
         * Assignment operator for a raw c-style pointer.
         */
        UP< TYPEUP >& operator=( TYPEUP* p )
        {
            if ( nullptr != m_ptr )
            {
                delete m_ptr;
            }
            m_ptr = p;

            return *this;
        }

        // Operations ----------------------------------------------------------
        /**
         * Arrow operator
         *
         * @return The TYPEUP pointer for dereferencing
         */
        TYPEUP* operator->() const
        {
            return m_ptr;
        }

        /**
         * Star operator
         *
         * @return A reference to the object this UP points to.
         */
        TYPEUP& operator*() const
        {
            return *m_ptr;
        }

        /**
         * Comparison operators.  Lumping these all together for ease.
         *
         * @param rhs - the right-hand-side of the comparison (this is lhs)
         *
         * @return true if this pointer has the relationship to the rhs as
         *         specified.
         */
        inline bool operator==( const UP< TYPEUP >& other ) const { return UnsafeAccess() == other.UnsafeAccess(); }
        inline bool operator!=( const UP< TYPEUP >& other ) const { return UnsafeAccess() != other.UnsafeAccess(); }
        inline bool operator<( const UP< TYPEUP >& other ) const  { return UnsafeAccess() < other.UnsafeAccess(); }
        inline bool operator<=( const UP< TYPEUP >& other ) const { return UnsafeAccess() <= other.UnsafeAccess(); }
        inline bool operator>( const UP< TYPEUP >& other ) const  { return UnsafeAccess() > other.UnsafeAccess(); }
        inline bool operator>=( const UP< TYPEUP >& other ) const { return UnsafeAccess() >= other.UnsafeAccess(); }

        // Methods -------------------------------------------------------------
        /**
         * Deletes the object this UP points to and clears the pointer (sets to
         * nullptr).
         */
        void Delete()
        {
            if ( nullptr != m_ptr )
            {
                delete m_ptr;
                m_ptr = nullptr;
            }
        }

        /**
         * Checks if the internal pointer is null.
         *
         * @return true is the internal pointer is nullptr, otherwise false.
         */
        bool IsNull() const
        {
            return ( nullptr == m_ptr );
        }

        /**
         * Pointer access.
         *
         * @note This is considered an unsafe operation and should be avoided.
         *       Possible exceptions include passing a TEMPORARY pointer to
         *       a system or third party function/method.
         *
         * @return The raw pointer to the object.
         */
        TYPEUP* UnsafeAccess() const
        {
            return m_ptr;
        }


        /**
         * Conversion to std::unique_ptr.
         *
         * Generates a std::unique_ptr< TYPEUP > from this object, while
         * clearing the internal pointer.
         *
         * @note Allows conversion to C++ standard smart pointers if needed.
         *
         * @return The std::unique_ptr that now has sole ownership of the
         *         referenced object.
         */
        std::unique_ptr< TYPEUP > StdUniquePtr()
        {
            std::unique_ptr< TYPEUP > ret( m_ptr );
            m_ptr = nullptr;

            return  ret;
        }

    private:
        // Constructors, destructor, and Assignment operator -------------------
        /**
         * Copy constructor disallowed as it would create a second UP that
         * points to the same object.
         */
        UP< TYPEUP >( const UP< TYPEUP >& );
        /**
         * Assignment operator disallowed as it would create a second UP that
         * points to the same object.
         */
        const UP< TYPEUP >& operator=( const UP< TYPEUP >& );

        // Attributes ----------------------------------------------------------
        // The pointer to the object this class manages.
        TYPEUP* m_ptr;

        // Friend classes ------------------------------------------------------
        // Give SP access to clear m_ptr when changing this UP to an SP.
        friend class SP< TYPEUP >;
    };
}; // End of namespace wbs

#endif // #ifndef _UP_H_
