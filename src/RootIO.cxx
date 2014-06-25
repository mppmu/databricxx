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

#include <TDataType.h>

#include "format.h"
#include "TypeReflection.h"


using namespace std;


namespace dbrx {


char RootIO::getTypeSymbol(const std::type_info& typeInfo) {
	if      (typeInfo == typeid(Bool_t))    return 'O';
	else if (typeInfo == typeid(Char_t))    return 'B';
	else if (typeInfo == typeid(UChar_t))   return 'b';
	else if (typeInfo == typeid(Short_t))   return 'S';
	else if (typeInfo == typeid(UShort_t))  return 's';
	else if (typeInfo == typeid(Int_t))     return 'I';
	else if (typeInfo == typeid(UInt_t))    return 'i';
	else if (typeInfo == typeid(Long64_t))  return 'L';
	else if (typeInfo == typeid(ULong64_t)) return 'l';
	else if (typeInfo == typeid(Double_t))  return 'D';
	else if (typeInfo == typeid(Float_t))   return 'F';
	else if (typeInfo == typeid(char*))     return 'C';
	else throw invalid_argument("No ROOT type symbol equivalent for type_info \"%s\""_format(typeInfo.name()));
}


void RootIO::inputValueFrom(WritableValue& value, TTree *tree, const std::string& branchName) {
	const char* bName = branchName.c_str();

	// Result of SetBranchAddress is not a reliable check for existence of the
	// branch (problem only with TChain?), so check with GetBranch first:
	if (tree->GetBranch(bName)) {
		tree->SetBranchStatus(bName, true);

		EDataType dataType = TDataType::GetType(value.typeInfo());
		Int_t result = -1;
		if (dataType == kNoType_t) { // Unknown type
			throw invalid_argument("Cannot set branch address for kNoType_t");
		} else if (dataType == EDataType::kOther_t) { // Object type
			const TClass *cl = TypeReflection(value.typeInfo()).getTClass();
			result = tree->SetBranchAddress(branchName.c_str(), value.untypedPPtr(), nullptr, const_cast<TClass*>(cl), dataType, true);
		} else { // Primitive type
			if (value.empty()) value.setToDefault();
			result = tree->SetBranchAddress(branchName.c_str(), value.untypedPtr(), nullptr, nullptr, dataType, false);
		}
		if (result < 0) throw runtime_error("Failed to set branch address for branch \"%s\""_format(branchName));

		tree->AddBranchToCache(bName);
	}
	else throw runtime_error("Branch \"%s\" not found"_format(branchName));
}


void RootIO::outputValueTo(Value& value, TTree *tree, const std::string& branchName, Int_t bufsize, Int_t splitlevel) {
	if (! value.valid()) throw invalid_argument("Cannot output invalid value object to branch");
	const char* bName = branchName.c_str();
	EDataType dataType = TDataType::GetType(value.typeInfo());

	TBranch* branch = nullptr;
	if (dataType == kNoType_t) { // Unknown type
		throw invalid_argument("Cannot create branch for kNoType_t");
	} else if (dataType == EDataType::kOther_t) { // Object type
		const TClass *cl = TypeReflection(value.typeInfo()).getTClass();
		branch = tree->Branch(bName, cl->GetName(), const_cast<void**>(value.untypedPPtr()), bufsize, splitlevel);
	} else { // Primitive type
		char typeSymbol = getTypeSymbol(value.typeInfo());
		if (value.empty()) throw invalid_argument("Cannot output empty value object of primitive type to branch");
		string formatString("%s/%c"_format(bName, typeSymbol));
		branch = tree->Branch(bName, const_cast<void*>(value.untypedPtr()), formatString.c_str(), bufsize);
	}
	if (branch == nullptr) throw runtime_error("Failed to create branch");
}


} // namespace dbrx
