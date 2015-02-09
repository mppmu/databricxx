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


#ifndef DBRX_ROOTIO_H
#define DBRX_ROOTIO_H

#include <TTree.h>

#include "Value.h"


namespace dbrx {


/// @brief Class for static ROOT I/O methods

class RootIO {
public:
	static char getTypeSymbol(const std::type_info& typeInfo);

	// Note: Do *not* change content address for a value of primitive type
	// while connected to an input branch!
	static void inputValueFrom(WritableValue& value, TTree *tree, const std::string& branchName);

	// Note: Do *not* change content address for a value of primitive type
	// while connected to an output branch!
	static void outputValueTo(const Value& value, TTree *tree, const std::string& branchName, Int_t bufsize = 32000, Int_t defaultSplitlevel = 99, bool adaptSplitlevel = true);
};


} // namespace dbrx

#endif // DBRX_ROOTIO_H
