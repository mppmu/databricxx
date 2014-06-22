// Copyright (C) 2014 Oliver Schulz <oschulz@mpp.mpg.de>

// This is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#ifndef DBRX_FUNCPROG_H
#define DBRX_FUNCPROG_H

#include <iterator>
#include <cstddef>


namespace dbrx {


template<
	typename T,
	typename = decltype( *std::declval<T&>() ),
	typename = decltype( ++std::declval<T&>() )
> auto as_iterator(T&& x) -> decltype(std::forward<T>(x))
	{ return std::forward<T>(x); }


template<
	typename T,
	typename Iter = decltype( std::declval<T&>().begin() ),
	typename = decltype( as_iterator(std::declval<Iter>()) )
> auto as_collection(T&& x) -> decltype(std::forward<T>(x))
	{ return std::forward<T>(x); }



template<typename Iter, typename Fct> class MappedIterator {
protected:
	Iter m_iter;
	Fct* m_fct;

public:
	using value_type = typename std::remove_reference< decltype( (*m_fct)(*m_iter) ) >::type;
	using reference = value_type;
	using difference_type = decltype(m_iter - m_iter);
	using iterator_category = std::input_iterator_tag;

	MappedIterator& operator++() { ++m_iter; return *this; }
	MappedIterator operator++(int) { MappedIterator tmp(*this); operator++(); return tmp; }

	reference operator*() const { return (*m_fct)(*m_iter); }

	MappedIterator() noexcept: m_iter(), m_fct(nullptr) {}
	MappedIterator(Iter iter, Fct &fct) noexcept: m_iter(iter), m_fct(&fct) {}
	MappedIterator(const MappedIterator &other) noexcept: m_iter(other.m_iter), m_fct(other.m_fct) {}

	friend bool operator==(MappedIterator a, MappedIterator b)
		{ return (a.m_iter == b.m_iter) && (a.m_fct == b.m_fct); }

	friend bool operator!=(MappedIterator a, MappedIterator b)
		{ return ! operator==(a, b); }
};


template <
	typename Iter, typename Fct,
	typename = decltype( std::declval<Fct&>()(* (++std::declval<Iter&>()) ) )
> MappedIterator<Iter, const Fct> mapped(Iter iter, const Fct& fct) noexcept
	{ return MappedIterator<Iter, const Fct>( iter, fct ); }

template <
	typename Iter, typename Fct,
	typename = decltype( std::declval<Fct&>()(* (++std::declval<Iter&>()) ) )
> MappedIterator<Iter, Fct> mapped(Iter iter, Fct& fct) noexcept
	{ return MappedIterator<Iter, Fct>( iter, fct ); }



template<typename Coll, typename Fct> class MappedColl {
protected:
	const Coll* m_coll = nullptr;
	Fct* m_fct;

	using CollIter = decltype(m_coll->begin());
public:

	using const_iterator = MappedIterator<CollIter, Fct>;

	const_iterator begin() const noexcept { return const_iterator(m_coll->begin(), *m_fct); }

	const_iterator cbegin() const noexcept { return const_iterator(m_coll->begin(), *m_fct); }

	const_iterator end() const noexcept { return const_iterator(m_coll->end(), *m_fct); }

	const_iterator cend() const noexcept { return const_iterator(m_coll->end(), *m_fct); }

	MappedColl() noexcept: m_coll(nullptr), m_fct(nullptr) {}
	MappedColl(const Coll& coll, Fct &fct) noexcept: m_coll(&coll), m_fct(&fct) {}
	MappedColl(const MappedColl &other) noexcept: m_coll(other.m_coll), m_fct(other.m_fct) {}
};


template <
	typename Coll, typename Fct,
	typename = decltype( std::declval<Fct&>()(* (++std::declval<Coll&>().begin()) ) )
> MappedColl<Coll, const Fct> mapped(const Coll& coll, const Fct& fct) noexcept
	{ return MappedColl<Coll, const Fct>( coll, fct ); }

template <
	typename Coll, typename Fct,
	typename = decltype( std::declval<Fct&>()(* (++std::declval<Coll&>().begin()) ) )
> MappedColl<Coll, Fct> mapped(const Coll& coll, Fct& fct) noexcept
	{ return MappedColl<Coll, Fct>( coll, fct ); }


} // namespace dbrx

#endif // DBRX_FUNCPROG_H
