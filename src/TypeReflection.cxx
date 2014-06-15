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
#include <cassert>

#include <TClass.h>
#include <TString.h>
#include <TDataType.h>
#include <TROOT.h>

#include "logging.h"

#include "format.h"

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
	return isPrimitive() ? getTypeInfo()->name() : getTClass()->GetName();
}


void* TypeReflection::newInstanceImpl(const TypeReflection& ptrType) {
	if (! ptrType.isAssignableFrom(*this))
		throw invalid_argument("Target pointer type \"%s\" cannot be assigned from object type \"%s\""_format(ptrType.name(), name()));

	if (isPrimitive()) {
		const type_info &ti = *getTypeInfo();
		if      (ti == typeid(bool)) return new bool();
		else if (ti == typeid(int8_t)) return new int8_t();
		else if (ti == typeid(uint8_t)) return new uint8_t();
		else if (ti == typeid(int16_t)) return new int16_t();
		else if (ti == typeid(uint16_t)) return new uint16_t();
		else if (ti == typeid(int32_t)) return new int32_t();
		else if (ti == typeid(uint32_t)) return new uint32_t();
		else if (ti == typeid(int64_t)) return new int64_t();
		else if (ti == typeid(uint64_t)) return new uint64_t();
		else if (ti == typeid(float)) return new float();
		else if (ti == typeid(double)) return new double();
		else throw invalid_argument("Dynamic instance creation of primitive type \"%s\" not supported"_format(ti.name()));
	} else {	
		void *obj = getTClass()->New(TClass::ENewType::kClassNew, true);
		if (obj == nullptr) {
			// May be necessary the first time the class is loaded, for some reason,
			// to make an inherited default constructor visible:
			log_debug("Failed to create object of class\"%s\", first-time load? Trying work-around.", name());
			gROOT->ProcessLine("delete new %s"_format(name()).c_str());
			obj = getTClass()->New();
		}
		if (obj == nullptr) throw runtime_error("Dynamic object creation of class \"%s\" failed"_format(name()));
		log_debug("Dynamically created object of class\"%s\"", name());
		return obj;
	}
}


bool TypeReflection::isAssignableFrom(const TypeReflection& other) const {
	if (isPrimitive()) return *getTypeInfo() == *other.getTypeInfo();
	else return isAssignableFrom(getTClass(), other.getTClass());
}


TypeReflection::TypeReflection(const std::type_info& typeInfo)
	: m_tClass(TClass::GetClass(typeInfo, true, true)), m_typeInfo(&typeInfo)
{
	// If class not found, check if primitive type, else throw exception
	if ( (m_tClass == nullptr) && (TDataType::GetType(typeInfo) == EDataType::kOther_t) )
		throw runtime_error("Could not resolve class for type_info \"%s\""_format(typeInfo.name()));
}


TypeReflection::TypeReflection(const char* typeName)
	: m_tClass(TClass::GetClass(typeName, true, true)), m_typeInfo(m_tClass->GetTypeInfo())
{
	// Currently does not support primitive types
	if (m_tClass == nullptr)
		throw runtime_error("Could not resolve class for type_info \"%s\""_format(typeName));
}

TypeReflection::TypeReflection(const std::string& typeName)
	: TypeReflection(typeName.c_str()) {}


TypeReflection::TypeReflection(const TClass* cl)
	: m_tClass(cl) {}


TypeReflection::TypeReflection(const TDataType& dt) {
	switch(EDataType(dt.GetType())) {
		case kChar_t: m_typeInfo = &typeid(Char_t); break;
		case kUChar_t: m_typeInfo = &typeid(UChar_t); break;
		case kShort_t: m_typeInfo = &typeid(Short_t); break;
		case kUShort_t: m_typeInfo = &typeid(UShort_t); break;
		case kInt_t: m_typeInfo = &typeid(Int_t); break;
		case kUInt_t: m_typeInfo = &typeid(UInt_t); break;
		case kLong_t: m_typeInfo = &typeid(Long_t); break;
		case kULong_t: m_typeInfo = &typeid(ULong_t); break;
		case kFloat_t: m_typeInfo = &typeid(Float_t); break;
		case kDouble_t: m_typeInfo = &typeid(Double_t); break;
		case kDouble32_t: m_typeInfo = &typeid(Double32_t); break;
		case kchar: m_typeInfo = &typeid(char); break;
		case kBool_t: m_typeInfo = &typeid(Bool_t); break;
		case kLong64_t: m_typeInfo = &typeid(Long64_t); break;
		case kULong64_t: m_typeInfo = &typeid(ULong64_t); break;
		case kOther_t: throw invalid_argument("TypeReflection does not support TDataType kOther_t"); break;
		case kNoType_t: throw invalid_argument("TypeReflection does not support TDataType kNoType_t"); break;
		case kFloat16_t: m_typeInfo = &typeid(Float16_t); break;
		case kCounter: throw invalid_argument("TypeReflection does not support TDataType kCounter"); break;
		case kCharStar: m_typeInfo = &typeid(char*); break;
		case kBits: throw invalid_argument("TypeReflection does not support TDataType kBits"); break;
		case kVoid_t: m_typeInfo = &typeid(void); break;
		case kDataTypeAliasUnsigned_t: throw invalid_argument("TypeReflection does not support TDataType kDataTypeAliasUnsigned_t"); break;
		default: assert(false);
	}
}


} // namespace dbrx
