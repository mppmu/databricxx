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


#ifndef DBRX_HASVALUE_H
#define DBRX_HASVALUE_H

#include <Value.h>


namespace dbrx {


class HasValue {
public:
	virtual const Value& value() const = 0;
	virtual Value& value() = 0;

	HasValue& operator=(const HasValue& v) = delete;
	HasValue& operator=(HasValue &&v) = delete;

	HasValue() {}
	HasValue(const Value &v) {}
	HasValue(Value &&v) {}

	virtual ~HasValue() {}
};



class HasWritableValue: public virtual HasValue {
public:
	virtual const WritableValue& value() const = 0;
	virtual WritableValue& value() = 0;
};



class HasPrimaryValue: public virtual HasWritableValue {
public:
	virtual const PrimaryValue& value() const = 0;
	virtual PrimaryValue& value() = 0;
};



class HasValueRef: public virtual HasWritableValue {
public:
	virtual const ValueRef& value() const = 0;
	virtual ValueRef& value() = 0;
};



class HasConstValueRef: public virtual HasValue {
public:
	virtual const ConstValueRef& value() const = 0;
	virtual ConstValueRef& value() = 0;
};



template <typename T> class HasTypedValue: public virtual HasValue {
public:
	virtual const TypedValue<T>& value() const = 0;
	virtual TypedValue<T>& value() = 0;

	operator const T& () const { return value(); }
	const T* operator->() const { return value().operator->(); }
	const T& get() const { return value().get(); }
};


template <typename T> class HasTypedWritableValue
	: public virtual HasWritableValue, public virtual HasTypedValue<T>
{
public:
	virtual const TypedWritableValue<T>& value() const = 0;
	virtual TypedWritableValue<T>& value() = 0;

	operator T& () { return value(); }
	T* operator->() { return value().operator->(); }
	T& get() { return value().get(); }

	HasTypedWritableValue<T>& operator=(const T &v)
		{ value() = v; return *this; }

	HasTypedWritableValue<T>& operator=(T &&v) noexcept
		{ value() = std::move(v); return *this; }

	HasTypedWritableValue<T>& operator=(std::unique_ptr<T> &&v) noexcept
		{ value() = std::move(v); return *this; }
};



template <typename T> class HasTypedPrimaryValue
	: public virtual HasPrimaryValue, public virtual HasTypedWritableValue<T>
{
public:
	virtual const TypedPrimaryValue<T>& value() const = 0;
	virtual TypedPrimaryValue<T>& value() = 0;

	HasTypedPrimaryValue<T>& operator=(const T &v)
		{ HasTypedWritableValue<T>::operator=(v); return *this; }

	HasTypedPrimaryValue<T>& operator=(T &&v) noexcept
		{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }

	HasTypedPrimaryValue<T>& operator=(std::unique_ptr<T> &&v) noexcept
		{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }
};


template <typename T> class HasTypedPrimaryValueImpl: public virtual HasTypedPrimaryValue<T> {
protected:
	TypedPrimaryValue<T> m_value;
public:
	const TypedPrimaryValue<T>& value() const { return m_value; }
	TypedPrimaryValue<T>& value() { return m_value; }
};



template <typename T> class HasTypedValueRef
	: public virtual HasValueRef, public virtual HasTypedWritableValue<T>
{
public:
	virtual const TypedValueRef<T>& value() const = 0;
	virtual TypedValueRef<T>& value() = 0;

	HasTypedValueRef<T>& operator=(const T &v)
		{ HasTypedWritableValue<T>::operator=(v); return *this; }

	HasTypedValueRef<T>& operator=(T &&v) noexcept
		{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }

	HasTypedValueRef<T>& operator=(std::unique_ptr<T> &&v) noexcept
		{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }
};


template <typename T> class HasTypedValueRefImpl: public virtual HasTypedValueRef<T> {
protected:
	TypedValueRef<T> m_value;
public:
	const TypedValueRef<T>& value() const { return m_value; }
	TypedValueRef<T>& value() { return m_value; }
};



template <typename T> class HasTypedConstValueRef
	: public virtual HasConstValueRef, public virtual HasTypedValue<T>
{
public:
	virtual const TypedConstValueRef<T>& value() const = 0;
	virtual TypedConstValueRef<T>& value() = 0;
};


template <typename T> class HasTypedConstValueRefImpl: public virtual HasTypedConstValueRef<T> {
protected:
	TypedConstValueRef<T> m_value;
public:
	const TypedConstValueRef<T>& value() const { return m_value; }
	TypedConstValueRef<T>& value() { return m_value; }
};


} // namespace dbrx

#endif // DBRX_HASVALUE_H
