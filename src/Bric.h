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


class Bric;



class BricComponent: public virtual HasName, public virtual Configurable {
protected:
	virtual void connectInputToInner(Bric &bric, Name inputName, PropPath::Fragment sourcePath) = 0;

public:
	PropPath absolutePath() const;

	virtual bool hasParent() const = 0;

	virtual const Bric& parent() const = 0;
	virtual Bric& parent() = 0;

	BricComponent& operator=(const BricComponent& v) = delete;
	BricComponent& operator=(BricComponent &&v) = delete;

	friend class Bric;
};


class BricComponentImpl: public virtual BricComponent {
protected:
	Name m_name;
	Bric *m_parent = nullptr;

public:
	Name name() const { return m_name; }
	Name& name() { return m_name; }

	bool hasParent() const { return m_parent != nullptr; }

	const Bric& parent() const { return *m_parent; }
	Bric& parent() { return *m_parent; }

	BricComponentImpl() {}
	BricComponentImpl(Name componentName): m_name(componentName) {}
	BricComponentImpl(Bric *parentBric, Name componentName);
};



class Bric: public virtual BricComponent {
public:
	class Terminal: public virtual BricComponent, public virtual HasValue {
	protected:
		virtual void connectInputToInner(Bric &bric, Name inputName, PropPath::Fragment sourcePath);
	};


	class OutputTerminal: public virtual Terminal, public virtual HasWritableValue {};


	class InputTerminal: public virtual Terminal, public virtual HasConstValueRef {
	public:
		virtual const PropPath& source() = 0;

		virtual void connectTo(const Terminal &other) = 0;

		virtual void connectTo(const Bric &bric, Name terminalName) = 0;

		virtual void connectTo(const Bric &bric) = 0;
	};


	class ParamTerminal : public virtual Terminal, public virtual HasWritableValue {};

protected:
	static const Name s_defaultInputName;
	static const Name s_defaultOutputName;

	std::map<Name, BricComponent*> m_components;
	std::map<Name, Bric*> m_brics;
	std::map<Name, Terminal*> m_terminals;
	std::map<Name, ParamTerminal*> m_params;
	std::map<Name, OutputTerminal*> m_outputs;
	std::map<Name, InputTerminal*> m_inputs;

	void registerComponent(BricComponent* component);
	void registerBric(Bric* bric) { registerComponent(bric); m_brics[bric->name()] = bric; }
	void registerTerminal(Terminal* terminal) { registerComponent(terminal); m_terminals[terminal->name()] = terminal; }
	void registerParam(ParamTerminal* param) { registerTerminal(param); m_params[param->name()] = param; }
	void registerOutput(OutputTerminal* output) { registerTerminal(output); m_outputs[output->name()] = output; }
	void registerInput(InputTerminal* input) { registerTerminal(input); m_inputs[input->name()] = input; }

	std::vector<Bric*> m_deps;
	void addDependency(Bric* dep) { m_deps.push_back(dep); }

	virtual void connectInputToInner(Bric &bric, Name inputName, PropPath::Fragment sourcePath);
	virtual void connectInputToSiblingOrUp(Bric &bric, Name inputName, PropPath::Fragment sourcePath);
	virtual void connectOwnInputTo(Name inputName, const Terminal& terminal);

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
		: public virtual TypedParamTerminal<T>, public BricComponentImpl, public HasTypedPrimaryValueImpl<T>
	{
	public:
		using HasTypedPrimaryValueImpl<T>::value;
		using HasTypedPrimaryValueImpl<T>::typeInfo;

		virtual void applyConfig(const PropVal& config) {
			PropVal currCfg = getConfig();
			if (currCfg.isProps() && config.isProps())
				assign_from(value().get(), currCfg.asProps() + config.asProps());
			else
				assign_from(value().get(), config);
		}

		virtual PropVal getConfig() const { PropVal config; assign_from(config, value().get()); return config; }

		Param<T>& operator=(const Param<T>& v) = delete;

		Param<T>& operator=(const T &v)
			{ TypedParamTerminal<T>::operator=(v); return *this; }

		Param<T>& operator=(T &&v) noexcept
			{ TypedParamTerminal<T>::operator=(std::move(v)); return *this; }

		Param<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ TypedParamTerminal<T>::operator=(std::move(v)); return *this; }


		Param(Bric *parentBric, Name paramName)
			: BricComponentImpl(parentBric, paramName) { parentBric->registerParam(this); }

		Param(const Param &other) = delete;
	};


	virtual void applyConfig(const PropVal& config);
	virtual PropVal getConfig() const;

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

	void connectInputs();

	virtual std::ostream & printInfo(std::ostream &os) const;

	virtual void init() {};

	friend class BricImpl;
};



class BricImpl: public virtual Bric, public BricComponentImpl {
public:
	BricImpl() {}
	BricImpl(Name bricName): BricComponentImpl(bricName) {}
	BricImpl(Bric *parentBric, Name bricName)
		: BricComponentImpl(parentBric, bricName) { parentBric->registerBric(this); }
};



inline BricComponentImpl::BricComponentImpl(Bric *parentBric, Name componentName)
	: BricComponentImpl(componentName) { m_parent = parentBric; }



class BricWithOutputs: public virtual Bric  {
public:
	template <typename T> class Output
		: public virtual TypedOutputTerminal<T>, public BricComponentImpl, public HasTypedPrimaryValueImpl<T>
	{
	public:
		using HasTypedPrimaryValueImpl<T>::value;
		using HasTypedPrimaryValueImpl<T>::typeInfo;

		virtual void applyConfig(const PropVal& config) { if (!config.isNone()) throw std::invalid_argument("Output is not configurable"); }
		virtual PropVal getConfig() const  { return PropVal(); }

		Output<T>& operator=(const Output<T>& v) = delete;

		Output<T>& operator=(const T &v)
			{ TypedOutputTerminal<T>::operator=(v); return *this; }

		Output<T>& operator=(T &&v) noexcept
			{ TypedOutputTerminal<T>::operator=(std::move(v)); return *this; }

		Output<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ TypedOutputTerminal<T>::operator=(std::move(v)); return *this; }


		Output(BricWithOutputs *parentBric, Name outputName)
			: BricComponentImpl(parentBric, outputName) { parentBric->registerOutput(this); }

		Output(BricWithOutputs *parentBric)
			: BricComponentImpl(parentBric, s_defaultOutputName) { parentBric->registerOutput(this); }

		Output(const Output &other) = delete;
	};

	virtual bool nextOutput() = 0;
};



class BricWithInputs: public virtual Bric  {
public:
	template <typename T> class Input
		: public virtual TypedInputTerminal<T>, public BricComponentImpl, public HasTypedConstValueRefImpl<T>
	{
	protected:
		PropPath m_source;

	public:
		using HasTypedConstValueRefImpl<T>::value;
		using HasTypedConstValueRefImpl<T>::typeInfo;

		virtual void applyConfig(const PropVal& config) { m_source = config; }
		virtual PropVal getConfig() const  { return m_source; }

		virtual const PropPath& source() { return m_source; }

		//void connectTo(const TypedTerminal<T> &other) { value().referTo(other.value()); }

		void connectTo(const Terminal &other) { value().referTo(other.value()); }

		void connectTo(const Bric &bric, Name terminalName)
			{ connectTo(bric.getOutput(terminalName, typeInfo())); }

		void connectTo(const Bric &bric)
			{ connectTo(bric.getOutput(s_defaultOutputName, typeInfo())); }

		Input(BricWithInputs *parentBric, Name inputName)
			: BricComponentImpl(parentBric, inputName) { parentBric->registerInput(this); }

		Input(BricWithInputs *parentBric)
			: BricComponentImpl(parentBric, s_defaultInputName) { parentBric->registerInput(this); }

		Input(const Input &other) = delete;
	};

	virtual void nextInput() = 0;
};



class BricWithInOut: public virtual BricWithInputs, public virtual BricWithOutputs {};



class ImportBric: public virtual BricWithOutputs, public BricImpl  {
public:
	using BricImpl::BricImpl;
};



class ExportBric: public virtual BricWithInputs, public BricImpl {
public:
	using BricImpl::BricImpl;
};



class MapperBric: public virtual BricWithInOut, public BricImpl {
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



class ReducerBric: public virtual BricWithInOut, public BricImpl {
public:
	virtual void resetOutput() = 0;
	virtual void nextInput() = 0;

	using BricImpl::BricImpl;
};


} // namespace dbrx


#endif // DBRX_BRIC_H
