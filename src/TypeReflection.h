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


#ifndef DBRX_TYPEREFLECTION_H
#define DBRX_TYPEREFLECTION_H

#include <memory>
#include <typeindex>


class TString;
class TClass;
class TDataType;


namespace dbrx {


/// @brief Type reflection class.
///
/// Implementation uses ROOT C++ reflection mechanisms.

class TypeReflection {
protected:
	const TClass* m_tClass{nullptr};
	const std::type_info* m_typeInfo{nullptr};

	static bool is(const TClass* a, const TClass* b);

	static bool isAssignableFrom(const TClass* a, const TClass* b);

	void* newInstanceImpl(const TypeReflection& ptrType);

public:
	template<typename T> std::unique_ptr<T> newInstance()
		{ return std::move(std::unique_ptr<T>( static_cast<T*>(newInstanceImpl( TypeReflection(typeid(T)) )) )); }

	const char* name() const;

	bool isClass() const { return m_tClass != nullptr; }
	bool isPrimitive() const { return !isClass(); }
	bool isAssignableFrom(const TypeReflection& other) const;

	const TClass* getTClass() const { return m_tClass; }

	// type_info is always available for primitive types, but may not be
	// available for class types.
	const std::type_info* getTypeInfo() const { return m_typeInfo; }

	TypeReflection() {}

	// Supports both class types and primitive types
	TypeReflection(const std::type_info& typeInfo);

	// Currently does not support primitive types
	TypeReflection(const char* typeName);

	// Currently does not support primitive types
	TypeReflection(const std::string& typeName);

	// Currently does not support primitive types
	TypeReflection(const TString& typeName);

	TypeReflection(const TClass* cl);

	// Currently does not support class types
	TypeReflection(const TDataType& dt);

	virtual ~TypeReflection() {}
};


} // namespace dbrx


#endif // DBRX_TYPEREFLECTION_H
