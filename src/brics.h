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


template<typename T> class SimpleInput: public InputBric {
public:
	OutputValue<T> value{this, "value"};

	SimpleInput(const Name &n): InputBric(n) { value = T(); }
	SimpleInput(const Name &n, T v): InputBric(n) { value = std::move(v); }
};


class Add: public MapperBric {
public:
	Add(const Name &n): MapperBric(n) {}
};



class Mult: public MapperBric {
public:
	Mult(const Name &n): MapperBric(n) {}
};


} // namespace dbrx

#endif // DBRX_BRICS_H
