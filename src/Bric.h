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
#include "HasValue.h"


namespace dbrx {


class Bric: public virtual HasName {
public:
	class Terminal
		: public virtual HasName, public virtual HasValue
	{
	public:
		Terminal& operator=(const Terminal& v) = delete;
		Terminal& operator=(Terminal &&v) = delete;
	};


	class OutputTerminal : public virtual Terminal, public virtual HasWritableValue {};


	class InputTerminal	: public virtual Terminal, public virtual HasConstValueRef {
	public:
		virtual void connectTo(const Terminal &other) = 0;

		virtual void connectTo(const Bric &bric, Name terminalName) = 0;

		virtual void connectTo(const Bric &bric) = 0;
	};


	class ParamTerminal : public virtual Terminal, public virtual HasWritableValue {};

protected:
	static const Name s_defaultInputName;
	static const Name s_defaultOutputName;

	Bric *m_parent = nullptr;

	std::map<Name, Terminal*> m_terminals;

	std::map<Name, OutputTerminal*> m_outputs;
	std::map<Name, InputTerminal*> m_inputs;
	std::map<Name, ParamTerminal*> m_params;

	void addTerminal(Terminal* terminal);

	void addParam(ParamTerminal* param);

public:
	template <typename T> class TypedTerminal
		: public virtual Terminal, public virtual HasTypedValue<T> {};


	template <typename T> class TypedOutputTerminal
		: public virtual TypedTerminal<T>, public virtual OutputTerminal, public virtual HasTypedWritableValue<T>
	{
	public:
		TypedOutputTerminal<T>& operator=(const T &v)
			{ HasTypedWritableValue<T>::operator=(v); return *this; }

		TypedOutputTerminal<T>& operator=(T &&v) noexcept
			{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }

		TypedOutputTerminal<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }
	};


	template <typename T> class TypedInputTerminal
		: public virtual TypedTerminal<T>, public virtual InputTerminal, public virtual HasTypedConstValueRef<T> {};


	template <typename T> class TypedParamTerminal
		: public virtual TypedTerminal<T>, public virtual ParamTerminal, public virtual HasTypedWritableValue<T>
	{
	public:
		TypedParamTerminal<T>& operator=(const T &v)
			{ HasTypedWritableValue<T>::operator=(v); return *this; }

		TypedParamTerminal<T>& operator=(T &&v) noexcept
			{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }

		TypedParamTerminal<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ HasTypedWritableValue<T>::operator=(std::move(v)); return *this; }
	};


	template <typename T> class Param
		: public virtual TypedParamTerminal<T>, public virtual HasNameImpl, public virtual HasTypedPrimaryValueImpl<T>
	{
	public:
		Param<T>& operator=(const Param<T>& v) = delete;

		Param<T>& operator=(const T &v)
			{ TypedParamTerminal<T>::operator=(v); return *this; }

		Param<T>& operator=(T &&v) noexcept
			{ TypedParamTerminal<T>::operator=(std::move(v)); return *this; }

		Param<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ TypedParamTerminal<T>::operator=(std::move(v)); return *this; }


		Param(Bric *bric, Name n): HasNameImpl(n) { bric->addParam(this); }
		Param(const Param &other) = delete;
	};


	bool hasParent() const { return m_parent != nullptr; }

	const Bric& parent() const { return *m_parent; }
	Bric& parent() { return *m_parent; }

	void parent(Bric *parentBric) { m_parent = parentBric; }


	virtual const Terminal& getTerminal(Name terminalName) const;
	virtual Terminal& getTerminal(Name terminalName);

	virtual const Terminal& getTerminal(Name terminalName, const std::type_info& typeInfo) const;
	virtual Terminal& getTerminal(Name terminalName, const std::type_info& typeInfo);


	virtual const OutputTerminal& getOutput(Name outputName) const;
	virtual OutputTerminal& getOutput(Name outputName);

	virtual const OutputTerminal& getOutput(Name outputName, const std::type_info& typeInfo) const;
	virtual OutputTerminal& getOutput(Name outputName, const std::type_info& typeInfo);


	virtual const InputTerminal& getInput(Name outputName) const;
	virtual InputTerminal& getInput(Name outputName);

	virtual const InputTerminal& getInput(Name outputName, const std::type_info& typeInfo) const;
	virtual InputTerminal& getInput(Name outputName, const std::type_info& typeInfo);


	virtual const ParamTerminal& getParam(Name outputName) const;
	virtual ParamTerminal& getParam(Name outputName);

	virtual const ParamTerminal& getParam(Name outputName, const std::type_info& typeInfo) const;
	virtual ParamTerminal& getParam(Name outputName, const std::type_info& typeInfo);


	virtual std::ostream & printInfo(std::ostream &os) const;

	virtual void init() {};

	virtual ~Bric() {}
};



class BricImpl: public virtual Bric, public HasNameImpl {
public:
	BricImpl() {}

	BricImpl(Name n) : HasNameImpl(n) {}

	BricImpl(Bric *parentBric, Name n)
		: BricImpl(n) { m_parent = parentBric; }
};



class BricWithOutputs: public virtual Bric  {
protected:
	void addOutput(OutputTerminal* output);

public:
	template <typename T> class Output
		: public virtual TypedOutputTerminal<T>, public virtual HasNameImpl, public virtual HasTypedPrimaryValueImpl<T>
	{
	public:
		Output<T>& operator=(const Output<T>& v) = delete;

		Output<T>& operator=(const T &v)
			{ TypedOutputTerminal<T>::operator=(v); return *this; }

		Output<T>& operator=(T &&v) noexcept
			{ TypedOutputTerminal<T>::operator=(std::move(v)); return *this; }

		Output<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ TypedOutputTerminal<T>::operator=(std::move(v)); return *this; }


		Output(BricWithOutputs *bric, Name n): HasNameImpl(n) { bric->addOutput(this); }
		Output(BricWithOutputs *bric): HasNameImpl(s_defaultOutputName) { bric->addOutput(this); }
		Output(const Output &other) = delete;
	};

	virtual bool nextOutput() = 0;
};



class BricWithInputs: public virtual Bric  {
protected:
	void addInput(InputTerminal* input);

public:
	template <typename T> class Input
		: public virtual TypedInputTerminal<T>, public virtual HasNameImpl, public virtual HasTypedConstValueRefImpl<T>
	{
	public:
		//void connectTo(const TypedTerminal<T> &other) { this->value().referTo(other.value()); }

		void connectTo(const Terminal &other) { this->value().referTo(other.value()); }

		void connectTo(const Bric &bric, Name terminalName)
			{ connectTo(bric.getOutput(terminalName, this->typeInfo())); }

		void connectTo(const Bric &bric)
			{ connectTo(bric.getOutput(s_defaultOutputName, this->typeInfo())); }

		Input(BricWithInputs *bric, Name n): HasNameImpl(n) { bric->addInput(this); }
		Input(BricWithInputs *bric): HasNameImpl(s_defaultInputName) { bric->addInput(this); }
		Input(const Input &other) = delete;
	};

	virtual void nextInput() = 0;
};



class BricWithInOut: public virtual BricWithInputs, public virtual BricWithOutputs {};



class ImportBric: public virtual BricWithOutputs, public virtual BricImpl  {
public:
	using BricImpl::BricImpl;
};



class ExportBric: public virtual BricWithInputs, public virtual BricImpl {
public:
	using BricImpl::BricImpl;
};



class MapperBric: public virtual BricWithInOut, public virtual BricImpl {
public:
	using BricImpl::BricImpl;
};



class TransformBric: public MapperBric {
private:
	bool m_processed = false;

public:
	void nextInput()
		{ m_processed = false; }

	bool nextOutput() {
		if (! m_processed) {
			process();
			m_processed = true;
			return true;
		} else return false;
	}

	virtual void process() = 0;

	using MapperBric::MapperBric;
};



class ReducerBric: public virtual BricWithInOut, public virtual BricImpl {
public:
	virtual void resetOutput() = 0;
	virtual void nextInput() = 0;

	using BricImpl::BricImpl;
};


} // namespace dbrx

/* Add to ...LinkDef.h:


*/

#endif // DBRX_BRIC_H