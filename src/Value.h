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


namespace dbrx {


class Value {
public:
	virtual bool valid() = 0;
	virtual bool empty() = 0;

	virtual const std::type_info& typeInfo() const = 0;

	virtual const void* const * untypedPPtr() const = 0;

	const void* untypedPtr() const { return *untypedPPtr(); }

	template<typename T> const T* const * typedPPtr() const {
		if (typeid(T) != typeInfo()) throw std::runtime_error("Type mismatch");
		return (const T* const *) untypedPPtr();
	}

	template<typename T> const T* typedPtr() const { return *typedPPtr<T>(); }

	virtual ~Value() {}
};



class WritableValue: public Value {
public:
	virtual void* * untypedPPtr() = 0;

	void* untypedPtr() { return *untypedPPtr(); }

	template<typename T> T* * typedPPtr() {
		if (typeid(T) != typeInfo()) throw std::runtime_error("Type mismatch");
		return (T* *) untypedPPtr();
	}

	template<typename T> T* typedPtr() { return *typedPPtr<T>(); }

	virtual void setToDefault() = 0;
	virtual void clear() = 0;
};



class UniqueValue: public WritableValue {
public:
	bool valid() { return true; }
};



class ValueRef: public WritableValue {
public:
	virtual void referTo(WritableValue &source) = 0;
};



class ConstValueRef: public Value {
public:
	virtual void referTo(const Value &source) = 0;
};



template <typename T> class TypedValueRef;



template <typename T> class TypedUniqueValue: public UniqueValue {
protected:
	T* m_value = nullptr;

public:
	bool empty() { return m_value == nullptr; }

	operator const T& () const { return *m_value; }
	operator T& () { return *m_value; }
	const T* operator->() const { return m_value; }
	T* operator->() { return m_value; }
	const T& get() const { return *m_value; }
	T& get() { return *m_value; }

	const std::type_info& typeInfo() const { return typeid(T); }

	const void* const * untypedPPtr() const { return (const void* const *) &m_value; }
	void* * untypedPPtr() { return (void* *)(&m_value); }
	const T* const * typedPPtr() const { return (const T* const *) &m_value; }
	T* * typedPPtr() { return (T* *) &m_value; }

	void setToDefault() { operator=(std::unique_ptr<T>( new T() )); }
	void clear() { operator=(std::unique_ptr<T>((T*)nullptr)); }

	const T* const * pptr() const { return &m_value; }
	T* * pptr() { return &m_value; }

	const T* ptr() const { return m_value; }
	T* ptr() { return m_value; }

	TypedUniqueValue<T>& operator=(const T &v) {
		if (m_value != nullptr) *m_value = v;
		else m_value = new T(v);
		return *this;
	}

	TypedUniqueValue<T>& operator=(T &&v) noexcept {
		if (m_value != nullptr) *m_value = std::move(v);
		else operator=(std::unique_ptr<T>( new T(std::move(v)) ));
		return *this;
	}

	TypedUniqueValue<T>& operator=(std::unique_ptr<T> &&v) noexcept {
		std::unique_ptr<T> thisV(m_value); m_value = nullptr;
		swap(thisV, v);
		m_value = thisV.release();
		return *this;
	}

	TypedUniqueValue<T>& operator=(const TypedUniqueValue<T>& v) = delete;

	TypedUniqueValue() = default;

	TypedUniqueValue(const TypedUniqueValue &other) = delete;

	TypedUniqueValue(const T &v) { *this = v; }

	TypedUniqueValue(T &&v) { *this = std::move(v); }

	TypedUniqueValue(std::unique_ptr<T> &&v) { *this = std::move(v); }

	~TypedUniqueValue() { if (m_value != nullptr) delete m_value; }

	friend class TypedValueRef<T>;
};



template <typename T> class TypedValueRef: public ValueRef {
protected:
	T* * m_value = nullptr;

public:
	bool valid() { return (m_value != nullptr); }
	bool empty() { return *m_value == nullptr; }

	operator const T& () const { return **m_value; }
	operator T& () { return **m_value; }
	const T* operator->() const { return *m_value; }
	T* operator->() { return *m_value; }
	const T& get() const { return **m_value; }
	T& get() { return **m_value; }

	void referTo(WritableValue &source)
		{ m_value = source.typedPPtr<T>(); }

	const std::type_info& typeInfo() const { return typeid(T); }

	const void* const * untypedPPtr() const { return (const void* const *) m_value; }
	void* * untypedPPtr() { return (void* *)(m_value); }
	const T* const * typedPPtr() const { return m_value; }
	T* * typedPPtr() { return (T* *) m_value; }

	void setToDefault() { operator=(std::unique_ptr<T>( new T() )); }
	void clear() { operator=(std::unique_ptr<T>((T*)nullptr)); }

	const T* const * pptr() const { return m_value; }
	T* * pptr() { return m_value; }

	const T* ptr() const { return *m_value; }
	T* ptr() { return *m_value; }

	TypedValueRef<T>& operator=(const T &v) {
		if (*m_value != nullptr) **m_value = v;
		else *m_value = new T(v);
		return *this;
	}

	TypedValueRef<T>& operator=(T &&v) noexcept {
		if (*m_value != nullptr) **m_value = std::move(v);
		else operator=(std::unique_ptr<T>( new T(std::move(v)) ));
		return *this;
	}

	TypedValueRef<T>& operator=(std::unique_ptr<T> &&v) noexcept {
		std::unique_ptr<T> thisV(*m_value); *m_value = nullptr;
		swap(thisV, v);
		*m_value = thisV.release();
		return *this;
	}

	TypedValueRef() = default;

	TypedValueRef(const TypedValueRef<T> &other) = default;

	TypedValueRef(TypedUniqueValue<T> &v) : m_value(v.pptr()) {}

	TypedValueRef(WritableValue &source) { referTo(source); }
};



template <typename T> class TypedConstValueRef: public ConstValueRef {
protected:
	const T* const * m_value = nullptr;

public:
	bool valid() { return (m_value != nullptr); }
	bool empty() { return *m_value == nullptr; }

	operator const T& () const { return **m_value; }
	const T* operator->() const { return *m_value; }
	const T& get() const { return **m_value; }

	void referTo(const Value &source)
		{ m_value = source.typedPPtr<T>(); }

	const std::type_info& typeInfo() const { return typeid(T); }

	const void* const * untypedPPtr() const { return (const void* const *) m_value; }
	const T* const * typedPPtr() const { return m_value; }

	const T* const * pptr() const { return m_value; }

	const T* ptr() const { return *m_value; }

	TypedConstValueRef() = default;

	TypedConstValueRef(const TypedConstValueRef<T> &other) = default;

	TypedConstValueRef(const TypedUniqueValue<T> &v) : m_value(v.pptr()) {}

	TypedConstValueRef(const TypedValueRef<T> &other) : m_value(other.pptr()) {}

	TypedConstValueRef(const Value &source) { referTo(source); }	
};


} // namespace dbrx

#endif // DBRX_VALUE_H
