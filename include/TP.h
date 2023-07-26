/**
 * TP.h
 *
 * Abstract:
 * Definition of a template class that is a smart pointer class that maintains
 * a "temporary pointer" to a memory location of the passed TYPETP. This
 * data type is used to allow other code blocks access to the object without
 * passing ownership.
 *
 * Unlike the UP class, there is no comparable stabdard library class to this TP
 * class.  The standard library convention is to use raw pointers for this kind
 * of temporary pointer.  This is poor code semantics as it's difficult to
 * determine the intended use of the raw pointer without tracking down its
 * source.  Using TP makes the intent clear in the code.
 *
 * @see UP.h SP.h
 *
 * @copyright Copyright(C) Working Bits Systems, LLC 2023
 */
#ifndef _TP_H_
#define _TP_H_

#include "UP.h"
#include "SP.h"

/*
 * The Working Bits Systems namespace.
 */
namespace wbs
{
    /**
     * @class TP
     *
     * This class is used to pass around access to an object with the explicit
     * semantics that the ownership of the object is NOT passed.
     *
     * Like raw pointers, TPs don't try to keep track of the source of the
     * allocated memory.  When used with UPs and SPs, it is up to the developer
     * to ensure that the UP and SP(s) do not go out of scope and therefore free
     * the allocated memory while the TP is in use.  If such a guarantee can't
     * be made, the use of SP (or WP) is recommended.
     */
    template< class TYPETP > class TP
    {
    public:
        // Constructors, destructor, and Assignment operator -------------------
        /**
         * Default constructor.
         */
        TP< TYPETP >()
        {
            m_ptr = nullptr;
        }

        /**
         * Constructor that makes a (non-owning) TP from an UP for the same
         * object type.
         *
         * @param owner - the UP that "owns" the allocated object.
         */
        TP< TYPETP >( const UP< TYPETP >& owner )
        {
            // Yes, UnsafeAccess(), BUT being used to init the TP.  This is an
            // OK use of this method.
            m_ptr = owner.UnsafeAccess();
        }

        /**
         * Constructor that makes a (non-owning) TP from an SP for the same
         * object type.
         *
         * @param owner - the SP that shares ownership with the allocated object.
         */
        TP< TYPETP >( const SP< TYPETP >& owner )
        {
            // Yes, UnsafeAccess(), BUT being used to init the TP.  This is an
            // OK use of this method.
            m_ptr = owner.UnsafeAccess();
        }

        /**
         * Constructor that makes a (non-owning) TP from the std::unique_ptr for
         * the same object type.
         *
         * @param owner - the unique_ptr that "owns" the allocated object.
         *
         * @note Note that this constructor is required as allowing the auto-
         *       conversion from a unique_ptr to UP in the other constructor
         *       would end up transferring ownership to the UP and then it would
         *       be delete the memory after the constructor completes since it
         *       would be a temporary object.
         */
        TP< TYPETP >( const std::unique_ptr< TYPETP >& owner )
        {
            m_ptr = owner.get();
        }

        /**
         * Constructor that makes a (non-owning) TP from the std::shared_ptr for
         * the same object type.
         *
         * @param owner - a shared_ptr that is an ownor of the allocated object.
         */
        TP< TYPETP >( const std::shared_ptr< TYPETP >& owner )
        {
            m_ptr = owner.get();
        }

        /**
         * Copy constructor.
         *
         * @param other - another TP pointing to an object we also will point to
         *
         * @note Unlike UP, we can make as many TP copies as we want from other
         *       TPs, since TPs don't own, and therefore do not delete the
         *       object.
         */
        TP< TYPETP >( const TP< TYPETP >& other )
        {
            m_ptr = other.m_ptr;
        }

        /**
         * Constructor from a raw c-style pointer.
         *
         * @param cptr - A pointer to an object of type TYPETP to reference.
         *
         * @note This provides a useful transition from std convention of using
         *       raw pointers for non-owning pointers to TPs.  BUT, this class
         *       does no management of the object for any other cases.
         */
        TP< TYPETP >( const TYPETP* cptr )
        {
            m_ptr = const_cast< TYPETP* >( cptr );
        }

        /**
         * Destructor
         * @note - no memory deletion occurs here as this object does NOT own
         *         the referenced memory.
         */
        ~TP< TYPETP >()
        {
            // We don't care if this gets called.  We don't own the object, so
            // we can't delete it.
        }

        /**
         * Assignment operator.
         */
        TP< TYPETP >& operator=( const TP< TYPETP >& other )
        {
            m_ptr = other.m_ptr;

            return *this;
        }

        // Operations ----------------------------------------------------------
        /**
         * Pointer dereference operator.
         *
         * @return The pointer for dereferencing.
         */
        TYPETP* const operator->()
        {
            return m_ptr;
        }

        /**
         * Object reference operator.
         *
         * @return A reference to the object pointed to by this TP.
         */
        TYPETP& operator*()
        {
            return *m_ptr;
        }

       /*
        * Comparison operators.  Lumping these all together for ease.
        *
        * @param rhs - the right-hand-side of the comparison (this is lhs)
        *
        * @return true if this pointer has the relationship to the rhs as
        *         specified.
        */
       inline bool operator==( const TP< TYPETP >& other ) const { return RawAccess() == other.RawAccess(); }
       inline bool operator!=( const TP< TYPETP >& other ) const { return RawAccess() != other.RawAccess(); }
       inline bool operator<( const TP< TYPETP >& other ) const  { return RawAccess() < other.RawAccess(); }
       inline bool operator<=( const TP< TYPETP >& other ) const { return RawAccess() <= other.RawAccess(); }
       inline bool operator>( const TP< TYPETP >& other ) const  { return RawAccess() > other.RawAccess(); }
       inline bool operator>=( const TP< TYPETP >& other ) const { return RawAccess() >= other.RawAccess(); }

        // Methods -------------------------------------------------------------
        /**
         * Convert from this TP to a c-style raw pointer.
         *
         * @return The c-style raw pointer.
         */
        const TYPETP* RawAccess() const
        {
            return m_ptr;
        }

        /**
         * Clears the internal pointer.
         */
        void Release()
        {
            m_ptr = nullptr;
        }

        /**
         * Checks whether the internal pointer is null.
         *
         * @return true if the internal pointer is null, false otherwise.
         */
        bool IsNull()
        {
            return ( nullptr == m_ptr );
        }

    private:
        // Attributes ----------------------------------------------------------
        // Points to the object we manage or null.
        TYPETP* m_ptr;
    };

    /**
     * 2-operand comparison operators for pointer comparisons with UP.
     * Lumping these all together for ease.
     *
     * @param rhs - the right-hand-side of the comparison (this is lhs)
     *
     * @return true if this pointer has the relationship to the rhs as
     *         specified.
     */
    template < typename TYPE > bool operator==( const TP< TYPE >& lhs, const UP< TYPE >& rhs ) { return lhs.RawAccess() == rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator!=( const TP< TYPE >& lhs, const UP< TYPE >& rhs ) { return lhs.RawAccess() != rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator<( const TP< TYPE >& lhs, const UP< TYPE >& rhs ) { return lhs.RawAccess() < rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator<=( const TP< TYPE >& lhs, const UP< TYPE >& rhs ) { return lhs.RawAccess() <= rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator>( const TP< TYPE >& lhs, const UP< TYPE >& rhs ) { return lhs.RawAccess() > rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator>=( const TP< TYPE >& lhs, const UP< TYPE >& rhs ) { return lhs.RawAccess() >= rhs.UnsafeAccess(); }

    template < typename TYPE > bool operator==( const UP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() == rhs.RawAccess(); }
    template < typename TYPE > bool operator!=( const UP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() != rhs.RawAccess(); }
    template < typename TYPE > bool operator<( const UP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() < rhs.RawAccess(); }
    template < typename TYPE > bool operator<=( const UP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() <= rhs.RawAccess(); }
    template < typename TYPE > bool operator>( const UP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() > rhs.RawAccess(); }
    template < typename TYPE > bool operator>=( const UP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() >= rhs.RawAccess(); }

    template < typename TYPE > bool operator==( const TP< TYPE >& lhs, const SP< TYPE >& rhs ) { return lhs.RawAccess() == rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator!=( const TP< TYPE >& lhs, const SP< TYPE >& rhs ) { return lhs.RawAccess() != rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator<( const TP< TYPE >& lhs, const SP< TYPE >& rhs ) { return lhs.RawAccess() < rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator<=( const TP< TYPE >& lhs, const SP< TYPE >& rhs ) { return lhs.RawAccess() <= rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator>( const TP< TYPE >& lhs, const SP< TYPE >& rhs ) { return lhs.RawAccess() > rhs.UnsafeAccess(); }
    template < typename TYPE > bool operator>=( const TP< TYPE >& lhs, const SP< TYPE >& rhs ) { return lhs.RawAccess() >= rhs.UnsafeAccess(); }

    template < typename TYPE > bool operator==( const SP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() == rhs.RawAccess(); }
    template < typename TYPE > bool operator!=( const SP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() != rhs.RawAccess(); }
    template < typename TYPE > bool operator<( const SP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() < rhs.RawAccess(); }
    template < typename TYPE > bool operator<=( const SP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() <= rhs.RawAccess(); }
    template < typename TYPE > bool operator>( const SP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() > rhs.RawAccess(); }
    template < typename TYPE > bool operator>=( const SP< TYPE >& lhs, const TP< TYPE >& rhs ) { return lhs.UnsafeAccess() >= rhs.RawAccess(); }

}; // End of wbs namespace

#endif // #ifndef _TP_H_
