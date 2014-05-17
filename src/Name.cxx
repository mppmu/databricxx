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


#include "Name.h"

#include <cassert>

#include "NameTable.h"

using namespace std;


namespace dbrx {


const std::string Name::s_emptyString{};


Name::Name(const std::string &s): Name(s, NameTable::global()) {}


Name::Name(const std::string &s, NameTable &table) {
	*this = table.resolve(s);
	assert( empty() || (str().size() > 0) );
}


} // namespace dbrx
