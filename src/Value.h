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


#ifndef DBRX_VALUE_H
#define DBRX_VALUE_H

#include <memory>
#include <typeindex>

#include "Props.h"


namespace dbrx {


class Value {
public:
	virtual bool valid() const = 0;
	virtual bool empty() const = 0;

	virtual const std::type_info& typeInfo() const = 0;

	bool isPtrAssignableTo(const std::type_info& otherType) const;

	virtual const void* const * untypedPPtr() const = 0;

	const void* untypedPtr() const { return *untypedPPtr(); }

	template<typename T> const T* const * typedPPtr() const {
		if ( typeid(T) == typeInfo() || isPtrAssignableTo(typeid(T)) )
			return (const T* const *) untypedPPtr();
		else throw std::bad_cast();
	}

	template<typename T> const T* typedPtr() const { return *typedPPtr<T>(); }

	virtual PropVal toPropVal() const = 0;

	Value& operator=(const Value& v) = delete;
	Value& operator=(Value &&v) = delete;

	Value() {}
	Value(const Value &v) = delete;
	Value(Value &&v) = delete;

	virtual ~Value() {}
};



class WritableValue: public virtual Value {
public:
	virtual void* * untypedPPtr() = 0;

	void* untypedPtr() { return *untypedPPtr(); }

	template<typename T> T* * typedPPtr() {
		if (typeid(T) == typeInfo()) return (T* *) untypedPPtr();
		else throw std::bad_cast();		
	}

	template<typename T> T* typedPtr() { return *typedPPtr<T>(); }

	virtual void setToDefault() = 0;
	virtual void clear() = 0;

	virtual void fromPropVal(const PropVal &p) = 0;

	friend void swap(WritableValue &a, WritableValue &b)
		{ std::swap(*a.untypedPPtr(), *b.untypedPPtr()); }
};



class PrimaryValue: public virtual WritableValue {
public:
	bool valid() const final override { return true; }

	friend void swap(PrimaryValue &a, PrimaryValue &b)
		{ swap(static_cast<WritableValue &>(a), static_cast<WritableValue &>(b)); }
};



class ValueRef: public virtual WritableValue {
public:
	virtual void referTo(WritableValue &source) = 0;

	friend void swap(ValueRef &a, ValueRef &b)
		{ swap(static_cast<WritableValue &>(a), static_cast<WritableValue &>(b)); }
};



class ConstValueRef: public virtual Value {
public:
	virtual void referTo(const Value &source) = 0;
};



template <typename T> class TypedValue: public virtual Value {
protected:
	// SFINAE-based default implementation if convertToPropVal not available for T.
    struct PropValConvGeneral {};
    struct PropValConvSpecial : PropValConvGeneral {};
	template <typename U> static auto assignToPropVal(PropVal &p, const U& x, PropValConvSpecial) -> decltype(assign_from(p, x)) { assign_from(p, x); }
	static void assignToPropVal(PropVal &p, const T& x, PropValConvGeneral) { throw std::invalid_argument("No conversion from content type of this Value to PropVal available"); }


public:
	virtual operator const T& () const = 0;
	virtual const T* operator->() const = 0;
	virtual const T& get() const = 0;

	virtual const T* const * pptr() const = 0;

	virtual const T* ptr() const = 0;

	PropVal toPropVal() const final override
		{ PropVal p; assignToPropVal(p, get(), PropValConvSpecial()); return p; }

	friend bool operator==(const TypedValue &a, const T &b) { return a.get() == b; }
	friend bool operator==(const T &a, const TypedValue &b) { return a == b.get(); }
};



template <typename T> class TypedWritableValue: public virtual WritableValue, public virtual TypedValue<T> {
protected:
	// SFINAE-based default implementation if assignFromPropVal not available for T.
	using PropValConvGeneral = typename TypedValue<T>::PropValConvGeneral;
	using PropValConvSpecial = typename TypedValue<T>::PropValConvSpecial;
	template <typename U> static auto assignFromPropVal(U& x, const PropVal &p, PropValConvSpecial) -> decltype(assign_from(x, p)) { assign_from(x, p); }
	static void assignFromPropVal(T &x, const PropVal &p, PropValConvGeneral) { throw std::invalid_argument("No conversion from PropVal to content type of this Value available"); }

public:
	virtual operator T& () = 0;
	virtual T* operator->() = 0;
	virtual T& get() = 0;

	virtual T* * pptr() = 0;

	virtual T* ptr() = 0;

	void setToDefault() final override { operator=(std::unique_ptr<T>( new T() )); }
	void clear() final override { operator=(std::unique_ptr<T>((T*)nullptr)); }

	virtual T* release() final {
		T* result = nullptr;
		std::swap(result, *pptr());
		return result;
	}

	TypedWritableValue<T>& operator=(const T &v) {
		if (ptr() != nullptr) *ptr() = v;
		else *pptr() = new T(v);
		return *this;
	}

	TypedWritableValue<T>& operator=(T &&v) noexcept {
		if (ptr() != nullptr) *ptr() = std::move(v);
		else operator=(std::unique_ptr<T>( new T(std::move(v)) ));
		return *this;
	}

	TypedWritableValue<T>& operator=(std::unique_ptr<T> &&v) noexcept {
		using namespace std;
		unique_ptr<T> thisV(ptr()); *pptr() = nullptr;
		swap(thisV, v);
		*pptr() = thisV.release();
		return *this;
	}

	void fromPropVal(const PropVal &p) final override
		{ assignFromPropVal(get(), p, PropValConvSpecial()); }

	friend void swap(TypedWritableValue &a, TypedWritableValue &b)
		{ swap(static_cast<WritableValue &>(a), static_cast<WritableValue &>(b)); }
};



template <typename T> class TypedValueRef;



template <typename T> class TypedPrimaryValue final
	: public virtual PrimaryValue, public virtual TypedWritableValue<T>
{
protected:
	T* m_value = nullptr;

public:
	bool empty() const final override { return m_value == nullptr; }

	operator const T& () const final override { return *m_value; }
	operator T& () final override { return *m_value; }
	const T* operator->() const final override { return m_value; }
	T* operator->() final override { return m_value; }
	const T& get() const final override { return *m_value; }
	T& get() final override { return *m_value; }

	const std::type_info& typeInfo() const final override { return typeid(T); }

	const void* const * untypedPPtr() const final override { return (const void* const *) &m_value; }
	void* * untypedPPtr() final override { return (void* *)(&m_value); }

	const T* const * pptr() const final override { return &m_value; }
	T* * pptr() final override { return &m_value; }

	const T* ptr() const final override { return m_value; }
	T* ptr() final override { return m_value; }

	TypedPrimaryValue<T>& operator=(const T &v)
		{ TypedWritableValue<T>::operator=(v); return *this; }

	TypedPrimaryValue<T>& operator=(T &&v) noexcept
		{ TypedWritableValue<T>::operator=(std::move(v)); return *this; }

	TypedPrimaryValue<T>& operator=(std::unique_ptr<T> &&v) noexcept
		{ TypedWritableValue<T>::operator=(std::move(v)); return *this; }

	TypedPrimaryValue<T>& operator=(const TypedPrimaryValue<T>& v) = delete;
	TypedPrimaryValue<T>& operator=(TypedPrimaryValue<T> &&v) = delete;

	TypedPrimaryValue(): m_value(new T()) {}

	TypedPrimaryValue(std::nullptr_t) {};

	TypedPrimaryValue(const TypedPrimaryValue<T> &other) { *this = other.get(); }

	TypedPrimaryValue(TypedPrimaryValue<T> &&other) { std::swap(m_value, other.m_value); }

	TypedPrimaryValue(const T &v) { *this = v; }

	TypedPrimaryValue(T &&v) { *this = std::move(v); }

	TypedPrimaryValue(std::unique_ptr<T> &&v) = delete;

	~TypedPrimaryValue() override { if (m_value != nullptr) delete m_value; }

	friend void swap(TypedPrimaryValue &a, TypedPrimaryValue &b)
		{ swap(static_cast<PrimaryValue &>(a), static_cast<PrimaryValue &>(b)); }
};



template <typename T> class TypedValueRef final
	: public virtual ValueRef, public virtual TypedWritableValue<T>
{
protected:
	T* * m_value = nullptr;

public:
	bool valid() const final override { return (m_value != nullptr); }
	bool empty() const final override { return *m_value == nullptr; }

	operator const T& () const final override { return **m_value; }
	operator T& () final override { return **m_value; }
	const T* operator->() const final override { return *m_value; }
	T* operator->() final override { return *m_value; }
	const T& get() const final override { return **m_value; }
	T& get() final override { return **m_value; }

	void referTo(WritableValue &source) final override
		{ m_value = source.typedPPtr<T>(); }

	const std::type_info& typeInfo() const final override { return typeid(T); }

	const void* const * untypedPPtr() const final override { return (const void* const *) m_value; }
	void* * untypedPPtr() final override { return (void* *)(m_value); }

	const T* const * pptr() const final override { return m_value; }
	T* * pptr() final override { return m_value; }

	const T* ptr() const final override { return *m_value; }
	T* ptr() final override { return *m_value; }

	TypedValueRef<T>& operator=(const T &v)
		{ TypedWritableValue<T>::operator=(v); return *this; }

	TypedValueRef<T>& operator=(T &&v) noexcept
		{ TypedWritableValue<T>::operator=(std::move(v)); return *this; }

	TypedValueRef<T>& operator=(std::unique_ptr<T> &&v) noexcept
		{ TypedWritableValue<T>::operator=(std::move(v)); return *this; }

	TypedValueRef<T>& operator=(const TypedValueRef<T>& v) = delete;
	TypedValueRef<T>& operator=(TypedValueRef<T> &&v) = delete;

	TypedValueRef() = default;

	TypedValueRef(TypedValueRef<T> &other) : m_value(other.pptr()) {}

	TypedValueRef(TypedPrimaryValue<T> &&other) { std::swap(m_value, other.m_value); }

	TypedValueRef(TypedWritableValue<T> &v) : m_value(v.pptr()) {}

	TypedValueRef(WritableValue &source) { referTo(source); }

	friend void swap(TypedValueRef &a, TypedValueRef &b)
		{ swap(static_cast<ValueRef &>(a), static_cast<ValueRef &>(b)); }
};



template <typename T> class TypedConstValueRef final
	: public virtual ConstValueRef, public virtual TypedValue<T>
{
protected:
	const T* const * m_value = nullptr;

public:
	bool valid() const final override { return (m_value != nullptr); }
	bool empty() const final override { return *m_value == nullptr; }

	operator const T& () const final override { return **m_value; }
	const T* operator->() const final override { return *m_value; }
	const T& get() const final override { return **m_value; }

	void referTo(const Value &source) final override
		{ m_value = source.typedPPtr<T>(); }

	const std::type_info& typeInfo() const final override { return typeid(T); }

	const void* const * untypedPPtr() const final override { return (const void* const *) m_value; }

	const T* const * pptr() const final override { return m_value; }

	const T* ptr() const final override { return *m_value; }

	TypedConstValueRef<T>& operator=(const TypedConstValueRef<T>& v) = delete;
	TypedConstValueRef<T>& operator=(TypedConstValueRef<T> &&v) = delete;

	TypedConstValueRef() = default;

	TypedConstValueRef(const TypedConstValueRef<T> &other) : m_value(other.pptr()) {}

	TypedConstValueRef(TypedPrimaryValue<T> &&other) { std::swap(m_value, other.m_value); }

	TypedConstValueRef(const TypedValue<T> &other) : m_value(other.pptr()) {}

	TypedConstValueRef(const Value &source) { referTo(source); }	
};


} // namespace dbrx

#endif // DBRX_VALUE_H
