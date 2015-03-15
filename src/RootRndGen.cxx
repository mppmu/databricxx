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


#include "RootRndGen.h"

#include <iostream>


using namespace std;


namespace dbrx {


void RootRndGen::processInput() {
	m_func = std::unique_ptr<TF1>(new TF1("", pdf.get().c_str(), xMin, xMax));
	m_func->SetNpx(nPoints);
	index = -1;
}


bool RootRndGen::nextOutput() {
	if (index + 1 < nOut) {
		++index;
		output = m_func->GetRandom();
		return true;
	} else return false;
}


} // namespace dbrx
