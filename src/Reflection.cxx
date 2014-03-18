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


#include "Reflection.h"

#include <stdexcept>

#include <TROOT.h>

#include "logging.h"

#include "../config.h"

using namespace std;


namespace dbrx {


void* Reflection::newObjectImpl(TClass* objType, TClass* ptrType) {
	if (! isAssignableFrom(ptrType, objType))
		throw invalid_argument(TString::Format("Target pointer type \"%s\" cannot be assigned from object type \"%s\"", ptrType->GetName(), objType->GetName()).Data());

	void *obj = objType->New(TClass::ENewType::kClassNew, true);
	if (obj == nullptr) {
		// May be necessary the first time the class is loaded, for some reason,
		// to make an inherited default constructor visible:
		log_debug("Failed to create object of class\"%s\", first-time load? Trying work-around.", objType->GetName());
		gROOT->ProcessLine(TString::Format("delete new %s", objType->GetName()));
		obj = objType->New();
	}
	if (obj == nullptr) throw runtime_error(TString::Format("Dynamic object creation of class \"%s\" failed", objType->GetName()).Data());
	log_debug("Dynamically created object of class\"%s\"", objType->GetName());
	return obj;
}


void* Reflection::newObjectImpl(const TString& objType, const TString& ptrType) {
	TClass *objTypeCl = getClass(objType);
	TClass *ptrTypeCl = getClass(ptrType);
	return newObjectImpl(objTypeCl, ptrTypeCl);
}


TClass* Reflection::getClass(const TString& className) {
	TClass *cl = TClass::GetClass(className, true, true);
	if (cl == nullptr) throw runtime_error(TString::Format("Could not resolve class \"%s\"", className.Data()).Data());
	return cl;
}


TClass* Reflection::getClass(const std::type_info& typeInfo) {
	TClass *cl = TClass::GetClass(typeInfo, true, true);
	if (cl == nullptr) throw runtime_error(TString::Format("Could not resolve class for type_info \"%s\"", typeInfo.name()).Data());
	return cl;
}


EDataType Reflection::getDataType(const std::type_info& typeInfo) {
	return TDataType::GetType(typeInfo);
}


char Reflection::getRootTypeSymbol(const std::type_info& typeInfo) {
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
	else throw invalid_argument(TString::Format("No ROOT type symbol equivalent for type_info \"%s\"", typeInfo.name()).Data());
}


bool Reflection::isAssignableFrom(TClass* a, TClass* b) {
	if (b->InheritsFrom(a)) return true;
	TList* bases = b->GetListOfBases();
	TIter next(bases, kIterForward);
	TClass *base;
	while ( (base = dynamic_cast<TClass*>(next())) ) {
		if (isAssignableFrom(a, base)) return true;
	}
	return false;
}


bool Reflection::isAssignableFrom(const TString& base, const TString& cl) {
	TClass *baseCl = getClass(base);
	TClass *classCl = getClass(cl);
	return isAssignableFrom(baseCl, classCl);
}


} // namespace dbrx
