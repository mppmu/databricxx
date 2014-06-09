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


#ifndef DBRX_ROOTCOLLECTION_H
#define DBRX_ROOTCOLLECTION_H

#include <TCollection.h>


namespace dbrx {


template<typename T, bool writeable> class RootCollectionTmpl {
public:
	using CollPtr = typename std::conditional<writeable, TCollection*, const TCollection*>::type;

protected:
	CollPtr m_coll{nullptr};

public:
	template <bool canWrite> class Iterator {
	protected:
		TIter m_iter;

	public:
		using reference = typename std::conditional<canWrite, T&, const T&>::type;
		using value_type = typename std::conditional<canWrite, T, const T>::type;
		using pointer = typename std::conditional<canWrite, T*, const T*>::type;
		using iterator_category = typename std::conditional<canWrite, typename std::forward_iterator_tag, typename std::input_iterator_tag>::type;

		bool operator==(const Iterator &other) const { return ! operator!=(other); }
		bool operator!=(const Iterator &other) const { return m_iter != other.m_iter; }

		Iterator& operator++() { ++m_iter; return *this; }
		Iterator operator++(int) { TIter tmp(*this); operator++(); return tmp; }

		reference operator*() const { return * operator->(); }
		pointer operator->() const { return dynamic_cast<pointer>(*m_iter); }

		Iterator(): m_iter(TIter::End()) {}
		Iterator(const TIter& iter): m_iter(iter) {}
		Iterator(CollPtr coll): m_iter(coll) { m_iter.Begin(); }
	};

	using iterator = Iterator<true>;
	using const_iterator = Iterator<false>;

	iterator begin() noexcept { return iterator(m_coll); }

	const_iterator begin() const noexcept { return const_iterator(m_coll); }

	const_iterator cbegin() const noexcept { return const_iterator(m_coll); }

	iterator end() noexcept { return iterator(); }

	const_iterator end() const noexcept { return iterator(); }

	const_iterator cend() const noexcept { return iterator(); }

	RootCollectionTmpl() {}
	RootCollectionTmpl(CollPtr coll): m_coll(coll) {}
};


template<typename T> using RootCollection = RootCollectionTmpl<T, true>;
template<typename T> using ConstRootCollection = RootCollectionTmpl<T, false>;


template<typename T> RootCollection<T> stdcoll(TCollection* coll) { return RootCollection<T>(coll); }
template<typename T> ConstRootCollection<T> stdcoll(const TCollection* coll) { return ConstRootCollection<T>(coll); }


} // namespace dbrx

#endif // DBRX_ROOTCOLLECTION_H
