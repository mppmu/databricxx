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
#include <map>
#include <iosfwd>

#include "Name.h"
#include "Prop.h"
#include "Value.h"


namespace dbrx {


class Bric: public virtual Named {
public:
	virtual std::ostream & printInfo(std::ostream &os) const;

	virtual void process() = 0;

	virtual ~Bric() {}
};



class BricImpl: public virtual Bric, public NamedImpl {
public:
	using NamedImpl::NamedImpl;
};



class BricWithOutputs: public virtual Bric  {
protected:
	std::map<Name, UniqueValue*> m_outputs;

	virtual std::ostream & printOutputInfo(std::ostream &os) const;

public:
	template <typename T> class OutputValue: public TypedUniqueValue<T> {
	public:
		OutputValue<T>& operator=(const OutputValue<T>& v) = delete;

		OutputValue<T>& operator=(const T &v)
			{ TypedUniqueValue<T>::operator=(v); return *this; }

		OutputValue<T>& operator=(T &&v) noexcept
			{ TypedUniqueValue<T>::operator=(std::move(v)); return *this; }

		OutputValue<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ TypedUniqueValue<T>::operator=(std::move(v)); return *this; }


		OutputValue(BricWithOutputs *bric, const Name &n) { bric->m_outputs[n] = this; }
		OutputValue(const OutputValue &other) = delete;
	};

	virtual std::ostream & printInfo(std::ostream &os) const;
};



class BricWithInputs: public virtual Bric  {
protected:
	std::map<Name, ConstValueRef*> m_inputs;

	virtual std::ostream & printInputInfo(std::ostream &os) const;

public:
	template <typename T> class InputValue: public TypedConstValueRef<T> {
	public:
		InputValue(BricWithInputs *bric, const Name &n) { bric->m_inputs[n] = this; }
		InputValue(const InputValue &other) = delete;
	};

	virtual std::ostream & printInfo(std::ostream &os) const;
};



class BricWithInOut: public virtual BricWithInputs, public virtual BricWithOutputs  {
public:
	virtual std::ostream & printInfo(std::ostream &os) const;
};



class InputBric: public BricWithOutputs, public BricImpl  {
public:
	using BricImpl::BricImpl;
};



class OutputBric: public BricWithInputs, public BricImpl {
public:
	using BricImpl::BricImpl;
};



class FilterBric: public BricWithInOut, public BricImpl {
public:
	using BricImpl::BricImpl;
};



class MapperBric: public BricWithInOut, public BricImpl {
public:
	using BricImpl::BricImpl;
};



class ReducerBric: public BricWithInOut, public BricImpl {
public:
	using BricImpl::BricImpl;
};


} // namespace dbrx

/* Add to ...LinkDef.h:


*/

#endif // DBRX_BRIC_H
