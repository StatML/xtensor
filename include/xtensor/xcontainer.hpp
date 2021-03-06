/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCONTAINER_HPP
#define XCONTAINER_HPP

#include <numeric>
#include <functional>

#include "xstrides.hpp"
#include "xiterable.hpp"
#include "xiterator.hpp"
#include "xoperation.hpp"
#include "xmath.hpp"
#include "xtensor_forward.hpp"

namespace xt
{

    namespace check_policy
    {
        struct none {};
        struct full {};
    }

    template <class D>
    struct xcontainer_iterable_types
    {
        using shape_type = typename xcontainer_inner_types<D>::shape_type;
        using container_type = typename xcontainer_inner_types<D>::container_type;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using stepper = xstepper<D>;
        using const_stepper = xstepper<const D>;
        using broadcast_iterator = xiterator<stepper, shape_type*>;
        using const_broadcast_iterator = xiterator<const_stepper, shape_type*>;
    };

    /**
     * @class xcontainer
     * @brief Base class for dense multidimensional containers.
     *
     * The xcontainer class defines the interface for dense multidimensional
     * container classes. It does not embed any data container, this responsibility
     * is delegated to the inheriting classes.
     *
     * @tparam D The derived type, i.e. the inheriting class for which xcontainer
     *           provides the interface.
     */
    template <class D>
    class xcontainer : public xiterable<D>
    {

    public:

        using derived_type = D;

        using inner_types = xcontainer_inner_types<D>;
        using container_type = typename inner_types::container_type;
        using value_type = typename container_type::value_type;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;
        using pointer = typename container_type::pointer;
        using const_pointer = typename container_type::const_pointer;
        using size_type = typename container_type::size_type;
        using difference_type = typename container_type::difference_type;

        using shape_type = typename inner_types::shape_type;
        using strides_type = typename inner_types::strides_type;

        using iterable_base = xiterable<D>;

        using iterator = typename iterable_base::iterator;
        using const_iterator = typename iterable_base::const_iterator;

        using stepper = typename iterable_base::stepper;
        using const_stepper = typename iterable_base::const_stepper;

        using broadcast_iterator = typename iterable_base::broadcast_iterator;
        using const_broadcast_iterator = typename iterable_base::broadcast_iterator;

        size_type size() const noexcept;

        size_type dimension() const noexcept;

        const shape_type& shape() const noexcept;
        const strides_type& strides() const noexcept;
        const strides_type& backstrides() const noexcept;

        void reshape(const shape_type& shape);
        void reshape(const shape_type& shape, layout l);
        void reshape(const shape_type& shape, const strides_type& strides);

        void transpose();

        template <class S, class Tag = check_policy::none>
        void transpose(S&& permutation, Tag check_policy = Tag());

#ifdef X_OLD_CLANG
        template <class I, class Tag = check_policy::none>
        void transpose(std::initializer_list<I> permutation, Tag check_policy = Tag());
#else
        template <class I, std::size_t N, class Tag = check_policy::none>
        void transpose(const I(&permutation)[N], Tag check_policy = Tag());
#endif

        template <class... Args>
        reference operator()(Args... args);

        template <class... Args>
        const_reference operator()(Args... args) const;

        reference operator[](const xindex& index);
        reference operator[](size_type i);
        const_reference operator[](const xindex& index) const;
        const_reference operator[](size_type i) const;

        template <class It>
        reference element(It first, It last);
        template <class It>
        const_reference element(It first, It last) const;

        container_type& data() noexcept;
        const container_type& data() const noexcept;

        template <class S>
        bool broadcast_shape(S& shape) const;

        template <class S>
        bool is_trivial_broadcast(const S& strides) const noexcept;

        iterator begin() noexcept;
        iterator end() noexcept;

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;
        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;

        template <class S>
        stepper stepper_begin(const S& shape) noexcept;
        template <class S>
        stepper stepper_end(const S& shape) noexcept;

        template <class S>
        const_stepper stepper_begin(const S& shape) const noexcept;
        template <class S>
        const_stepper stepper_end(const S& shape) const noexcept;

    protected:

        xcontainer() = default;
        ~xcontainer() = default;

        xcontainer(const xcontainer&) = default;
        xcontainer& operator=(const xcontainer&) = default;

        xcontainer(xcontainer&&) = default;
        xcontainer& operator=(xcontainer&&) = default;

        shape_type& get_shape() noexcept;
        strides_type& get_strides() noexcept;
        strides_type& get_backstrides() noexcept;

    private:

        template <class S>
        void transpose_impl(S&& permutation, check_policy::none);

        template <class S>
        void transpose_impl(S&& permutation, check_policy::full);

        shape_type m_shape;
        strides_type m_strides;
        strides_type m_backstrides;
    };

    /******************************
     * xcontainer implementation *
     ******************************/

    template <class D>
    inline auto xcontainer<D>::get_shape() noexcept -> shape_type&
    {
        return m_shape;
    }

    template <class D>
    inline auto xcontainer<D>::get_strides() noexcept -> strides_type&
    {
        return m_strides;
    }

    template <class D>
    inline auto xcontainer<D>::get_backstrides() noexcept -> strides_type&
    {
        return m_backstrides;
    }

    /**
     * @name Size and shape
     */
    //@{
    /**
     * Returns the number of element in the container.
     */
    template <class D>
    inline auto xcontainer<D>::size() const noexcept -> size_type
    {
        return data().size();
    }

    /**
     * Returns the number of dimensions of the container.
     */
    template <class D>
    inline auto xcontainer<D>::dimension() const noexcept -> size_type
    {
        return m_shape.size();
    }

    /**
     * Returns the shape of the container.
     */
    template <class D>
    inline auto xcontainer<D>::shape() const noexcept -> const shape_type&
    {
        return m_shape;
    }

    /**
     * Returns the strides of the container.
     */
    template <class D>
    inline auto xcontainer<D>::strides() const noexcept -> const strides_type&
    {
        return m_strides;
    }

    /**
     * Returns the backstrides of the container.
     */
    template <class D>
    inline auto xcontainer<D>::backstrides() const noexcept -> const strides_type&
    {
        return m_backstrides;
    }

    /**
     * Reshapes the container.
     * @param shape the new shape
     */
    template <class D>
    inline void xcontainer<D>::reshape(const shape_type& shape)
    {
        if(shape != m_shape)
        {
            reshape(shape, layout::row_major);
        }
    }

    /**
     * Reshapes the container.
     * @param shape the new shape
     * @param l the new layout
     */
    template <class D>
    inline void xcontainer<D>::reshape(const shape_type& shape, layout l)
    {
        m_shape = shape;
        resize_container(m_strides, m_shape.size());
        resize_container(m_backstrides, m_shape.size());
        size_type data_size = compute_strides(m_shape, l, m_strides, m_backstrides);
        data().resize(data_size);
    }

    /**
     * Reshapes the container.
     * @param shape the new shape
     * @param strides the new strides
     */
    template <class D>
    inline void xcontainer<D>::reshape(const shape_type& shape, const strides_type& strides)
    {
        m_shape = shape;
        m_strides = strides;
        resize_container(m_backstrides, m_strides.size());
        adapt_strides(m_shape, m_strides, m_backstrides);
        data().resize(compute_size(this->shape()));
    }

    /**
     * Transposes the container inplace by reversing the dimensions.
     */
    template <class D>
    inline void xcontainer<D>::transpose()
    {
        // reverse stride and shape
        std::reverse(m_shape.begin(), m_shape.end());
        std::reverse(m_strides.begin(), m_strides.end());
        std::reverse(m_backstrides.begin(), m_backstrides.end());
    }

    /**
     * Transposes the container inplace by permuting the shape with @p permutation.
     * @param permutation the sequence containing permutation
     * @param check_policy the check level (check_policy::full() or check_policy::none())
     * @tparam Tag selects the level of error checking on permutation vector defaults to check_policy::none.
     */
    template <class D>
    template <class S, class Tag>
    inline void xcontainer<D>::transpose(S&& permutation, Tag check_policy)
    {
        transpose_impl(std::forward<S>(permutation), check_policy);
    }

#ifdef X_OLD_CLANG
    template <class D>
    template <class I, class Tag>
    inline void xcontainer<D>::transpose(std::initializer_list<I> permutation, Tag check_policy)
    {
        std::vector<I> perm(permutation);
        transpose_impl(std::move(perm), check_policy);
    }
#else
    template <class D>
    template <class I, std::size_t N, class Tag>
    inline void xcontainer<D>::transpose(const I(&permutation)[N], Tag check_policy)
    {
        transpose_impl(permutation, check_policy);
    }
#endif

    template <class D>
    template <class S>
    inline void xcontainer<D>::transpose_impl(S&& permutation, check_policy::full)
    {
        // check if axis appears twice in permutation
        for (size_type i = 0; i < container_size(permutation); ++i)
        {
            for (size_type j = i + 1; j < container_size(permutation); ++j)
            {
                if (permutation[i] == permutation[j])
                {
                    throw transpose_error("Permutation contains axis more than once");
                }
            }
        }
        transpose_impl(permutation, check_policy::none());
    }

    template <class D>
    template <class S>
    inline void xcontainer<D>::transpose_impl(S&& permutation, check_policy::none)
    {
        if (container_size(permutation) != m_shape.size())
        {
            throw transpose_error("Permutation does not have the same size as shape");
        }

        // permute stride and shape
        strides_type temp_strides;
        resize_container(temp_strides, m_strides.size());

        shape_type temp_shape;
        resize_container(temp_shape, m_shape.size());

        shape_type temp_backstrides;
        resize_container(temp_backstrides, m_backstrides.size());

        for (size_type i = 0; i < m_shape.size(); ++i)
        {
            if (size_type(permutation[i]) >= dimension())
            {
                throw transpose_error("Permutation contains wrong axis");
            }
            temp_shape[i] = m_shape[permutation[i]];
            temp_strides[i] = m_strides[permutation[i]];
            temp_backstrides[i] = m_backstrides[permutation[i]];
        }
        m_shape = std::move(temp_shape);
        m_strides = std::move(temp_strides);
        m_backstrides = std::move(temp_backstrides);
    }
    //@}


    /**
     * @name Data
     */
    //@{
    /**
     * Returns a reference to the element at the specified position in the container.
     * @param args a list of indices specifying the position in the container. Indices
     * must be unsigned integers, the number of indices should be equal or greater than
     * the number of dimensions of the container.
     */
    template <class D>
    template <class... Args>
    inline auto xcontainer<D>::operator()(Args... args) -> reference
    {
        size_type index = data_offset<size_type>(m_strides, static_cast<size_type>(args)...);
        return data()[index];
    }

    /**
     * Returns a constant reference to the element at the specified position in the container.
     * @param args a list of indices specifying the position in the container. Indices
     * must be unsigned integers, the number of indices should be equal or greater than
     * the number of dimensions of the container.
     */
    template <class D>
    template <class... Args>
    inline auto xcontainer<D>::operator()(Args... args) const -> const_reference
    {
        size_type index = data_offset<size_type>(m_strides, static_cast<size_type>(args)...);
        return data()[index];
    }

    /**
     * Returns a reference to the element at the specified position in the container.
     * @param index a sequence of indices specifying the position in the container. Indices
     * must be unsigned integers, the number of indices in the list should be equal or greater
     * than the number of dimensions of the container.
     */
    template <class D>
    inline auto xcontainer<D>::operator[](const xindex& index) -> reference
    {
        return element(index.cbegin(), index.cend());
    }

    template <class D>
    inline auto xcontainer<D>::operator[](size_type i) -> reference
    {
        return operator()(i);
    }

    /**
     * Returns a constant reference to the element at the specified position in the container.
     * @param index a sequence of indices specifying the position in the container. Indices
     * must be unsigned integers, the number of indices in the list should be equal or greater
     * than the number of dimensions of the container.
     */
    template <class D>
    inline auto xcontainer<D>::operator[](const xindex& index) const -> const_reference
    {
        return element(index.cbegin(), index.cend());
    }

    template <class D>
    inline auto xcontainer<D>::operator[](size_type i) const -> const_reference
    {
        return operator()(i);
    }

    /**
     * Returns a reference to the element at the specified position in the container.
     * @param first iterator starting the sequence of indices
     * @param last iterator ending the sequence of indices
     * The number of indices in the squence should be equal to or greater
     * than the number of dimensions of the container.
     */
    template <class D>
    template <class It>
    inline auto xcontainer<D>::element(It first, It last) -> reference
    {
        return data()[element_offset<size_type>(m_strides, first, last)];
    }

    /**
     * Returns a reference to the element at the specified position in the container.
     * @param first iterator starting the sequence of indices
     * @param last iterator ending the sequence of indices
     * The number of indices in the squence should be equal to or greater
     * than the number of dimensions of the container.
     */
    template <class D>
    template <class It>
    inline auto xcontainer<D>::element(It first, It last) const -> const_reference
    {
        return data()[element_offset<size_type>(m_strides, first, last)];
    }

    /**
     * Returns a reference to the buffer containing the elements of the container.
     */
    template <class D>
    inline auto xcontainer<D>::data() noexcept -> container_type&
    {
        return static_cast<derived_type*>(this)->data_impl();
    }

    /**
     * Returns a constant reference to the buffer containing the elements of the
     * container.
     */
    template <class D>
    inline auto xcontainer<D>::data() const noexcept -> const container_type&
    {
        return static_cast<const derived_type*>(this)->data_impl();
    }
    //@}

    /**
     * @name Broadcasting
     */
    //@{
    /**
     * Broadcast the shape of the container to the specified parameter.
     * @param shape the result shape
     * @return a boolean indicating whether the broadcasting is trivial
     */
    template <class D>
    template <class S>
    inline bool xcontainer<D>::broadcast_shape(S& shape) const
    {
        return xt::broadcast_shape(m_shape, shape);
    }

    /**
     * Compares the specified strides with those of the container to see whether
     * the broadcasting is trivial.
     * @return a boolean indicating whether the broadcasting is trivial
     */
    template <class D>
    template <class S>
    inline bool xcontainer<D>::is_trivial_broadcast(const S& str) const noexcept
    {
        return str.size() == strides().size() &&
            std::equal(str.cbegin(), str.cend(), strides().begin());
    }
    //@}

    /****************
     * iterator api *
     ****************/

    /**
     * @name Iterators
     */
    //@{
    /**
     * Returns an iterator to the first element of the buffer containing
     * the elements of the container.
     */
    template <class D>
    inline auto xcontainer<D>::begin() noexcept -> iterator
    {
        return data().begin();
    }

    /**
     * Returns an iterator to the element following the last element of
     * the buffer containing the elements of the container.
     */
    template <class D>
    inline auto xcontainer<D>::end() noexcept -> iterator
    {
        return data().end();
    }

    /**
     * Returns a constant iterator to the first element of the buffer
     * containing the elements of the container.
     */
    template <class D>
    inline auto xcontainer<D>::begin() const noexcept -> const_iterator
    {
        return cbegin();
    }

    /**
     * Returns a constant iterator to the element following the last
     * element of the buffer containing the elements of the container.
     */
    template <class D>
    inline auto xcontainer<D>::end() const noexcept -> const_iterator
    {
        return cend();
    }

    /**
     * Returns a constant iterator to the first element of the buffer
     * containing the elements of the container.
     */
    template <class D>
    inline auto xcontainer<D>::cbegin() const noexcept -> const_iterator
    {
        return data().cbegin();
    }

    /**
     * Returns a constant iterator to the element following the last
     * element of the buffer containing the elements of the container.
     */
    template <class D>
    inline auto xcontainer<D>::cend() const noexcept -> const_iterator
    {
        return data().cend();
    }
    //@}

    /***************
     * stepper api *
     ***************/

    template <class D>
    template <class S>
    inline auto xcontainer<D>::stepper_begin(const S& shape) noexcept -> stepper
    {
        size_type offset = shape.size() - dimension();
        return stepper(static_cast<derived_type*>(this), data().begin(), offset);
    }

    template <class D>
    template <class S>
    inline auto xcontainer<D>::stepper_end(const S& shape) noexcept -> stepper
    {
        size_type offset = shape.size() - dimension();
        return stepper(static_cast<derived_type*>(this), data().end(), offset);
    }

    template <class D>
    template <class S>
    inline auto xcontainer<D>::stepper_begin(const S& shape) const noexcept -> const_stepper
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(static_cast<const derived_type*>(this), data().begin(), offset);
    }

    template <class D>
    template <class S>
    inline auto xcontainer<D>::stepper_end(const S& shape) const noexcept -> const_stepper
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(static_cast<const derived_type*>(this), data().end(), offset);
    }

}

#endif

