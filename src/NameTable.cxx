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


#include "NameTable.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

using namespace std;


namespace dbrx {

struct NameTable::Internals {
	using Mutex = std::mutex;
	using LockGuard = std::lock_guard<Mutex>;

	class ConstStringRef {
	protected:
		const std::string *m_ptr = nullptr;

	public:
		struct Hash {
		protected:
			std::hash<std::string> m_hash;
		public:
			size_t operator()(const ConstStringRef& sr) const { return m_hash(sr.value()); }
		};

		const std::string& value() const { return *m_ptr; }
		const std::string* ptr() const { return m_ptr; }

		bool operator==(ConstStringRef other) const { return value() == other.value(); }

		ConstStringRef() = default;
		ConstStringRef(const std::string *p): m_ptr(p) {}
	};


	Mutex mutex;
	std::vector< std::unique_ptr<std::string> > strings;
	std::unordered_map<ConstStringRef, Name, ConstStringRef::Hash> stringMap;
};


NameTable& NameTable::global() {
	static NameTable globalTable;
	return globalTable;
}


Name NameTable::resolve(const std::string &s) {
	if (s.empty()) return Name();
	else {
		auto &strings = m_internals->strings;
		auto &stringMap = m_internals->stringMap;
		using ConstStringRef = Internals::ConstStringRef;

		Internals::LockGuard lock(m_internals->mutex);
		auto found = stringMap.find(ConstStringRef(&s));
		if (found != stringMap.end()) return found->second;
		else {
			std::unique_ptr<std::string> stringPtr(new std::string(s));
			ConstStringRef sr(&*stringPtr);
			Name name(&*stringPtr);

			if (strings.size() == strings.capacity())
				strings.reserve(std::max(size_t(16), strings.size() * 2));
			strings.push_back(std::move(stringPtr));

			return stringMap[sr] = name;
		}
	}
}


NameTable::NameTable()
	: m_internals(new NameTable::Internals)
{}


NameTable::~NameTable() {
	delete m_internals;
}


} // namespace dbrx
