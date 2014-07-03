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


#ifndef DBRX_ROOTIOBRICS_H
#define DBRX_ROOTIOBRICS_H

#include "Bric.h"

class TTree;
class TChain;
#include<TChain.h>

namespace dbrx {


class TTreeIterBric final: public MapperBric {
protected:
	std::unique_ptr<TChain> m_chain;

public:
	class Entry final: public DynOutputGroup {
	public:
		void connectBranches(Bric* contextBric, TTree* inputTree);
		using DynOutputGroup::DynOutputGroup;
	};

	Input<std::string> fileName{this, "fileName"};

	Param<std::string> treeName{this, "treeName"};
	Param<int64_t> cacheSize{this, "cacheSize", "Input read-ahead cache size (-1 for default)", -1};

	Entry entry{this, "entry"};

	Output<ssize_t> size{this, "size", "Number of entries"};
	Output<ssize_t> index{this, "index", "Number of entries"};

	void processInput() override;

	bool nextOutput() override;

	using MapperBric::MapperBric;
};


} // namespace dbrx

#endif // DBRX_ROOTIOBRICS_H
