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
#include <TSystem.h>
#include <TBase64.h>

#include "format.h"


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
		case Type::BYTES: return (other.m_type == Type::BYTES) && (*m_content.y == *other.m_content.y);
		case Type::ARRAY: return (other.m_type == Type::ARRAY) && (*m_content.a == *other.m_content.a);
		case Type::PROPS: return (other.m_type == Type::PROPS) && (*m_content.o == *other.m_content.o);
		default: assert(false);
	}
}


bool PropVal::substVarsImplContainsVar(const std::string &input) {
	size_t nEscapes = 0;

	for (size_t pos = 0; pos < input.size(); ++pos) {
		char c = input[pos];
		if (c == '\\') {
			++nEscapes;
		} else {
			if ((c == '$') && (nEscapes % 2 == 0) && (pos + 1 < input.size())) {
				return true;
			}
			nEscapes = 0;
		}
	}

	return false;
}


PropVal PropVal::substVarsImplSubstVars(const std::string &input, const Props &varValues, Props* envVarValues, bool ignoreMissing) {
	size_t npos = input.npos;
	stringstream result;

	size_t nEscapes = 0;
	size_t varBegin = npos;
	size_t varEnd = npos;
	bool varBraces = false;
	size_t pos = 0;

	while (pos < input.size()) {
		char c = input[pos];
		if (varBegin == npos) {
			if (c == '\\') {
				++nEscapes;
				result << c;
			} else {
				if ((c == '$') && (nEscapes % 2 == 0) && (pos + 1 < input.size())) {
					varBegin = pos + 1;
					// if (! (varBegin < input.size())) throw invalid_argument("Encountered \"$\" at end of string \"%s\" during variable substitution"_format(input));
				} else {
					result << c;
				}
				nEscapes = 0;
			}
			++pos;
		} else {
			if (c == '{') {
				if (varBraces) {
					throw invalid_argument("Encountered extra \"{\" during variable substitution in string \"%s\""_format(input));
				} else if (pos == varBegin) {
					varBegin = pos + 1;
					varBraces = true;
					// if (! (varBegin < input.size())) throw invalid_argument("Encountered \"${\" at end of string \"%s\" during variable substitution"_format(input));
				} else {
					varEnd = pos;
				}
			} else {
				if (!isalnum(c) && (c != '_')) {
					if (varBraces) {
						if (c == '}') {
							varEnd = pos;
							++pos;
						} else if (c == '\\') {
							throw invalid_argument("Encountered illegal \"\\\" character inside \"${...}\" during variable substitution in string \"%s\""_format(input));
						}
					} else {
						varEnd = pos;
					}
				} else if (isdigit(c) && (pos == varBegin)) {
					throw invalid_argument("Illegal variable name, starting with a digit, during variable substitution in string \"%s\""_format(input));
				}
			}

			if ( (varEnd == npos) && (pos + 1 == input.size()) ) {
				if (varBraces) {
					throw invalid_argument("Missing \"}\" for \"${\" during variable substitution in string \"%s\""_format(input));
				} else {
					++pos;
					varEnd = pos;
				}
			}

			if (varEnd != npos) {
				if (varEnd > varBegin) {
					Name varName(string(&input[varBegin], varEnd - varBegin));

					const PropVal* foundValue = nullptr;

					auto found = varValues.find(varName);
					if (found != varValues.end()) {
						foundValue = &found->second;
					} else if (envVarValues) {
						auto foundEnv = envVarValues->find(varName);
						if (foundEnv != envVarValues->end()) {
							foundValue = &foundEnv->second;
						} else {
							const char* valPtr = gSystem->Getenv(varName.c_str());
							PropVal value = PropVal::fromString(valPtr ? valPtr : "");
							(*envVarValues)[varName] = value;
							foundValue = &envVarValues->at(varName);
						}
					}

					size_t varExprBegin = varBraces ? varBegin - 2 : varBegin - 1;
					size_t varExprEnd = varBraces ? varEnd + 1 : varEnd;
					if (foundValue) {
						if ((varExprBegin == 0) && (varExprEnd == input.size())) {
							return *foundValue;
						}
						result << *foundValue;
					} else {
						if (ignoreMissing) for (size_t i = varExprBegin; i < varExprEnd; ++i) result << input[i];
						else throw invalid_argument("Unknown variable \"%s\" during variable substitution in string \"%s\""_format(varName, input));
					}
				} else {
					if (varBraces) {
						throw invalid_argument("Encountered illegal \"${}\" during variable substitution in string \"%s\""_format(input));
					} else {
						result << input[pos-1] << input[pos];
						++pos;
					}
				}
				varBegin = npos;
				varEnd = npos;
				varBraces = false;
			} else {
				++pos;
			}
		}
	}

	return result.str();
}


void PropVal::substVarsImpl(const Props &varValues, Props* envVarValues, bool ignoreMissing) {
	if (m_type == Type::STRING) {
		const std::string &x = m_content.s;
		if (substVarsImplContainsVar(x))
			*this = substVarsImplSubstVars(x, varValues, envVarValues, ignoreMissing);
	} else if (m_type == Type::ARRAY) {
		Array &x = *m_content.a;
		for (auto &v: x) {
			v.substVarsImpl(varValues, envVarValues, ignoreMissing);
		}
	} else if (m_type == Type::PROPS) {
		Props &x = *m_content.o;
		for (auto &e: x) {
			e.second.substVarsImpl(varValues, envVarValues, ignoreMissing);
		}
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


void PropVal::substVars(const Props &varValues, bool useEnvVars, bool ignoreMissing) {
	if (useEnvVars) {
		Props envVarValues;
		substVarsImpl(varValues, &envVarValues, ignoreMissing);
	} else {
		substVarsImpl(varValues, nullptr, ignoreMissing);
	}
}


void PropVal::toJSON(std::ostream &out, Real x) {
	out.precision(16);
	out << x;
}


void PropVal::toJSON(std::ostream &out, Name x) {
	toJSON(out, x.str());
}


void PropVal::toJSON(std::ostream &out, const Bytes &x) {
	out << "\"data:,";
	out << TBase64::Encode((const char*)(x.data()), x.size());
	out << "\"";
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
		case Type::BYTES: toJSON(out, *m_content.y); break;
		case Type::ARRAY: toJSON(out, *m_content.a); break;
		case Type::PROPS: toJSON(out, *m_content.o); break;
		default: assert(false);
	}
}


PropVal PropVal::fromJSON(std::istream &in) {
	using Encoding = typename rapidjson::UTF8<>;
	using rapidjson::SizeType;

	class JSONHandler {
	public:
		using Ch = typename Encoding::Ch;

	protected:
		std::vector<PropVal> m_valueStack;

		PropVal decodeString(const Ch* str, SizeType length) {
			if ((length >= 6) && (str[0] == 'd') && (str[1] == 'a') && (str[2] == 't') && (str[3] == 'a') && (str[4] == ':') && (str[5] == ',')) {
				assert(str[length] == 0);
				TString decoded = TBase64::Decode(str + 6);
				Bytes bytes(decoded.Length());
				const char* decodedData = decoded.Data();
				for (size_t i = 0; i < bytes.size(); ++i) bytes[i] = uint8_t(decodedData[i]);
				return bytes;
			} else {
				return string(str, length);
			}
		}

		void popCurrent() { m_valueStack.pop_back(); }

		void store(PropVal &&pIn) {
			if (m_valueStack.capacity() == m_valueStack.size())
				m_valueStack.reserve(m_valueStack.size() * 2);
			m_valueStack.push_back(std::move(pIn));
		}

	public:
		void Null_() { store(PropVal()); }

		void Bool_(bool b) { store(b); }

		void Int(int i) { store(int64_t(i)); }
		void Uint(unsigned i) { store(int64_t(i)); }
		void Int64(int64_t i) { store(i); }
		void Uint64(uint64_t i) { store(int64_t(i)); }

		void Double(double d) { store(d); }

		void String(const Ch* str, SizeType length, bool copy) { store(decodeString(str, length)); }

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
		throw invalid_argument("JSON parse error, input is not valid JSON");

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


PropVal PropVal::fromString(const std::string &in) {
	// fromJSON currently can't handle non-object input (due to RapidJSON),
	// so we have to check for numeric, bool and null values explicitly:
	try {
		size_t pos = 0;
		Integer x = std::stoll(in, &pos);
		if (pos != in.size()) throw std::invalid_argument("");
		return PropVal(x);
	} catch(std::invalid_argument &e) {
		try {
			size_t pos = 0;
			Real x = std::stod(in, &pos);
			if (pos != in.size()) throw std::invalid_argument("");
			return PropVal(x);
		} catch(std::invalid_argument &e) {
			try {
				return PropVal::fromJSON(in);
			} catch(std::invalid_argument &e) {
				if (in == "null") return PropVal();
				else if (in == "true") return PropVal(true);
				else if (in == "false") return PropVal(false);
				else return PropVal(in);
			}
		}
	}
}


std::ostream& PropVal::print(std::ostream &os) const {
	switch (m_type) {
		case Type::NAME: os << m_content.n; break;
		case Type::STRING: os << m_content.s; break;
		default: toJSON(os);
	}
	return os;
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



std::ostream& PropPath::Fragment::print(std::ostream &os) const {
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
	else if (propVal.isInteger()) *this = propVal.asInteger();
	else if (propVal.isString()) *this = propVal.asString();
	else if (propVal.isArray()) {
		const auto& a = propVal.asArray();
		m_elements.reserve(a.size());
		for (const auto& x: a) m_elements.push_back(x);
	} else throw std::invalid_argument("Can't initialize PropPath from content of this PropVal");
	return *this;
}




} // namespace dbrxq
