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


#include "TypeReflection.h"

#include <stdexcept>

#include <TClass.h>
#include <TString.h>
#include <TROOT.h>

#include "logging.h"

#include "../config.h"

using namespace std;


namespace dbrx {


bool TypeReflection::isAssignableFrom(const TClass* a, const TClass* b) {
	if (b->InheritsFrom(a)) return true;
	TList* bases = const_cast<TClass*>(b)->GetListOfBases();
	TIter next(bases, kIterForward);
	const TClass *base;
	while ( (base = dynamic_cast<const TClass*>(next())) ) {
		if (isAssignableFrom(a, base)) return true;
	}
	return false;
}


const char* TypeReflection::name() const {
	return getTClass()->GetName();	
}


void* TypeReflection::newInstanceImpl(const TypeReflection& ptrType) {
	if (! ptrType.isAssignableFrom(*this))
		throw invalid_argument(TString::Format("Target pointer type \"%s\" cannot be assigned from object type \"%s\"", ptrType.name(), name()).Data());

	void *obj = getTClass()->New(TClass::ENewType::kClassNew, true);
	if (obj == nullptr) {
		// May be necessary the first time the class is loaded, for some reason,
		// to make an inherited default constructor visible:
		log_debug("Failed to create object of class\"%s\", first-time load? Trying work-around.", name());
		gROOT->ProcessLine(TString::Format("delete new %s", name()));
		obj = getTClass()->New();
	}
	if (obj == nullptr) throw runtime_error(TString::Format("Dynamic object creation of class \"%s\" failed", name()).Data());
	log_debug("Dynamically created object of class\"%s\"", name());
	return obj;
}


bool TypeReflection::isAssignableFrom(const TypeReflection& other) const {
	return isAssignableFrom(getTClass(), other.getTClass());
}


TypeReflection::TypeReflection(const std::type_info& typeInfo) {
	m_tClass = TClass::GetClass(typeInfo, true, true);
	if (m_tClass == nullptr) throw runtime_error(TString::Format("Could not resolve class for type_info \"%s\"", typeInfo.name()).Data());
}


TypeReflection::TypeReflection(const char* typeName) {
	m_tClass = TClass::GetClass(typeName, true, true);
	if (m_tClass == nullptr) throw runtime_error(TString::Format("Could not resolve class \"%s\"", typeName).Data());
}


TypeReflection::TypeReflection(const std::string& typeName)
	: TypeReflection(typeName.c_str()) {}

TypeReflection::TypeReflection(const TString& typeName)
	: TypeReflection(typeName.Data()) {}

TypeReflection::TypeReflection(const TClass* cl)
	: m_tClass(cl) {}


} // namespace dbrx
