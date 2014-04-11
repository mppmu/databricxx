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


#ifndef DBRX_PROPS_H
#define DBRX_PROPS_H

#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <iosfwd>

#include "Name.h"
#include "Value.h"


namespace dbrx {


class PropVal;

using Prop = std::pair<const Name, PropVal>;
using IndexedPropVal = std::pair<const int64_t, PropVal>;


class PropVal {
public:
	enum class Type: int32_t {
		NONE = 0,
		BOOL = 1,
		INTEGER = 2,
		REAL = 3,
		NAME = 4,
		STRING = 5,
		ARRAY = 6,
		INDEXED = 7,
		STRUC = 8,
	};


	using None = std::nullptr_t;
	using Bool = bool;
	using Integer = int64_t;
	using Real = double;
	using String = std::string;
	using Array = std::vector<PropVal>;
	using Indexed = std::map<Integer, PropVal>;
	using Struc = std::map<Name, PropVal, Name::CompareById>;


	template<typename T> static void swapMem(T &a, T &b) noexcept {
		struct Mem { uint8_t bytes[sizeof(T)]; };
		static_assert(sizeof(Mem) == sizeof(T), "swapMem internal memory size mismatch");
		Mem &memA = (Mem&) a;
		Mem &memB = (Mem&) b;
		std::swap(memA, memB);
	}


	static int32_t castToInt32(Integer value) {
		int32_t r = static_cast<int32_t>(value);
		if (r == value) return r;
		else throw std::bad_cast();
	}

	static bool castToBool(Integer value) {
		switch (value) {
			case 0: return false;
			case 1: return true;
			default: throw std::bad_cast();
		}
	}

protected:
	using ArrayPtr = std::unique_ptr<Array>;
	using IndexedPtr = std::unique_ptr<Indexed>;
	using StrucPtr = std::unique_ptr<Struc>;

	union Content {
		None e;
		Bool b;
		Integer i;
		Real r;
		Name n;
		String s;
		ArrayPtr a;
		IndexedPtr m;
		StrucPtr o;

		Content() : e() { }

		Content(None value) : e(std::move(value)) { }
		Content(Bool value) : b(std::move(value)) { }
		Content(Integer value) : i(std::move(value)) { }
		Content(Real value) : r(std::move(value)) { }
		Content(Name value) : n(std::move(value)) { }
		Content(String value) : s(std::move(value)) { }
		Content(Array value) : a( new Array(std::move(value)) ) { }
		Content(Indexed value) : m( new Indexed(std::move(value)) ) { }
		Content(Struc value) : o( new Struc(std::move(value)) ) { }

		Content(std::initializer_list<PropVal> init) : a(new Array(init)) { }

		Content(Type type) {
			switch (type) {
				case Type::NONE: new (&e) None(); break;
				case Type::BOOL: new (&b) Bool(); break;
				case Type::INTEGER: new (&i) Integer(); break;
				case Type::REAL: new (&r) Real(); break;
				case Type::NAME: new (&n) Name(); break;
				case Type::STRING: new (&s) String(); break;
				case Type::ARRAY: new (&a) ArrayPtr(new Array()); break;
				case Type::INDEXED: new (&m) IndexedPtr(new Indexed()); break;
				case Type::STRUC: new (&o) StrucPtr(new Struc()); break;
			}
		}

		Content(Type type, const Content &other) {
			switch (type) {
				case Type::NONE: new (&e) None(other.e); break;
				case Type::BOOL: new (&b) Bool(other.b); break;
				case Type::INTEGER: new (&i) Integer(other.i); break;
				case Type::REAL: new (&r) Real(other.r); break;
				case Type::NAME: new (&n) Name(other.n); break;
				case Type::STRING: new (&s) String(other.s); break;
				case Type::ARRAY: new (&a) ArrayPtr(new Array(*other.a)); break;
				case Type::INDEXED: new (&m) IndexedPtr(new Indexed(*other.m)); break;
				case Type::STRUC: new (&o) StrucPtr(new Struc(*other.o)); break;
			}
		}

		~Content() {}
	};


	Type m_type = Type::NONE;

	Content m_content;

	void destructorImpl();

	bool comparisonImpl(const PropVal &other) const;

public:
	Type type() const { return m_type; }

	bool isNone() const { return m_type == Type::NONE; }
	bool isBool() const { return m_type == Type::BOOL; }
	bool isInteger() const { return m_type == Type::INTEGER; }
	bool isReal() const { return m_type == Type::INTEGER || m_type == Type::REAL; }
	bool isName() const { return m_type == Type::NAME; }
	bool isString() const { return m_type == Type::STRING; }
	bool isArray() const { return m_type == Type::ARRAY; }
	bool isIndexed() const { return m_type == Type::INDEXED; }
	bool isStruc() const { return m_type == Type::STRUC; }

	bool asBool() const {
		switch (m_type) {
			case Type::BOOL: return m_content.b;
			case Type::INTEGER: return castToBool(m_content.i);
			default: throw std::bad_cast();
		}
	}

	int32_t asInt() const {
		switch (m_type) {
			case Type::INTEGER: return castToInt32(m_content.i);
			case Type::BOOL: return m_content.b ? 1 : 0;
			default: throw std::bad_cast();
		}
	}

	int64_t asLong64() const {
		switch (m_type) {
			case Type::INTEGER: return m_content.i;
			case Type::BOOL: return m_content.b ? 1 : 0;
			default: throw std::bad_cast();
		}
	}

	double asDouble() const {
		switch (m_type) {
			case Type::INTEGER:
				return m_content.i;
			case Type::REAL:
				return m_content.r;
			case Type::BOOL:
				return m_content.b ? 1 : 0;
			default: throw std::bad_cast();
		}
	}

	Name asName() const {
		switch (m_type) {
			case Type::NAME:
				return m_content.n;
			case Type::STRING:
				return Name(m_content.s);
			default: throw std::bad_cast();
		}
	}

	const std::string& asString() const {
		switch (m_type) {
			case Type::STRING:
				return m_content.s;
			case Type::NAME:
				return m_content.n.str();
			default: throw std::bad_cast();
		}
	}


	const Array& asArray() const {
		if (m_type == Type::ARRAY) return *m_content.a;
		else throw std::bad_cast();
	}

	Array& asArray() {
		if (m_type == Type::ARRAY) return *m_content.a;
		else throw std::bad_cast();
	}


	const Indexed& asIndexed() const {
		if (m_type == Type::INDEXED) return *m_content.m;
		else throw std::bad_cast();
	}

	Indexed& asIndexed() {
		if (m_type == Type::INDEXED) return *m_content.m;
		else throw std::bad_cast();
	}


	const Struc& asStruc() const {
		if (m_type == Type::STRUC) return *m_content.o;
		else throw std::bad_cast();
	}

	Struc& asStruc() {
		if (m_type == Type::STRUC) return *m_content.o;
		else throw std::bad_cast();
	}


	// std::vector is guaranteed to be contiguous in memory, vector iterators
	// can be converted to simple pointers:
	PropVal* begin() noexcept { return m_type == Type::ARRAY ? &*m_content.a->begin() : this; }
	const PropVal* begin() const noexcept  { return m_type == Type::ARRAY ? &*m_content.a->begin() : this; }
	const PropVal* cbegin() const noexcept { return m_type == Type::ARRAY ? &*m_content.a->cbegin() : this; }
	PropVal* end() noexcept { return m_type == Type::ARRAY ? &*m_content.a->end() : this; }
	const PropVal* end() const noexcept  { return m_type == Type::ARRAY ? &*m_content.a->end() : this; }
	const PropVal* cend() const noexcept { return m_type == Type::ARRAY ? &*m_content.a->cend() : this; }


	friend void swap(PropVal &a, PropVal&b) noexcept {
		std::swap(a.m_type, b.m_type);
		swapMem(a.m_content, b.m_content);
	}


	bool operator==(PropVal other) const {
		switch (m_type) {
			case Type::NONE: return other.m_type == Type::NONE;
			case Type::BOOL:
				switch (other.m_type) {
					case Type::BOOL: return m_content.b == other.m_content.b;
					case Type::INTEGER: return (m_content.b ? 1 : 0) == other.m_content.i;
					default: return false;
				}
			case Type::INTEGER:
				switch (other.m_type) {
					case Type::INTEGER: return m_content.i == other.m_content.i;
					case Type::BOOL: return m_content.i == (other.m_content.b ? 1 : 0);
					default: return false;
				}
			case Type::REAL: return (other.m_type == Type::REAL) &&
				(m_content.r == other.m_content.r);
			default: return comparisonImpl(other);
		}
	}

	PropVal& operator=(PropVal other) {
		using namespace std;
		swap(*this, other);
		return *this;
	}


	template <typename T> static std::ostream& print(std::ostream &os, const T &x) { return os << x; }
	static std::ostream& print(std::ostream &os, None x) { return os << "none"; }
	static std::ostream& print(std::ostream &os, Bool x) { return os << (x ? "true" : "false"); }
	static std::ostream& print(std::ostream &os, Real x);
	static std::ostream& print(std::ostream &os, const Name x);
	static std::ostream& print(std::ostream &os, const String &x);
	static std::ostream& print(std::ostream &os, const Array &x);
	static std::ostream& print(std::ostream &os, const Indexed &x);
	static std::ostream& print(std::ostream &os, const Struc &x);

	std::ostream& print(std::ostream &os) const;
	std::string toString() const;


	PropVal() {}

	PropVal(const PropVal &other)
		: m_type(other.m_type), m_content(other.m_type, other.m_content) {}

	PropVal(PropVal &&other) {
		using namespace std;
		swap(*this, other);
	}

	PropVal(Type type) : m_type(type), m_content(type) {}


	PropVal(Bool value) : m_type(Type::BOOL), m_content(value) {}

	PropVal(int32_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {}

	PropVal(Integer value) : m_type(Type::INTEGER), m_content(value) {}

	PropVal(double value) {
		int64_t intVal = static_cast<int64_t>(value);
		if (intVal == value) {
			m_type = Type::INTEGER;
			m_content.i = intVal;
		}
		else {
			m_type = Type::REAL;
			m_content.r = value;
		}
	}

	PropVal(Name value) : m_type(Type::NAME), m_content(value) {}

	PropVal(const String &value) : m_type(Type::STRING), m_content(value) {}

	PropVal(const char* value) : m_type(Type::STRING), m_content(std::string(value)) {}

	PropVal(const Array &value) : m_type(Type::ARRAY), m_content(value) {}
	PropVal(Array &&value) : m_type(Type::ARRAY), m_content(std::move(value)) {}
	PropVal(std::initializer_list<PropVal> init) : m_type(Type::ARRAY), m_content(init) { }

	PropVal(const Indexed &value) : m_type(Type::INDEXED), m_content(value) {}
	PropVal(Indexed &&value) : m_type(Type::INDEXED), m_content(std::move(value)) {}

	PropVal(const Struc &value) : m_type(Type::STRUC), m_content(value) {}
	PropVal(Struc &&value) : m_type(Type::STRUC), m_content(std::move(value)) {}


	static PropVal array() { return PropVal(Type::ARRAY); }

	template <typename InputIterator> static PropVal array(InputIterator first, InputIterator last) {
		PropVal result(Type::ARRAY);
		Array &a = result.asArray();
		a.reserve(last - first);
		while (first != last) a.push_back(*first++);
		return result;
	}

	static PropVal array(std::initializer_list<PropVal> init)
		{ return PropVal(Array(init)); }


	static PropVal indexed() { return PropVal(Type::INDEXED); }

	static PropVal indexed(std::initializer_list<IndexedPropVal> init)
		{ return PropVal(Indexed(init)); }


	static PropVal struc() { return PropVal(Type::STRUC); }

	static PropVal struc(std::initializer_list<Prop> init)
		{ return PropVal(Struc(init)); }


	~PropVal() {
		if (m_type > Type::NAME) destructorImpl();
	}

	friend class PropKey;
};


inline std::ostream& operator<<(std::ostream &os, const PropVal &value)
	{ return value.print(os); }



using Props = PropVal::Struc;


} // namespace dbrx

#endif // DBRX_PROPS_H
