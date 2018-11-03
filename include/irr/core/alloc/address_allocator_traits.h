// Copyright (C) 2018 Mateusz 'DevSH' Kielan
// This file is part of the "IrrlichtBAW Engine"
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_ADDRESS_ALLOCATOR_TYPE_TRAITS_H_INCLUDED__
#define __IRR_ADDRESS_ALLOCATOR_TYPE_TRAITS_H_INCLUDED__

namespace irr
{
namespace core
{

    template<typename AddressType>
    struct address_type_traits;

    template<>
    struct address_type_traits<uint32_t>
    {
        address_type_traits() = delete;
        static constexpr uint32_t   invalid_address = 0xdeadbeefu;
    };

    template<>
    struct address_type_traits<uint64_t>
    {
        address_type_traits() = delete;
        static constexpr uint64_t   invalid_address = 0xdeadbeefBADC0FFEull;
    };


    namespace impl
    {
        template<class AddressAlloc, bool hasAttribute>
        struct address_allocator_traits_base;
        //provide default traits
        template<class AddressAlloc>
        struct address_allocator_traits_base<AddressAlloc,false>
        {
            static constexpr bool       supportsArbitraryOrderFrees     = true;
            static constexpr uint32_t   maxMultiOps                     = 256u;
            static constexpr bool       supportsNullBuffer              = false;

            typedef typename AddressAlloc::size_type size_type;

            static inline void         multi_alloc_addr(AddressAlloc& alloc, uint32_t count, size_type* outAddresses, const size_type* bytes,
                                                        const size_type* alignment, const size_type* hint=nullptr) noexcept
            {
                for (uint32_t i=0; i<count; i++)
                {
                    if (outAddresses[i]!=AddressAlloc::invalid_address)
                        continue;

                    outAddresses[i] = alloc.alloc_addr(bytes[i],alignment[i],hint ? hint[i]:0ull);
                }
            }

            static inline void         multi_free_addr(AddressAlloc& alloc, uint32_t count, const size_type* addr, const size_type* bytes) noexcept
            {
                for (uint32_t i=0; i<count; i++)
                {
                    if (addr[i]==AddressAlloc::invalid_address)
                        continue;

                    alloc.free_addr(addr[i],bytes[i]);
                }
            }


            static inline size_type    get_real_addr(const AddressAlloc& alloc, size_type allocated_addr)
            {
                return allocated_addr;
            }
        };
        //forward existing traits
        template<class AddressAlloc>
        struct address_allocator_traits_base<AddressAlloc,true>
        {
            static constexpr bool       supportsArbitraryOrderFrees     = AddressAlloc::supportsArbitraryOrderFrees;
            static constexpr uint32_t   maxMultiOps                     = AddressAlloc::maxMultiOps;
            /// C++17 inline static constexpr bool supportsNullBuffer   = AddressAlloc::supportsNullBuffer;

            typedef typename AddressAlloc::size_type size_type;

            template<typename... Args>
            static inline void         multi_alloc_addr(AddressAlloc& alloc, Args&&... args) noexcept
            {
                alloc.multi_alloc_addr(std::forward<Args>(args)...);
            }

            template<typename... Args>
            static inline void         multi_free_addr(AddressAlloc& alloc, Args&&... args) noexcept
            {
                alloc.multi_free_addr(std::forward<Args>(args)...);
            }


            static inline size_type    get_real_addr(const AddressAlloc& alloc, size_type allocated_addr)
            {
                return alloc.get_real_addr(allocated_addr);
            }
        };
    }


    //! TODO: https://en.cppreference.com/w/cpp/experimental/is_detected
    template<class AddressAlloc>
    class address_allocator_traits : protected AddressAlloc //maybe private?
    {
        public:
            typedef AddressAlloc                        allocator_type;
            typedef typename AddressAlloc::size_type    size_type;
        private:
            class ConstGetter : public AddressAlloc
            {
                    ConstGetter() = delete;
                    virtual ~ConstGetter() = default;
                public:
                    inline void*        getBufferStart() const noexcept     {return AddressAlloc::getBufferStart();}
                    inline const void*  getReservedSpacePtr() const noexcept{return AddressAlloc::getReservedSpacePtr();}
                    inline size_type    max_size() const noexcept           {return AddressAlloc::max_size();}
                    inline size_type    min_size() const noexcept           {return AddressAlloc::min_size();}
                    inline size_type    max_alignment() const noexcept      {return AddressAlloc::max_alignment();}
                    inline size_type    get_align_offset() const noexcept   {return AddressAlloc::get_align_offset();}
                    inline size_type    get_free_size() const noexcept      {return AddressAlloc::get_free_size();}
                    inline size_type    get_allocated_size() const noexcept {return AddressAlloc::get_allocated_size();}
                    inline size_type    get_total_size() const noexcept     {return AddressAlloc::get_total_size();}
            };
            virtual ~address_allocator_traits() = default;

            template<class U> using cstexpr_supportsArbitraryOrderFrees = decltype(std::declval<U&>().supportsArbitraryOrderFrees);
            template<class U> using cstexpr_maxMultiOps                 = decltype(std::declval<U&>().maxMultiOps);
            template<class U> using cstexpr_supportsNullBuffer          = decltype(std::declval<U&>().supportsNullBuffer);

            template<class U> using func_multi_alloc_addr               = decltype(std::declval<U&>().multi_alloc_addr(0u,nullptr,nullptr,nullptr,nullptr));
            template<class U> using func_multi_free_addr                = decltype(std::declval<U&>().multi_free_addr(0u,nullptr,nullptr));

            template<class U> using template_func_multi_alloc_addr      = decltype(std::declval<U&>().template multi_alloc_addr(0u,nullptr,nullptr,nullptr,nullptr));
            template<class U> using template_func_multi_free_addr       = decltype(std::declval<U&>().template multi_free_addr(0u,nullptr,nullptr));

            template<class U> using func_get_real_addr                  = decltype(std::declval<U&>().get_real_addr(0u));
        /// C++17 protected:
        public:
            template<class,class=void> struct has_supportsArbitraryOrderFrees                   : std::false_type {};
            template<class,class=void> struct has_maxMultiOps                                   : std::false_type {};
            template<class,class=void> struct has_supportsNullBuffer                            : std::false_type {};

            template<class,class=void> struct has_func_multi_alloc_addr                         : std::false_type {};
            template<class,class=void> struct has_func_multi_free_addr                          : std::false_type {};
            template<class,class=void> struct has_template_func_multi_alloc_addr                : std::false_type {};
            template<class,class=void> struct has_template_func_multi_free_addr                 : std::false_type {};

            template<class,class=void> struct has_func_get_real_addr                            : std::false_type {};


            template<class U> struct has_supportsArbitraryOrderFrees<U,void_t<cstexpr_supportsArbitraryOrderFrees<U> > >
                                                                                                : std::is_same<cstexpr_supportsArbitraryOrderFrees<U>,void> {};
            template<class U> struct has_maxMultiOps<U,void_t<cstexpr_maxMultiOps<U> > >        : std::is_same<cstexpr_maxMultiOps<U>,void> {};
            template<class U> struct has_supportsNullBuffer<U,void_t<cstexpr_supportsNullBuffer<U> > >
                                                                                                : std::is_same<cstexpr_supportsNullBuffer<U>,void> {};

            template<class U> struct has_func_multi_alloc_addr<U,void_t<func_multi_alloc_addr<U> > >
                                                                                                : std::is_same<func_multi_alloc_addr<U>,void> {};
            template<class U> struct has_func_multi_free_addr<U,void_t<func_multi_free_addr<U> > >
                                                                                                : std::is_same<func_multi_free_addr<U>,void> {};
            template<class U> struct has_template_func_multi_alloc_addr<U,void_t<func_multi_alloc_addr<U> > >
                                                                                                : std::is_same<template_func_multi_alloc_addr<U>,void> {};
            template<class U> struct has_template_func_multi_free_addr<U,void_t<func_multi_free_addr<U> > >
                                                                                                : std::is_same<template_func_multi_free_addr<U>,void> {};

            template<class U> struct has_func_get_real_addr<U,void_t<func_get_real_addr<U> > >  : std::is_same<func_get_real_addr<U>,void> {};
        /// C++17 public:
            /// C++17 inline static constexpr bool supportsArbitraryOrderFrees  = impl::address_allocator_traits_base<AddressAlloc,has_supportsArbitraryOrderFrees<AddressAlloc>::value>::supportsArbitraryOrderFrees;
            /// C++17 inline static constexpr uint32_t maxMultiOps              = impl::address_allocator_traits_base<AddressAlloc,has_maxMultiOps<AddressAlloc>::value>::maxMultiOps;
            /// C++17 inline static constexpr bool supportsNullBuffer           = impl::address_allocator_traits_base<AddressAlloc,has_supportsNullBuffer<AddressAlloc>::value>::supportsNullBuffer;


            using AddressAlloc::AddressAlloc;
/*
            template<typename... Args>
            address_allocator_traits(void* reservedSpc, size_t alignOff, typename AddressAlloc::size_type bufSz, Args&&... args) noexcept :
                    AddressAlloc(reservedSpc, nullptr, alignOff, bufSz, std::forward<Args>(args)...)
            {
            }
*/

            static inline size_type        get_real_addr(const AddressAlloc& alloc, size_type allocated_addr) noexcept
            {
                return impl::address_allocator_traits_base<AddressAlloc,has_func_get_real_addr<AddressAlloc>::value>::get_real_addr(allocated_addr);
            }


            //!
            /** Warning outAddresses needs to be primed with `invalid_address` values,
            otherwise no allocation happens for elements not equal to `invalid_address`. */
            static inline void              multi_alloc_addr(AddressAlloc& alloc, uint32_t count, size_type* outAddresses,
                                                             const size_type* bytes, const size_type* alignment, const size_type* hint=nullptr) noexcept
            {
                constexpr uint32_t maxMultiOps = impl::address_allocator_traits_base<AddressAlloc,has_maxMultiOps<AddressAlloc>::value>::maxMultiOps;
                for (uint32_t i=0; i<count; i+=maxMultiOps)
                    impl::address_allocator_traits_base<AddressAlloc,has_func_multi_alloc_addr<AddressAlloc>::value||has_template_func_multi_alloc_addr<AddressAlloc>::value>::multi_alloc_addr(
                                                                alloc,std::min(count-i,maxMultiOps),outAddresses+i,bytes+i,alignment+i,hint ? (hint+i):nullptr);
            }

            static inline void             multi_free_addr(AddressAlloc& alloc, uint32_t count, const size_type* addr, const size_type* bytes) noexcept
            {
                constexpr uint32_t maxMultiOps = impl::address_allocator_traits_base<AddressAlloc,has_maxMultiOps<AddressAlloc>::value>::maxMultiOps;
                for (uint32_t i=0; i<count; i+=maxMultiOps)
                    impl::address_allocator_traits_base<AddressAlloc,has_func_multi_free_addr<AddressAlloc>::value||has_template_func_multi_free_addr<AddressAlloc>::value>::multi_free_addr(
                                                                alloc,std::min(count-i,maxMultiOps),addr+i,bytes+i);
            }


            static inline void*             getBufferStart(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).getBufferStart();
            }
            static inline const void*       getReservedSpacePtr(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).getReservedSpacePtr();
            }


            static inline size_type        max_size(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).max_size();
            }
            static inline size_type        min_size(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).min_size();
            }
            static inline size_type        max_alignment(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).max_alignment();
            }
            static inline size_type        get_align_offset(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).get_align_offset();
            }


            static inline size_type        get_free_size(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).get_free_size();
            }
            static inline size_type        get_allocated_size(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).get_allocated_size();
            }
            static inline size_type        get_total_size(const AddressAlloc& alloc) noexcept
            {
                return static_cast<const ConstGetter&>(alloc).get_total_size();
            }
    };

}
}

#endif // __IRR_ADDRESS_ALLOCATOR_TYPE_TRAITS_H_INCLUDED__