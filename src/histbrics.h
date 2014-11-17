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


#ifndef DBRX_HISTBRICS_H
#define DBRX_HISTBRICS_H

#include <TH1F.h>

#include "Bric.h"


namespace dbrx {


template<typename T> class Hist1Builder: public ReducerBric {
public:
	using Hist = TH1F;

	Input<T> input{this};
	Output<Hist> output{this};

	Param<std::string> histName{this, "histName", "Histogram Name", "hist"};
	Param<std::string> histTitle{this, "histTitle", "Histogram Title", ""};
	Param<Int_t> nBins{this, "nBins", "Number of bins", 8};
	Param<Double_t> xlow{this, "xlow", "Low edge of first bin", 0};
	Param<Double_t> xup{this, "xup", "Upper edge of last bin (not included in last bin)", 10};

	void newReduction() override {
		output.value() = std::unique_ptr<Hist>(
			new Hist(histName.get().c_str(), histTitle.get().c_str(), nBins, xlow, xup)
		);
	}

	void processInput() override {
		output->Fill(input);
	}

	using ReducerBric::ReducerBric;
};


} // namespace dbrx

/* Add to ...LinkDef.h:

// histbrics.h
#pragma link C++ class dbrx::histbrics+;

*/

#endif // DBRX_HISTBRICS_H
