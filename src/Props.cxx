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


#include "Props.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "external/rapidjson/reader.h"
#include "external/rapidjson/genericstream.h"

#include <TString.h>


using namespace std;


namespace dbrx {


void PropKey::toJSON(std::ostream &out, const String &x) {
	out << "\"";
    for (char c: x) {
        switch (c) {
            case '\\': out << "\\\\"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '"': out << "\\\""; break;
            default: out << c; break;
        }
    }
	out << "\"";
}


void PropKey::toJSON(std::ostream &out) const {
	switch (m_type) {
		case Type::INTEGER: out << "\"" << m_content.i << "\""; break;
		case Type::NAME: toJSON(out, m_content.n.str()); break;
		default: assert(false);
	}
}


std::string PropKey::toJSON() const {
	stringstream tmp;
	toJSON(tmp);
	return tmp.str();
}


std::ostream& PropKey::print(std::ostream &os) const {
	switch (m_type) {
		case Type::INTEGER: os << m_content.i; break;
		case Type::NAME: os << m_content.n; break;
		default: assert(false);
	}
	return os;
}


std::string PropKey::toString() const {
	stringstream out;
	print(out);
	return out.str();
}


PropKey::PropKey(const std::string &value) {
	try {
		long long i = stoll(value);
		m_content.i = Integer(i);
		if (m_content.i != i) throw std::out_of_range("Integer key out of range");
	}
	catch (std::invalid_argument &e) {
		m_content.n = value;
		m_type = Type::NAME;
	}
}


bool PropVal::comparisonImpl(const PropVal &other) const {
	switch (m_type) {
		case Type::NAME:
			switch (other.m_type) {
				case Type::NAME: return m_content.n == other.m_content.n;
				case Type::STRING: return m_content.n.str() == other.m_content.s;
				default: return false;
			}
		case Type::STRING:
			switch (other.m_type) {
				case Type::STRING: return m_content.s == other.m_content.s;
				case Type::NAME: return m_content.s == other.m_content.n.str();
				default: return false;
			}
		case Type::ARRAY: return (other.m_type == Type::ARRAY) && (*m_content.a == *other.m_content.a);
		case Type::PROPS: return (other.m_type == Type::PROPS) && (*m_content.o == *other.m_content.o);
		default: assert(false);
	}
}


bool operator==(const Props &a, const Props &b) {
	auto itA = a.begin(), itB = b.begin();

	while ((itA != a.end()) && (itB != b.end())) {
		const auto &keyA = itA->first;
		const auto &valA = itA->second;
		const auto &keyB = itB->first;
		const auto &valB = itB->second;

		if (keyA == keyB) {
			if (valA != valB) return false;
			++itA; ++itB;
		} else if (PropKey::CompareById()(keyA, keyB)) {
			if (! valA.isNone()) return false;
			++itA;
		} else {
			if (! valB.isNone()) return false;
			++itB;
		}
	}

	while (itA != a.end()) {
		const auto &valA = itA->second;
		if (! valA.isNone()) return false;
		++itA;
	}

	while (itB != b.end()) {
		const auto &valB = itB->second;
		if (! valB.isNone()) return false;
		++itB;
	}

	return true;
}


void PropVal::toJSON(std::ostream &out, Real x) {
	out.precision(17);
	out << x;
}


void PropVal::toJSON(std::ostream &out, Name x) {
	toJSON(out, x.str());
}


void PropVal::toJSON(std::ostream &out, const Array &x) {
	out << "[";
	bool first = true;
	for (auto const &v: x) {
		if (!first) out << ", ";
		v.toJSON(out);
		first = false;
	}
	out << "]";
}


void PropVal::toJSON(std::ostream &out, const Props &x) {
	using CPRef = std::pair<PropKey, const PropVal*>;
	vector<CPRef> props;
	props.reserve(x.size());

	for (auto const &e: x) props.push_back({e.first, &e.second});
	auto compare = [](const CPRef &a, const CPRef &b) { return a.first < b.first; };
	sort(props.begin(), props.end(), compare);

	out << "{";
	bool first = true;
	for (auto const &e: props) {
		if (!first) out << ", ";
		e.first.toJSON(out);
		out << ": ";
		e.second->toJSON(out);
		first = false;
	}
	out << "}";
}


void PropVal::toJSON(std::ostream &out) const {
	switch (m_type) {
		case Type::NONE: toJSON(out, m_content.e); break;
		case Type::BOOL: toJSON(out, m_content.b); break;
		case Type::INTEGER: toJSON(out, m_content.i); break;
		case Type::REAL: toJSON(out, m_content.r); break;
		case Type::NAME: toJSON(out, m_content.n); break;
		case Type::STRING: toJSON(out, m_content.s); break;
		case Type::ARRAY: toJSON(out, *m_content.a); break;
		case Type::PROPS: toJSON(out, *m_content.o); break;
		default: assert(false);
	}
}


PropVal PropVal::fromJSON(std::istream &in) {
	using Encoding = typename rapidjson::UTF8<>;
	using rapidjson::SizeType;

	class JSONHandler {
	protected:
		std::vector<PropVal> m_valueStack;

		void popCurrent() { m_valueStack.pop_back(); }

		void store(PropVal &&pIn) {
			if (m_valueStack.capacity() == m_valueStack.size())
				m_valueStack.reserve(m_valueStack.size() * 2);
			m_valueStack.push_back(std::move(pIn));
		}

	public:
		using Ch = typename Encoding::Ch;

		void Null_() { store(PropVal()); }

		void Bool_(bool b) { store(b); }

		void Int(int i) { store(int64_t(i)); }
		void Uint(unsigned i) { store(int64_t(i)); }
		void Int64(int64_t i) { store(i); }
		void Uint64(uint64_t i) { store(int64_t(i)); }

		void Double(double d) { store(d); }

		void String(const Ch* str, SizeType length, bool copy) { store(string(str, length)); }

		void StartObject() {}

		void EndObject(SizeType memberCount) {
			if (m_valueStack.size() < memberCount * 2) throw logic_error("Parsing stack size mismatch on object");
			PropVal objProp = PropVal::props();
			Props obj = objProp.asProps();
			for (auto p = m_valueStack.end() - memberCount * 2; p < m_valueStack.end();) {
				PropKey k(std::move(*p++));
				obj[std::move(k)] = std::move(*p++);
			}
			m_valueStack.resize(m_valueStack.size() - 2 * memberCount);
			store(std::move(obj));
		}

		void StartArray() {}

		void EndArray(SizeType elementCount) {
			if (m_valueStack.size() < elementCount) throw logic_error("Parsing stack size mismatch on array");
			PropVal arrayProp = PropVal::array();
			Array arr = arrayProp.asArray();
			arr.reserve(elementCount);
			for (auto p = m_valueStack.end() - elementCount; p < m_valueStack.end(); ++p)
				arr.push_back(std::move(*p));
			m_valueStack.resize(m_valueStack.size() - elementCount);
			store(std::move(arr));
		}

		PropVal& getResult() {
			if (m_valueStack.size() != 1) throw logic_error("Parsing stack size mismatch - expected size 1");
			return m_valueStack.front();
		}

		JSONHandler() {
			m_valueStack.reserve(8);
		}
	};

	rapidjson::GenericReadStream genericIn(in);

	rapidjson::GenericReader<Encoding> reader;
	JSONHandler handler;

	if (! reader.Parse<rapidjson::kParseDefaultFlags, rapidjson::GenericReadStream, JSONHandler> (genericIn, handler))
		throw runtime_error("JSON parse error");

	return PropVal( std::move(handler.getResult()) );
}


std::string PropVal::toJSON() const {
	stringstream tmp;
	toJSON(tmp);
	return tmp.str();
}


PropVal PropVal::fromJSON(const std::string &in) {
	stringstream tmp(in);
	return fromJSON(tmp);
}


void PropVal::toFile(const std::string &outFileName) const {
	TString fileName(outFileName);

	if (fileName.EndsWith(".json")) {
		ofstream out(fileName.Data());
		toJSON(out);
		out << "\n";
	} else throw runtime_error("Unsupported input file type for PropVal");
}


PropVal PropVal::fromFile(const std::string &inFileName) {
	TString fileName(inFileName);

	if (fileName.EndsWith(".json")) {
		ifstream in(fileName.Data());
		return fromJSON(in);
	} else throw runtime_error("Unsupported input file type for PropVal");
}


std::ostream& PropVal::print(std::ostream &os) const {
	switch (m_type) {
		case Type::NAME: os << m_content.n; break;
		case Type::STRING: os << m_content.s; break;
		default: toJSON(os);
	}
	return os;
}


std::string PropVal::toString() const {
	stringstream out;
	print(out);
	return out.str();
}


Props PropVal::diff(const Props &a, const Props &b) {
	auto itA = a.begin(), itB = b.begin();

	Props result;

	while ((itA != a.end()) && (itB != b.end())) {
		const auto &keyA = itA->first;
		const auto &valA = itA->second;
		const auto &keyB = itB->first;
		const auto &valB = itB->second;

		if (keyA == keyB) {
			if (valA.isProps() && valB.isProps()) {
				Props d = operator-(valA.asProps(), valB.asProps());
				if (! d.empty()) result[keyA] = PropVal(std::move(d));
			}
			else if (valA != valB) result[keyA] = valA;
			++itA; ++itB;
		} else if (PropKey::CompareById()(keyA, keyB)) {
			result[keyA] = valA;
			++itA;
		} else {
			result[keyB] = PropVal();
			++itB;
		}
	}

	while (itA != a.end()) {
		const auto &keyA = itA->first;
		const auto &valA = itA->second;
		result[keyA] = valA;
		++itA;
	}

	while (itB != b.end()) {
		const auto &keyB = itB->first;
		result[keyB] = PropVal();
		++itB;
	}

	return result;
}


Props& PropVal::patchMerge(Props &a, Props b, bool merge) {
	auto itA = a.begin(), itB = b.begin();

	while ((itA != a.end()) && (itB != b.end())) {
		const auto &keyA = itA->first;
		auto &valA = itA->second;
		const auto &keyB = itB->first;
		auto &valB = itB->second;

		if (keyA == keyB) {
			if (valA.isProps() && valB.isProps())
				patchMerge(valA.asProps(), std::move(valB.asProps()), merge);
			else {
				if ((merge) && (valA != valB))
					throw invalid_argument("Can't merge Props with conflicting contents");
				else valA = std::move(valB);
			}
			++itA; ++itB;
		} else if (PropKey::CompareById()(keyA, keyB)) {
			++itA;
		} else {
			a[keyB] = std::move(valB);
			++itB;
		}
	}

	while (itB != b.end()) {
		const auto &keyB = itB->first;
		auto &valB = itB->second;
		a[keyB] = std::move(valB);
		++itB;
	}

	return a;
}



std::string PropPath::SubPath::toString() const {
	stringstream out;
	print(out);
	return out.str();
}


std::ostream& PropPath::SubPath::print(std::ostream &os) const {
	bool first = true;
	for (PropKey key: *this) {
		if (first) first = false; else os << ".";
		os << key;
	}
	return os;
}


PropPath& PropPath::operator=(const std::string& path) {
	size_t from = 0;
	for (size_t i = 0; i < path.size(); ++i) {
		if (path[i] == '.') {
			m_elements.push_back(PropKey(path.substr(from, i - from)));
			if (i + 1 == path.size()) m_elements.push_back(Name());
			from = ++i;
			//if (i == path.size()) m_elements.push_back(Name());
		}
	}
	if (from < path.size()) m_elements.push_back(PropKey(path.substr(from, path.size() - from)));
	return *this;
}


PropPath& PropPath::operator=(const PropVal& propVal) {
	if (propVal.isName()) *this = propVal.asName();
	else if (propVal.isString()) *this = propVal.asString();
	else if (propVal.isArray()) {
		const auto& a = propVal.asArray();
		m_elements.reserve(a.size());
		for (const auto& x: a) m_elements.push_back(x);
	} else throw std::invalid_argument("Can't initialize PropPath from content of this PropVal");
	return *this;
}




} // namespace dbrxq
