/**
 * @file 
 * @brief A simplified replacement for std::span, since AVR doesn't have a standard library
 */
#pragma once
#include <stdint.h>

constexpr size_t dynamic_extent = SIZE_MAX;

namespace detail {

template <typename E, size_t S>
struct span_storage {
    constexpr span_storage() noexcept = default;

    constexpr span_storage(E* p_ptr, size_t /*unused*/) noexcept
       : ptr(p_ptr)
    {}

    E* ptr = nullptr;
    static constexpr size_t size = S;
};

template <typename E>
struct span_storage<E, dynamic_extent> {
    constexpr span_storage() noexcept = default;

    constexpr span_storage(E* p_ptr, size_t p_size) noexcept
        : ptr(p_ptr), size(p_size)
    {}

    E* ptr = nullptr;
    size_t size = 0;
};

}

template <typename ElementType, size_t Extent>
struct span {
    using storage_type = detail::span_storage<ElementType, Extent>;

    // constants and types
    using element_type = ElementType;
    using value_type = ElementType;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;
    using iterator = pointer;
    using reverse_iterator = pointer;


    // [span.cons], span constructors, copy, assignment, and destructor
    constexpr span() noexcept
    {       
    }

    constexpr span(pointer ptr, size_type count)
        : storage_(ptr, count)
    {
    }

    constexpr span(pointer first_elem, pointer last_elem)
        : storage_(first_elem, last_elem - first_elem)
    {
    }

    template <size_t N>
    constexpr span(element_type (&arr)[N]) noexcept 
        : storage_(arr, N)
    {       
    }

    constexpr span(const span& other) noexcept = default;

    ~span() noexcept = default;    
 
    span& operator=(const span& other) noexcept = default;

    // [span.sub], span subviews
    template <size_t Count>
    span<element_type, Count> first() const
    {
        return {data(), Count};
    }

    template <size_t Count>
    span<element_type, Count> last() const
    {
        return {data() + (size() - Count), Count};
    }

    template <size_t Offset, size_t Count = dynamic_extent>
    using subspan_return_t =
        span<ElementType, Count != dynamic_extent
                              ? Count
                              : (Extent != dynamic_extent ? Extent - Offset
                                                          : dynamic_extent)>;

    template <size_t Offset, size_t Count = dynamic_extent>
    subspan_return_t<Offset, Count> subspan() const
    {
        return {data() + Offset,
                Count != dynamic_extent ? Count : size() - Offset};
    }

    span<element_type, dynamic_extent> first(size_type count) const
    {
        return {data(), count};
    }

    span<element_type, dynamic_extent> last(size_type count) const
    {
        return {data() + (size() - count), count};
    }

    span<element_type, dynamic_extent> subspan(size_type offset, size_type count = dynamic_extent) const
    {
        return {data() + offset,
                count == dynamic_extent ? size() - offset : count};
    }

    // [span.obs], span observers
    constexpr size_type size() const noexcept { return storage_.size; }

    constexpr size_type size_bytes() const noexcept
    {
        return size() * sizeof(element_type);
    }

    constexpr bool empty() const noexcept
    {
        return size() == 0;
    }

    // [span.elem], span element access
    reference operator[](size_type idx) const
    {
        return *(data() + idx);
    }

    reference front() const
    {
        return *data();
    }

    reference back() const
    {
        return *(data() + (size() - 1));
    }

    constexpr pointer data() const noexcept { return storage_.ptr; }

    // [span.iterators], span iterator support
    constexpr iterator begin() const noexcept { return data(); }

    constexpr iterator end() const noexcept { return data() + size(); }

    reverse_iterator rbegin() const noexcept
    {
        return reverse_iterator(end());
    }

    reverse_iterator rend() const noexcept
    {
        return reverse_iterator(begin());
    }

private:
    storage_type storage_{};    
};