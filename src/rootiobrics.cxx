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


#include "rootiobrics.h"

#include <iostream>

#include <TChain.h>

#include "RootIO.h"


using namespace std;


namespace dbrx {


void TTreeIterBric::Entry::connectBranches(Bric* contextBric, TTree* inputTree) {
	for (auto &elem: m_outputs) {
		OutputTerminal *terminal = elem.second;
		dbrx_log_debug("Connecting TTree branch \"%s\" in \"%s\"", terminal->name(), absolutePath());
		RootIO::inputValueFrom(terminal->value(), inputTree, terminal->name());
	}
}


void TTreeIterBric::processInput() {
	dbrx_log_debug("Bric \"%s\" opens TTree \"%s\" from file \"%s\"", absolutePath(), treeName.get(), fileName.get());
	m_chain = std::unique_ptr<TChain>(new TChain(treeName->c_str()));
	m_chain->Add(fileName->c_str());

	m_chain->SetCacheSize(cacheSize);
	m_chain->SetBranchStatus("*", false);

	entry.connectBranches(this, m_chain.get());

	index = -1;
	size = m_chain->GetEntries();
}


bool TTreeIterBric::nextOutput() {
	if (index.get() < size.get()) {
		m_chain->GetEntry(index++);
		return true;
	} else return false;
}


} // namespace dbrx
