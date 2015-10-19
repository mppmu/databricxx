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
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <iosfwd>
#include <limits>
#include <cassert>

#include "Name.h"
#include "Printable.h"


namespace dbrx {


class PropVal;


class PropKey final: public virtual Printable {
public:
	enum class Type: int32_t {
		INTEGER = 2,
		NAME = 4
	};

	using Integer = int64_t;
	using String = std::string;

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

protected:
	union Content {
		Integer i;
		Name n;

		Content() : n() { }
		Content(Integer value) : i(std::move(value)) { }
		Content(Name value) : n(std::move(value)) { }

		Content(Type type, const Content &other) {
			switch (type) {
				case Type::INTEGER: new (&i) Integer(other.i); break;
				case Type::NAME: new (&n) Name(other.n); break;
				default: assert(false);
			}
		}
	};


	Type m_type = Type::INTEGER;

	Content m_content{0};

public:
	Type type() const { return m_type; }

	size_t hash() const {
		switch (m_type) {
			case Type::INTEGER: return std::hash<Integer>()(m_content.i);
			case Type::NAME: return std::hash<Name>()(m_content.n);
		}
	}

	bool isInteger() const { return m_type == Type::INTEGER; }
	bool isName() const { return m_type == Type::NAME; }

	Integer asInteger() const {
		switch (m_type) {
			case Type::INTEGER: return m_content.i;
			default: throw std::bad_cast();
		}
	}

	int32_t asInt32() const { return castToInt32(asInteger()); }
	int64_t asLong64() const { return asInteger(); }

	Name asName() const {
		switch (m_type) {
			case Type::NAME:
				return m_content.n;
			default: throw std::bad_cast();
		}
	}

	struct CompareById {
		bool operator() (const PropKey &a, const PropKey &b) const {
			switch (a.m_type) {
				case Type::INTEGER:  return b.m_type == Type::INTEGER ?
					a.m_content.i < b.m_content.i : true;
				case Type::NAME: return b.m_type == Type::NAME ?
					a.m_content.n.id() < b.m_content.n.id() : false;
				default: throw std::bad_cast();
			}
		}
	};


	friend bool operator<(const PropKey &a, const PropKey &b) {
		switch (a.m_type) {
			case Type::INTEGER:  return b.m_type == Type::INTEGER ?
				a.m_content.i < b.m_content.i : true;
			case Type::NAME: return b.m_type == Type::NAME ?
				a.m_content.n < b.m_content.n : false;
			default: throw std::bad_cast();
		}
	}


	friend void swap(PropKey &a, PropKey&b) noexcept {
		std::swap(a.m_type, b.m_type);
		swapMem(a.m_content, b.m_content);
	}


	friend bool operator==(PropKey a, PropKey b) {
		switch (a.m_type) {
			case Type::INTEGER:
				switch (b.m_type) {
					case Type::INTEGER: return a.m_content.i == b.m_content.i;
					default: return false;
				}
			case Type::NAME:
				switch (b.m_type) {
					case Type::NAME: return a.m_content.n == b.m_content.n;
					default: return false;
				}
			default: assert(false);
		}
	}

	friend bool operator!=(PropKey a, PropKey b) { return ! operator==(a, b); }

	PropKey& operator=(PropKey other) {
		using namespace std;
		swap(*this, other);
		return *this;
	}


	static void toJSON(std::ostream &out, const String &x);

	void toJSON(std::ostream &out) const;
	std::string toJSON() const;

	std::ostream& print(std::ostream &os) const override;


	PropKey() : m_type(Type::NAME), m_content(Name()) {}

	PropKey(const PropKey &other)
		: m_type(other.m_type), m_content(other.m_type, other.m_content) {}

	PropKey(PropKey &&other) {
		using namespace std;
		swap(*this, other);
	}

	PropKey(int32_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {}

	PropKey(uint32_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {}

	PropKey(int64_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {}

	PropKey(uint64_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {
		if (uint64_t(m_content.i) != value) throw std::bad_cast();
	}

	PropKey(Name value) : m_type(Type::NAME), m_content(value) {}

	PropKey(const std::string &value);

	PropKey(const char* value) : PropKey(std::string(value)) {}
};


inline void assign_from(PropKey &to, const PropKey &from) { to = from; }

inline void assign_from(int8_t &to, const PropKey &from) { to = from.asInt32(); }
inline void assign_from(uint8_t &to, const PropKey &from) { to = from.asInt32(); }
inline void assign_from(int16_t &to, const PropKey &from) { to = from.asInt32(); }
inline void assign_from(uint16_t &to, const PropKey &from) { to = from.asInt32(); }
inline void assign_from(int32_t &to, const PropKey &from) { to = from.asInt32(); }
inline void assign_from(uint32_t &to, const PropKey &from) { to = from.asInt32(); }
inline void assign_from(int64_t &to, const PropKey &from) { to = from.asLong64(); }
inline void assign_from(uint64_t &to, const PropKey &from) { to = from.asLong64(); }
inline void assign_from(Name &to, const PropKey &from) { to = from.asName(); }
inline void assign_from(std::string &to, const PropKey &from) { to = from.toString(); }



using Prop = std::pair<const PropKey, PropVal>;


class PropVal final: public virtual Printable {
public:
	enum class Type: int32_t {
		NONE = 0,
		BOOL = 1,
		INTEGER = 2,
		REAL = 3,
		NAME = 4,
		STRING = 5,
		BYTES = 6,
		ARRAY = 7,
		PROPS = 8,
	};


	using None = std::nullptr_t;
	using Bool = bool;
	using Integer = PropKey::Integer;
	using Real = double;
	using String = PropKey::String;
	using Bytes = std::vector<uint8_t>;
	using Array = std::vector<PropVal>;
	using Props = std::map<PropKey, PropVal, PropKey::CompareById>;


	template<typename T> static void swapMem(T &a, T &b) noexcept { PropKey::swapMem(a, b); }


	static int32_t castToInt32(Integer value) { return PropKey::castToInt32(value); }

	static bool castToBool(Integer value) {
		switch (value) {
			case 0: return false;
			case 1: return true;
			default: throw std::bad_cast();
		}
	}

protected:
	using BytesPtr = std::unique_ptr<Bytes>;
	using ArrayPtr = std::unique_ptr<Array>;
	using PropsPtr = std::unique_ptr<Props>;

	union Content {
		None e;
		Bool b;
		Integer i;
		Real r;
		Name n;
		String s;
		BytesPtr y;
		ArrayPtr a;
		PropsPtr o;

		Content() : e() { }

		Content(None value) : e(std::move(value)) { }
		Content(Bool value) : b(std::move(value)) { }
		Content(Integer value) : i(std::move(value)) { }
		Content(Real value) : r(std::move(value)) { }
		Content(Name value) : n(std::move(value)) { }
		Content(String value) : s(std::move(value)) { }
		Content(Bytes value) : y( new Bytes(std::move(value)) ) { }
		Content(Array value) : a( new Array(std::move(value)) ) { }
		Content(Props value) : o( new Props(std::move(value)) ) { }

		Content(std::initializer_list<PropVal> init) : a(new Array(init)) { }

		Content(Type type) {
			switch (type) {
				case Type::NONE: new (&e) None(); break;
				case Type::BOOL: new (&b) Bool(); break;
				case Type::INTEGER: new (&i) Integer(); break;
				case Type::REAL: new (&r) Real(); break;
				case Type::NAME: new (&n) Name(); break;
				case Type::STRING: new (&s) String(); break;
				case Type::BYTES: new (&y) BytesPtr(new Bytes()); break;
				case Type::ARRAY: new (&a) ArrayPtr(new Array()); break;
				case Type::PROPS: new (&o) PropsPtr(new Props()); break;
				default: assert(false);
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
				case Type::BYTES: new (&y) BytesPtr(new Bytes(*other.y)); break;
				case Type::ARRAY: new (&a) ArrayPtr(new Array(*other.a)); break;
				case Type::PROPS: new (&o) PropsPtr(new Props(*other.o)); break;
				default: assert(false);
			}
		}

		~Content() {}
	};


	Type m_type = Type::NONE;

	Content m_content;

	void destructorImpl() {
		switch (m_type) {
			case Type::STRING: m_content.s.~String(); break;
			case Type::BYTES: m_content.y.~BytesPtr(); break;
			case Type::ARRAY: m_content.a.~ArrayPtr(); break;
			case Type::PROPS: m_content.o.~PropsPtr(); break;
			default: assert(false);
		}
	}

	bool comparisonImpl(const PropVal &other) const;

	static bool substVarsImplContainsVar(const std::string &input);
	static PropVal substVarsImplSubstVars(const std::string &input, const Props &varValues, Props* envVarValues, bool ignoreMissing);
	void substVarsImpl(const Props &varValues, Props* envVarValues, bool ignoreMissing);

	static const PropVal s_noneValue;

public:
	Type type() const { return m_type; }

	bool isNone() const { return m_type == Type::NONE; }
	bool isBool() const { return m_type == Type::BOOL; }
	bool isInteger() const { return m_type == Type::INTEGER; }
	bool isReal() const { return m_type == Type::INTEGER || m_type == Type::REAL; }
	bool isName() const { return m_type == Type::NAME; }
	bool isString() const { return m_type == Type::NAME || m_type == Type::STRING; }
	bool isBytes() const { return m_type == Type::BYTES; }
	bool isArray() const { return m_type == Type::ARRAY; }
	bool isProps() const { return m_type == Type::PROPS; }

	bool asBool() const {
		switch (m_type) {
			case Type::BOOL: return m_content.b;
			case Type::INTEGER: return castToBool(m_content.i);
			default: throw std::bad_cast();
		}
	}

	Integer asInteger() const {
		switch (m_type) {
			case Type::INTEGER: return m_content.i;
			case Type::BOOL: return m_content.b ? 1 : 0;
			default: throw std::bad_cast();
		}
	}

	int32_t asInt32() const { return castToInt32(asInteger()); }
	int64_t asLong64() const { return asInteger(); }


	double asDouble() const {
		switch (m_type) {
			case Type::NONE:
				return std::numeric_limits<double>::quiet_NaN();
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
		if (m_type == Type::NAME) return m_content.n;
		else throw std::bad_cast();
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


	const Bytes& asBytes() const {
		if (m_type == Type::BYTES) return *m_content.y;
		else throw std::bad_cast();
	}


	const Array& asArray() const {
		if (m_type == Type::ARRAY) return *m_content.a;
		else throw std::bad_cast();
	}

	Array& asArray() {
		if (m_type == Type::ARRAY) return *m_content.a;
		else throw std::bad_cast();
	}


	const Props& asProps() const {
		if (m_type == Type::PROPS) return *m_content.o;
		else throw std::bad_cast();
	}

	Props& asProps() {
		if (m_type == Type::PROPS) return *m_content.o;
		else throw std::bad_cast();
	}


	size_t size() const {
		switch (m_type) {
			case Type::ARRAY: return m_content.a->size();
			case Type::NONE: return 0;
			default: return 1;
		}
	}

	using iterator = PropVal*;
	using const_iterator = const PropVal*;

	iterator begin() noexcept { return m_type == Type::ARRAY ? &*m_content.a->begin() : this; }
	const_iterator begin() const noexcept { return m_type == Type::ARRAY ? &*m_content.a->begin() : this; }
	const_iterator cbegin() const noexcept { return m_type == Type::ARRAY ? &*m_content.a->cbegin() : this; }
	iterator end() noexcept { return m_type == Type::ARRAY ? &*m_content.a->end() : this + 1; }
	const_iterator end() const noexcept { return m_type == Type::ARRAY ? &*m_content.a->end() : this + 1; }
	const_iterator cend() const noexcept { return m_type == Type::ARRAY ? &*m_content.a->cend() : this + 1; }


	PropVal& operator[](PropKey key) {
		if (m_type == Type::PROPS) return (*m_content.o)[key];
		else if (key.isInteger()) {
			Integer index = key.asInteger();
			if (m_type == Type::ARRAY) return (*m_content.a)[index];
			else if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
		else throw std::invalid_argument("Can't use non-integer key with non-Props PropVal value");
	}


	const PropVal& operator[](PropKey key) const {
		if (m_type == Type::PROPS) return (*m_content.o)[key];
		else if (key.isInteger()) {
			Integer index = key.asInteger();
			if (m_type == Type::ARRAY) return (*m_content.a)[index];
			else if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
		else throw std::invalid_argument("Can't use non-integer key with non-Props PropVal value");
	}


	PropVal& operator[](Integer index) {
		if (m_type == Type::PROPS) return operator[](PropKey(index));
		else if (m_type == Type::ARRAY) return (*m_content.a)[index];
		else if (index == 0) return *this;
		else throw std::out_of_range("PropVal of this type has fixed size 1");
	}


	const PropVal& operator[](Integer index) const {
		if (m_type == Type::PROPS) return operator[](PropKey(index));
		else if (m_type == Type::ARRAY) return (*m_content.a)[index];
		else if (index == 0) return *this;
		else throw std::out_of_range("PropVal of this type has fixed size 1");
	}


	PropVal& at(PropKey key) {
		if (m_type == Type::PROPS) {
			PropVal &result = m_content.o->at(key);
			if (result.isNone()) throw std::out_of_range("PropVal is None");
			return result;
		} else if (key.isInteger()) {
			Integer index = key.asInteger();
			if (m_type == Type::ARRAY) return m_content.a->at(index);
			else if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
		else throw std::invalid_argument("Can't use non-integer key with non-Props PropVal value");
	}


	const PropVal& at(PropKey key) const {
		if (m_type == Type::PROPS) {
			PropVal &result = m_content.o->at(key);
			if (result.isNone()) throw std::out_of_range("PropVal is None");
			return result;
		} else if (key.isInteger()) {
			Integer index = key.asInteger();
			if (m_type == Type::ARRAY) return m_content.a->at(index);
			else if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
		else throw std::invalid_argument("Can't use non-integer key with non-Props PropVal value");
	}


	PropVal& at(Integer index) {
		if (m_type == Type::PROPS) return at(PropKey(index));
		else if (m_type == Type::ARRAY) return m_content.a->at(index);
		else {
			if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
	}


	const PropVal& at(Integer index) const {
		if (m_type == Type::PROPS) return at(PropKey(index));
		else if (m_type == Type::ARRAY) return m_content.a->at(index);
		else {
			if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
	}


	const PropVal& atOrNone(PropKey key) const {
		if (m_type == Type::PROPS) {
			auto result = m_content.o->find(key);
			return (result != m_content.o->end() ? result->second : s_noneValue);
		} else if (m_type == Type::NONE) {
			return s_noneValue;
		} else if (key.isInteger()) {
			Integer index = key.asInteger();
			if (m_type == Type::ARRAY) return m_content.a->at(index);
			else if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
		else throw std::invalid_argument("Can't use non-integer key with non-Props PropVal value");
	}


	const PropVal& atOrNone(Integer index) const {
		if (m_type == Type::PROPS) {
			auto result = m_content.o->find(index);
			return (result != m_content.o->end() ? result->second : s_noneValue);
		} else if (m_type == Type::ARRAY) {
			return m_content.a->at(index);
		} else if (m_type == Type::NONE) {
			return s_noneValue;
		} else {
			if (index == 0) return *this;
			else throw std::out_of_range("PropVal of this type has fixed size 1");
		}
	}


	const bool contains(PropKey key) const {
		if (m_type == Type::PROPS) {
			auto found = m_content.o->find(key);
			return (found != m_content.o->end()) && !found->second.isNone();
		} else if (key.isInteger()) {
			Integer index = key.asInteger();
			if (m_type == Type::ARRAY) {
				if (index < 0) return false;
				else return (m_content.a->size() > size_t(index));
			}
			else if (index == 0) return !isNone();
			else return false;
		}
		else return false;
	}


	const bool contains(Integer index) const {
		if (m_type == Type::PROPS) return contains(PropKey(index));
		else if (m_type == Type::ARRAY) {
			if (index < 0) return false;
			else return (m_content.a->size() > size_t(index));
		} else {
			if (index == 0) return !isNone();
			else return false;
		}
	}


	friend void swap(PropVal &a, PropVal&b) noexcept {
		std::swap(a.m_type, b.m_type);
		swapMem(a.m_content, b.m_content);
	}


	friend bool operator==(const PropVal &a, const PropVal &b) {
		switch (a.m_type) {
			case Type::NONE: return b.m_type == Type::NONE;
			case Type::BOOL:
				switch (b.m_type) {
					case Type::BOOL: return a.m_content.b == b.m_content.b;
					case Type::INTEGER: return (a.m_content.b ? 1 : 0) == b.m_content.i;
					default: return false;
				}
			case Type::INTEGER:
				switch (b.m_type) {
					case Type::INTEGER: return a.m_content.i == b.m_content.i;
					case Type::BOOL: return a.m_content.i == (b.m_content.b ? 1 : 0);
					default: return false;
				}
			case Type::REAL: return (b.m_type == Type::REAL) &&
				(a.m_content.r == b.m_content.r);
			default: return a.comparisonImpl(b);
		}
	}

	friend bool operator!=(const PropVal &a, const PropVal &b) { return ! operator==(a, b); }

	friend bool operator==(const Props &a, const Props &b);

	friend bool operator!=(const Props &a, const Props &b) { return ! operator==(a, b); }


	PropVal& operator=(PropVal other) {
		using namespace std;
		swap(*this, other);
		return *this;
	}


	void substVars(const Props &varValues = Props(), bool useEnvVars = false, bool ignoreMissing = false);


	template <typename T> static void toJSON(std::ostream &out, const T &x) { out << x; }
	static void toJSON(std::ostream &out, None x) { out << "null"; }
	static void toJSON(std::ostream &out, Bool x) { out << (x ? "true" : "false"); }
	static void toJSON(std::ostream &out, Real x);
	static void toJSON(std::ostream &out, const Name x);
	static void toJSON(std::ostream &out, const Bytes &x);
	static void toJSON(std::ostream &out, const String &x) { PropKey::toJSON(out, x); }
	static void toJSON(std::ostream &out, const Array &x);
	static void toJSON(std::ostream &out, const Props &x);


	void toJSON(std::ostream &out) const;
	static PropVal fromJSON(std::istream &in);

	std::string toJSON() const;
	static PropVal fromJSON(const std::string &in);

	void toFile(const std::string &outFileName) const;
	static PropVal fromFile(const std::string &inFileName);


	static PropVal fromString(const std::string &in);

	std::ostream& print(std::ostream &os) const override;


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

	PropVal(uint32_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {}

	PropVal(int64_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {}

	PropVal(uint64_t value) : m_type(Type::INTEGER), m_content(Integer(value)) {
		if (uint64_t(m_content.i) != value) throw std::bad_cast();
	}

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
	PropVal(String &&value) : m_type(Type::STRING), m_content(std::move(value)) {}

	PropVal(const char* value) : PropVal(std::string(value)) {}

	PropVal(const Bytes &value) : m_type(Type::BYTES), m_content(value) {}
	PropVal(Bytes &&value) : m_type(Type::BYTES), m_content(std::move(value)) {}

	PropVal(const Array &value) : m_type(Type::ARRAY), m_content(value) {}
	PropVal(Array &&value) : m_type(Type::ARRAY), m_content(std::move(value)) {}
	PropVal(std::initializer_list<PropVal> init) : m_type(Type::ARRAY), m_content(init) { }

	PropVal(const Props &value) : m_type(Type::PROPS), m_content(value) {}
	PropVal(Props &&value) : m_type(Type::PROPS), m_content(std::move(value)) {}


	PropVal(PropKey x) {
		if (x.isInteger()) { m_content.i = x.asInteger(); m_type = Type::INTEGER; }
		else {  m_content.n = x.asName(); m_type = Type::NAME; }
	}

	operator PropKey () const {
		switch (m_type) {
			case Type::INTEGER: return PropKey(m_content.i);
			case Type::NAME: return PropKey(m_content.n);
			case Type::STRING: return PropKey(m_content.s);
			default: throw std::bad_cast();
		}
	}


	template<typename T, typename = decltype(assign_from(std::declval<PropVal&>(), std::declval<T>()))>
		explicit PropVal(const T &x) : PropVal() { assign_from(*this, x); }


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


	static PropVal props() { return PropVal(Type::PROPS); }

	static PropVal props(std::initializer_list<Prop> init)
		{ return PropVal(Props(init)); }


	~PropVal() {
		if (m_type > Type::NAME) destructorImpl();
	}


protected:
	static Props diff(const Props &a, const Props &b);

	static Props& patchMerge(Props &a, Props b, bool merge);

public:
	// Necessary for Props, etc., despite inheriting from Printable:
	friend std::string to_string(const PropVal &propVal) { return propVal.toString(); }

	friend Props operator-(const Props &a, const Props &b) { return diff(a, b); }

	friend Props& operator+=(Props &a, Props b) { return patchMerge(a, std::move(b), false); }

	friend Props operator+(Props a, const Props b) { return operator+=(a, std::move(b)); }

	friend Props& operator&=(Props &a, Props b) { return patchMerge(a, std::move(b), true); }

	friend Props operator&(Props a, const Props b) { return operator&=(a, std::move(b)); }
};



inline void assign_from(PropVal &to, const PropVal &from) { to = from; }


template<typename T, typename = decltype(std::declval<T&>() = std::declval<const PropVal&>())>
	void assign_from(T& to, const PropVal& from) { to = from; }

template<typename T, typename = decltype(std::declval<PropVal&>() = std::declval<const T&>())>
	void assign_from(PropVal& to, const T& from) { to = from; }


inline void assign_from(bool &to, const PropVal &from) { to = from.asBool(); }
inline void assign_from(int8_t &to, const PropVal &from) { to = from.asInt32(); }
inline void assign_from(uint8_t &to, const PropVal &from) { to = from.asInt32(); }
inline void assign_from(int16_t &to, const PropVal &from) { to = from.asInt32(); }
inline void assign_from(uint16_t &to, const PropVal &from) { to = from.asInt32(); }
inline void assign_from(int32_t &to, const PropVal &from) { to = from.asInt32(); }
inline void assign_from(uint32_t &to, const PropVal &from) { to = from.asInt32(); }
inline void assign_from(int64_t &to, const PropVal &from) { to = from.asLong64(); }
inline void assign_from(uint64_t &to, const PropVal &from) { to = from.asLong64(); }
inline void assign_from(float &to, const PropVal &from) { to = from.asDouble(); }
inline void assign_from(double &to, const PropVal &from) { to = from.asDouble(); }
inline void assign_from(Name &to, const PropVal &from) { to = from.asName(); }
inline void assign_from(std::string &to, const PropVal &from) { to = from.asString(); }
inline void assign_from(PropVal::Bytes &to, const PropVal &from) { to = from.asBytes(); }
inline void assign_from(PropVal::Array &to, const PropVal &from) { to = from.asArray(); }
inline void assign_from(PropVal::Props &to, const PropVal &from) { to = from.asProps(); }


inline void assign_from(PropVal &to, const PropVal::Bytes &from) { to = PropVal(from); }


template<typename T, typename Alloc>
auto assign_from(std::vector<T, Alloc> &to, const PropVal &from) -> decltype(assign_from(to.front(), from)) {
	const PropVal::Array &a = from.asArray();
	to.clear();
	to.reserve(a.size());
	for (const auto& x: a) { T tmp; assign_from(tmp, x); to.push_back(std::move(tmp)); }
}

template<typename T, typename Alloc>
auto assign_from(PropVal &to, const std::vector<T, Alloc> &from) -> decltype(assign_from(to, from.front())) {
	if (to.isArray()) to.asArray().clear();
	else to = PropVal::array();
	PropVal::Array &a = to.asArray();
	a.reserve(from.size());
	for (const auto& x: from) { PropVal tmp; assign_from(tmp, x); a.push_back(std::move(tmp)); }
}


template<typename T, typename Alloc>
auto assign_from(std::list<T, Alloc> &to, const PropVal &from) -> decltype(assign_from(to.front(), from)) {
	const PropVal::Array &a = from.asArray();
	to.clear();
	for (const auto& x: a) { T tmp; assign_from(tmp, x); to.push_back(std::move(tmp)); }
}

template<typename T, typename Alloc>
auto assign_from(PropVal &to, const std::list<T, Alloc> &from) -> decltype(assign_from(to, from.front())) {
	if (to.isArray()) to.asArray().clear();
	else to = PropVal::array();
	PropVal::Array &a = to.asArray();
	a.reserve(from.size());
	for (const auto& x: from) { PropVal tmp; assign_from(tmp, x); a.push_back(std::move(tmp)); }
}


template<typename K, typename V, typename Compare, typename Alloc>
void assign_from(std::map<K, V, Compare, Alloc> &to, const PropVal &from) {
	to.clear();
	for (const auto& x: from.asProps()) {
		K tmpKey; assign_from(tmpKey, x.first);
		V tmpVal; assign_from(tmpVal, x.second);
		to[std::move(tmpKey)] = std::move(tmpVal);
	}
}

template<typename K, typename V, typename Compare, typename Alloc>
void assign_from(PropVal::Props &to, const std::map<K, V, Compare, Alloc> &from) {
	to.clear();
	for (const auto& x: from) {
		PropKey tmpKey; assign_from(tmpKey, x.first);
		PropVal tmpVal; assign_from(tmpVal, x.second);
		to[std::move(tmpKey)] = std::move(tmpVal);
	}
}

template<typename K, typename V, typename Compare, typename Alloc>
void assign_from(PropVal &to, const std::map<K, V, Compare, Alloc> &from) {
	if (! to.isProps()) to = PropVal::props();
	assign_from(to.asProps(), from);
}


using Props = PropVal::Props;


// Necessary for Props, etc., despite inheriting from Printable:
inline std::ostream& operator<<(std::ostream &os, const PropVal &value)
	{ return value.print(os); }



class PropPath final: public virtual Printable {
public:
	using Elements = std::vector<PropKey>;
protected:
	Elements m_elements;

public:
	using iterator = Elements::iterator;
	using const_iterator = Elements::const_iterator;

	class Fragment final: public virtual Printable {
	public:
		using const_iterator = typename PropPath::const_iterator;
		using size_type = typename const_iterator::difference_type;

	protected:
		const_iterator m_begin{};
		const_iterator m_end{};

	public:
		bool empty() const { return m_begin == m_end; }
		size_type size() const { return m_end - m_begin; }

		const_iterator begin() const noexcept { return m_begin; }
		const_iterator cbegin() const noexcept { return m_begin; }
		const_iterator end() const noexcept { return m_end; }
		const_iterator cend() const noexcept { return m_end; }

		PropKey front() const { return *begin(); }
		Fragment tail() const { const_iterator from = begin(); return Fragment(++from, end()); }

		std::ostream& print(std::ostream &os) const override;

		Fragment() {}
		Fragment(const_iterator from, const_iterator until): m_begin(from), m_end(until){}
		Fragment(const PropPath &path): m_begin(path.begin()), m_end(path.end()) {}
	};

	bool empty() const { return m_elements.empty(); }

	Elements& elements() noexcept { return m_elements; }
	const Elements& elements() const noexcept { return m_elements; }

	iterator begin() noexcept { return m_elements.begin(); }
	const_iterator begin() const noexcept { return m_elements.begin(); }
	const_iterator cbegin() const noexcept { return m_elements.cbegin(); }
	iterator end() noexcept { return m_elements.end(); }
	const_iterator end() const noexcept { return m_elements.end(); }
	const_iterator cend() const noexcept { return m_elements.cend(); }

	PropPath& operator%=(PropKey key) {
		m_elements.reserve(4); // minimum capacity
		m_elements.push_back(key);
		return *this;
	}

	PropPath& operator+=(const PropPath& other) {
		for (PropKey key: other.m_elements) operator%=(key);
		return *this;
	}

	std::ostream& print(std::ostream &os) const override { return Fragment(*this).print(os); }

	operator PropVal () const { return PropVal(toString()); }

	PropPath& operator=(const PropPath& other) = default;
	PropPath& operator=(PropPath&& other) = default;

	PropPath& operator=(const std::vector<PropKey>& path) { m_elements = path; return *this; }
	PropPath& operator=(std::vector<PropKey>&& path) { m_elements = std::move(path); return *this; }

	PropPath& operator=(PropKey key) { m_elements.clear(); m_elements.push_back(key); return *this; }
	PropPath& operator=(Name name) { return *this = PropKey(name); }
	PropPath& operator=(PropVal::Integer i) { return *this = PropKey(i); }

	PropPath& operator=(const std::string& path);
	PropPath& operator=(const char *path) { return *this = std::string(path); }

	PropPath& operator=(const PropVal& propVal);

	PropPath() { }

	PropPath(const PropPath& other) { *this = other; }
	PropPath(PropPath&& other) { *this = std::move(other); }

	PropPath(std::initializer_list<PropKey> elements): m_elements(elements.begin(), elements.end()) {}

	PropPath(std::vector<PropKey> path) { *this = std::move(path); }

	PropPath(PropKey key) { *this = key; }
	PropPath(Name name) { *this = name; }
	PropPath(PropVal::Integer i) { *this = i; }

	PropPath(const std::string& path) { *this = path; }
	PropPath(const char *path) { *this = path; }

	PropPath(const PropVal& path) { *this = path; }

	friend PropPath operator%(PropPath a, PropKey b) { a %= b; return a; }
	friend PropPath operator+(PropPath a, const PropPath& b) { a += b; return a; }
};



class Configurable {
public:
	virtual void applyConfig(const PropVal& config) = 0;
	virtual PropVal getConfig() const = 0;

	virtual ~Configurable() {}
};


} // namespace dbrx



namespace std {


template<> struct hash<dbrx::PropKey> {
	size_t operator()(const dbrx::PropKey key) const { return key.hash(); }
};


} // namespace std


#endif // DBRX_PROPS_H
