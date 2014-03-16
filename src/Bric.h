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


#ifndef DBRX_BRIC_H
#define DBRX_BRIC_H

#include <stdexcept>
#include <memory>
#include <typeindex>
#include <list>
#include <iosfwd>

#include "Name.h"
#include "Prop.h"
#include "Value.h"


namespace dbrx {


class Bric: public Named {
public:
	class GenericNamedProxy: public Named {
	public:
		virtual std::type_index typeInfo() const = 0;

		virtual const void* const * untypedPPtr() const = 0;

		template<typename T> const T* const * typedPPtr() const {
			if (std::type_index(typeid(T)) != typeInfo()) throw std::runtime_error("Type mismatch");
			return (const T* const *) untypedPPtr();
		}

		GenericNamedProxy(const Name &n): Named(n) {}
	};

	class GenericOutput: public GenericNamedProxy {
	public:
		virtual void* const * untypedPPtr() = 0;

		template<typename T> T* const * typedPPtr() {
			if (std::type_index(typeid(T)) != typeInfo()) throw std::runtime_error("Type mismatch");
			return (T* const *) untypedPPtr();
		}

		using GenericNamedProxy::GenericNamedProxy;
	};

	class GenericInput: public GenericNamedProxy {
	public:
		virtual void connectTo(const GenericNamedProxy &source) = 0;

		using GenericNamedProxy::GenericNamedProxy;
	};

protected:
	std::list<GenericOutput*> m_outputs;
	std::list<GenericInput*> m_inputs;

public:

	template <typename T> class Output: public GenericOutput {
	protected:
		T* m_value = nullptr;

	public:
		operator const T& () const { return *m_value; }
		operator T& () { return *m_value; }
		const T* operator->() const { return m_value; }
		T* operator->() { return m_value; }
		const T& value() const { return *m_value; }
		T& value() { return *m_value; }

		std::type_index typeInfo() const { return typeid(T); }
		const void* const * untypedPPtr() const { return (const void* const *) &m_value; }
		void* const * untypedPPtr() { return (void* const *) &m_value; }

		Output<T>& operator=(const T &v) {
			if (m_value != nullptr) *m_value = v;
			else m_value = new T(v);
			return *this;
		}

		Output<T>& operator=(T &&v) noexcept {
			if (m_value != nullptr) *m_value = std::move(v);
			else operator=(std::unique_ptr<T>( new T(std::move(v)) ));
			return *this;
		}

		Output<T>& operator=(std::unique_ptr<T> &&v) noexcept {
			std::unique_ptr<T> thisV(m_value); m_value = nullptr;
			swap(thisV, v);
			m_value = thisV.release();
			return *this;
		}

		Output(Bric *bric, const Name &n): GenericOutput(n)
			{ bric->m_outputs.push_back(this); }
		Output(const Output &other) = delete;

		~Output() { if (m_value != nullptr) delete m_value; }
	};


	template <typename T> class Input: public GenericInput {
	protected:
		Name m_name;
		const T* const * m_value;

	public:
		operator const T& () const { return **m_value; }
		const T* operator->() const { return *m_value; }
		const T& value() const { return **m_value; }

		void connectTo(const GenericNamedProxy &source)
			{ m_value = source.typedPPtr<T>(); }

		std::type_index typeInfo() const { return typeid(T); }
		const void* const * untypedPPtr() const { return (const void* const *) m_value; }

		Input(Bric *bric, const Name &n): GenericInput(n)
			{ bric->m_inputs.push_back(this); }
		Input(const Input &other) = delete;
	};

	std::ostream & printInfo(std::ostream &os) const;

	virtual void process();

	Bric(const Name &n);
	virtual ~Bric();
};



class InputBric: public Bric {
public:
	InputBric(const Name &n): Bric(n) {}
};



class MapperBric: public Bric {
public:
	MapperBric(const Name &n): Bric(n) {}
};


} // namespace dbrx

/* Add to ...LinkDef.h:


*/

#endif // DBRX_BRIC_H
