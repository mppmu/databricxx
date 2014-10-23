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


#ifndef DBRX_NAME_H
#define DBRX_NAME_H

#include <string>
#include <functional>
#include <iosfwd>


namespace dbrx {


class NameTable;


class Name final {
protected:
	static const std::string s_emptyString;

	const std::string *m_value = nullptr;

	Name(const std::string *ptr) : m_value(ptr) {}

public:
	using Id = intptr_t;

	struct CompareById {
		bool operator() (Name a, Name b) { return a.id() < b.id(); }
	};

	Id id() const { return Id(m_value); }

	size_t hash() const { return std::hash<Id>()(id()); }

	bool empty() const { return m_value == nullptr; }

	const std::string& str() const { return (m_value != nullptr) ? *m_value : s_emptyString; }
	const char* c_str() const { return m_value->c_str(); }

	operator const std::string& () const { return str(); }
	operator const char* () const { return str().c_str(); }

	bool operator==(Name other) const { return m_value == other.m_value; }
	bool operator!=(Name other) const { return m_value != other.m_value; }
	bool operator<(Name other) const { return str() < other.str(); }
	bool operator>(Name other) const { return str() > other.str(); }
	bool operator<=(Name other) const { return operator==(other) || operator<(other); }
	bool operator>=(Name other) const { return operator==(other) || operator>(other); }

	Name& operator=(const Name &other) { m_value = other.m_value; return *this; }

	Name() = default;

	Name(const Name &other) { operator=(other); }

	Name(const std::string &s);
	Name(const std::string &s, NameTable &table);

	Name(const char *s) : Name(std::string(s)) { }

	friend class NameTable;

	friend const std::string& to_string(Name name) { return name.str(); }
};


inline std::ostream & operator<<(std::ostream &os, Name n) {
	return os << n.str();
}


} // namespace dbrx



namespace std {


template<> struct hash<dbrx::Name> {
	size_t operator()(dbrx::Name name) const { return name.hash(); }
};


} // namespace std


#endif // DBRX_NAME_H
