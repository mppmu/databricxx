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


#include "RootIO.h"

#include <stdexcept>

#include "RootReflection.h"


using namespace std;


namespace dbrx {


void RootIO::inputValueFrom(PrimaryValue& value, TTree *tree, const TString& branchName) {
	const char* bName = branchName.Data();

	// Result of SetBranchAddress is not a reliable check for existence of the
	// branch (problem only with TChain?), so check with GetBranch first:
	if (tree->GetBranch(bName)) {
		tree->SetBranchStatus(bName, true);

		EDataType dataType = RootReflection::getDataType(value.typeInfo());
		Int_t result = -1;
		if (dataType == kNoType_t) { // Unknown type
			throw invalid_argument("Cannot set branch address for kNoType_t");
		} else if (dataType == EDataType::kOther_t) { // Object type
			TClass *cl = RootReflection::getClass(value.typeInfo());
			result = tree->SetBranchAddress(branchName, value.untypedPPtr(), nullptr, cl, dataType, true);
		} else { // Primitive type
			if (value.empty()) value.setToDefault();
			result = tree->SetBranchAddress(branchName, value.untypedPtr(), nullptr, nullptr, dataType, false);
		}
		if (result < 0) throw runtime_error("Failed to set branch address");

		tree->AddBranchToCache(bName);
	}
	else throw runtime_error("Branch not found");
}


void RootIO::outputValueTo(Value& value, TTree *tree, const TString& branchName, Int_t bufsize, Int_t splitlevel) {
	if (! value.valid()) throw invalid_argument("Cannot output invalid value object to branch");
	const char* bName = branchName.Data();
	EDataType dataType = RootReflection::getDataType(value.typeInfo());

	TBranch* branch = nullptr;
	if (dataType == kNoType_t) { // Unknown type
		throw invalid_argument("Cannot create branch for kNoType_t");
	} else if (dataType == EDataType::kOther_t) { // Object type
		TClass *cl = RootReflection::getClass(value.typeInfo());
		branch = tree->Branch(bName, cl->GetName(), const_cast<void**>(value.untypedPPtr()), bufsize, splitlevel);
	} else { // Primitive type
		char typeSymbol = RootReflection::getRootTypeSymbol(value.typeInfo());
		if (value.empty()) throw invalid_argument("Cannot output empty value object of primitive type to branch");
		TString formatString = TString::Format("%s/%c", bName, typeSymbol);
		branch = tree->Branch(bName, const_cast<void*>(value.untypedPtr()), formatString.Data(), bufsize);
	}
	if (branch == nullptr) throw runtime_error("Failed to create branch");
}


} // namespace dbrx
