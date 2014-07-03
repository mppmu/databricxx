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


#ifndef DBRX_FUNCBRICS_H
#define DBRX_FUNCBRICS_H

#include "Bric.h"


namespace dbrx {


template<typename R, typename A> class UnaryFunctionBric: public TransformBric {
public:
	Input<A> input{this};
	Output<R> output{this};

	using TransformBric::TransformBric;
};



template<typename R, typename A, typename B> class BinaryFunctionBric: public TransformBric {
public:
	Input<A> a{this, "a"};
	Input<B> b{this, "b"};

	Output<R> output{this};

	using TransformBric::TransformBric;
};



template<typename R, typename A, typename B> struct Adder final: public BinaryFunctionBric<R, A, B> {
	void processInput() override { this->output = this->a + this->b; }
	using BinaryFunctionBric<R, A, B>::BinaryFunctionBric;
};


template<typename R, typename A, typename B> struct Subtractor final: public BinaryFunctionBric<R, A, B> {
	void processInput() override { this->output = this->a - this->b; }
	using BinaryFunctionBric<R, A, B>::BinaryFunctionBric;
};


template<typename R, typename A, typename B> struct Multiplier final: public BinaryFunctionBric<R, A, B> {
	void processInput() override { this->output = this->a * this->b; }
	using BinaryFunctionBric<R, A, B>::BinaryFunctionBric;
};


template<typename R, typename A, typename B> struct Divider final: public BinaryFunctionBric<R, A, B> {
	void processInput() override { this->output = this->a / this->b; }
	using BinaryFunctionBric<R, A, B>::BinaryFunctionBric;
};


} // namespace dbrx

#endif // DBRX_FUNCBRICS_H
