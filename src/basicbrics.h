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


#ifndef DBRX_BASICBRICS_H
#define DBRX_BASICBRICS_H

#include "Bric.h"
#include "format.h"


namespace dbrx {


template<typename T> class ConstBric final: public ImportBric {
public:
	Param<T> value{this, "value"};

	Output<T> output{this};

	void import() override { output = value; }

	ConstBric() { }

	ConstBric(Name n): ImportBric(n) {  }

	ConstBric(Name n, T v): ImportBric(n) { value = std::move(v); }

	ConstBric(Bric *parentBric, Name n, T v)
		: ImportBric(parentBric, n) { value = std::move(v); }
};



template<typename From, typename To> class ConvertBric final: public TransformBric {
public:
	Input<From> input{this};
	Output<To> output{this};

	void processInput() override { assign_from(output.get(), input.get()); }

	using TransformBric::TransformBric;
};



template<typename T> class CopyBric final: public TransformBric {
public:
	Input<T> input{this};
	Output<T> output{this};

	void processInput() override { output.value() = input.value(); }

	using TransformBric::TransformBric;
};


} // namespace dbrx

#endif // DBRX_BASICBRICS_H
