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


#ifndef DBRX_COLLBRICS_H
#define DBRX_COLLBRICS_H

#include <utility>

#include "Bric.h"


namespace dbrx {


template<typename Coll> class CollIterBric: public MapperBric {
public:
	Input<Coll> input{this};

protected:
	using Iter = decltype(input->begin());
	using QT = decltype(*std::declval<Iter&>());

	Iter m_iter{};

public:
	using T = typename std::remove_cv<typename std::remove_reference<QT>::type>::type;

	Output<T> element{this, "element"};
	Output<size_t> index{this, "index"};

	void processInput() {
		m_iter = input->begin();
		index = 0;
	}

	bool nextOutput() {
		if (m_iter != input->end()) {
			element = *m_iter;
			++m_iter;
			++index;
			return true;
		} else return false;
	}

	using MapperBric::MapperBric;
};



template<typename Coll> class CollBuilderBric: public ReducerBric {
public:
	Output<Coll> output{this};

protected:
	using Iter = decltype(std::declval<Coll&>().begin());
	using QT = decltype(*std::declval<Iter&>());

public:
	using T = typename std::remove_cv<typename std::remove_reference<QT>::type>::type;;

	Input<T> input{this};

	void newReduction() {
		output->clear();
	}

	void processInput() {
		output->push_back(input);
	}

	using ReducerBric::ReducerBric;
};


} // namespace dbrx

#endif // DBRX_COLLBRICS_H
