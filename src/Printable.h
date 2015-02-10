// Copyright (C) 2015 Oliver Schulz <oschulz@mpp.mpg.de>

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


#ifndef DBRX_PRINTABLE_H
#define DBRX_PRINTABLE_H

#include <string>
#include <iosfwd>


namespace dbrx {


class Printable {
public:
	virtual std::ostream& print(std::ostream &os) const = 0;

	virtual std::string toString() const;

	friend std::string to_string(const Printable &x) { return x.toString(); }
};


inline std::ostream& operator<<(std::ostream &os, const Printable &ref)
	{ return ref.print(os); }


} // namespace dbrx


#endif // DBRX_PRINTABLE_H
