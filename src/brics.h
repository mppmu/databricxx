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


#ifndef DBRX_BRICS_H
#define DBRX_BRICS_H

#include <Bric.h>


namespace dbrx {


template<typename T> class SimpleInput: public ImportBric {
	bool m_hasNextOutput = true;
public:
	void import() {}

	Output<T> output{this};

	SimpleInput(Name n): ImportBric(n) { output = T(); }

	SimpleInput(Name n, T v): ImportBric(n) { output = std::move(v); }

	SimpleInput(Bric *parentBric, Name n, T v)
		: ImportBric(parentBric, n) { output = std::move(v); }
};



template<typename A, typename B, typename C> class Add: public TransformBric {
public:
	Input<A> a{this, "a"};
	Input<B> b{this, "b"};

	Output<C> result{this, "result"};

	void processInput() {
		result = a + b;
	}

	using TransformBric::TransformBric;
};



template<typename A, typename B, typename C> class Mult: public TransformBric {
public:
	Input<A> a{this, "a"};
	Input<B> b{this, "b"};

	Output<C> result{this, "result"};

	void processInput() {
		result = a * b;
	}

	using TransformBric::TransformBric;
};


} // namespace dbrx

#endif // DBRX_BRICS_H
