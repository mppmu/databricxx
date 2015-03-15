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


#ifndef DBRX_ROOTRNDGEN_H
#define DBRX_ROOTRNDGEN_H

#include <TF1.h>

#include "Bric.h"


namespace dbrx {


class RootRndGen final: public dbrx::MapperBric {
protected:
	std::unique_ptr<TF1> m_func;

public:
	Input<std::string> pdf{this, "pdf"};

	Output<Double_t> output{this};
	Output<Int_t> index{this, "index"};

	Param<Double_t> xMin{this, "xMin", "Lower limit of output range", 0};
	Param<Double_t> xMax{this, "xMax", "Upper limit of output range", 1};
	Param<Double_t> nPoints{this, "nPoints", "Number of points for function evalutation", 100};

	Param<Int_t> nOut{this, "nOut", "Number of random values to produce for each input", 100};


	void processInput() override;

	bool nextOutput() override;

	using dbrx::MapperBric::MapperBric;
};


} // namespace dbrx

#endif // DBRX_ROOTRNDGEN_H
