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
#include "HasValue.h"


namespace dbrx {


class Bric: public virtual HasName {
public:
	class Terminal
		: public virtual HasName, public virtual HasValue
	{
		Terminal& operator=(const Terminal& v) = delete;
		Terminal& operator=(Terminal &&v) = delete;
	};

	class OutputTerminal : public virtual Terminal, public virtual HasWritableValue {};

	class InputTerminal	: public virtual Terminal, public virtual HasConstValueRef {};


	template <typename T> class TypedTerminal
		: public virtual Terminal, public virtual HasTypedValue<T> {};


	template <typename T> class TypedOutputTerminal
		: public virtual TypedTerminal<T>, public virtual OutputTerminal, public virtual HasTypedWritableValue<T> {

		TypedOutputTerminal<T>& operator=(const T &v)
			{ HasTypedWritableValue<T>::operator=(v); return *this; }

		TypedOutputTerminal<T>& operator=(T &&v) noexcept
			{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }

		TypedOutputTerminal<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }
	};


	template <typename T> class TypedInputTerminal
		: public virtual TypedTerminal<T>, public virtual InputTerminal, public virtual HasTypedConstValueRef<T> {};


	virtual const OutputTerminal& getOutput(const Name &outputName) const;
	virtual OutputTerminal& getOutput(const Name &outputName);

	virtual const InputTerminal& getInput(const Name &outputName) const;
	virtual InputTerminal& getInput(const Name &outputName);

	virtual std::ostream & printInfo(std::ostream &os) const;

	virtual void init() {};

	virtual void process() {};

	virtual ~Bric() {}
};



class BricImpl: public virtual Bric, public HasNameImpl {
public:
	using HasNameImpl::HasNameImpl;
};



class BricWithOutputs: public virtual Bric  {
protected:
	std::map<Name, OutputTerminal*> m_outputs;

	void addOutput(OutputTerminal *output);

	virtual std::ostream & printOutputInfo(std::ostream &os) const;

public:
	template <typename T> class Output
		: public virtual TypedOutputTerminal<T>, public virtual HasNameImpl, public virtual HasTypedPrimaryValueImpl<T>
	{
	public:
		Output<T>& operator=(const Output<T>& v) = delete;

		Output<T>& operator=(const T &v)
			{ HasTypedPrimaryValue<T>::operator=(v); return *this; }

		Output<T>& operator=(T &&v) noexcept
			{ HasTypedPrimaryValue<T>::operator=(std::move(v)); return *this; }

		Output<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ HasTypedPrimaryValue<T>::operator=(std::move(v)); return *this; }


		Output(BricWithOutputs *bric, const Name &n): HasNameImpl(n) { bric->addOutput(this); }
		Output(const Output &other) = delete;
	};

	const OutputTerminal& getOutput(const Name &outputName) const;
	OutputTerminal& getOutput(const Name &outputName);

	virtual std::ostream & printInfo(std::ostream &os) const;
};



class BricWithInputs: public virtual Bric  {
protected:
	std::map<Name, InputTerminal*> m_inputs;

	void addInput(InputTerminal *input);

	virtual std::ostream & printInputInfo(std::ostream &os) const;

public:
	template <typename T> class Input
		: public virtual TypedInputTerminal<T>, public virtual HasNameImpl, public virtual HasTypedConstValueRefImpl<T>
	{
	public:
		void connectTo(const OutputTerminal &output) { this->value().referTo(output.value()); }

		Input(BricWithInputs *bric, const Name &n): HasNameImpl(n) { bric->addInput(this); }
		Input(const Input &other) = delete;
	};

	const InputTerminal& getInput(const Name &inputName) const;
	InputTerminal& getInput(const Name &inputName);

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
