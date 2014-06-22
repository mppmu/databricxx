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


#include "Bric.h"

#include <iostream>
#include <algorithm>

#include "TypeReflection.h"

#include "format.h"


using namespace std;


namespace dbrx {



PropPath BricComponent::absolutePath() const {
	return hasParent() ? parent().absolutePath() % name() : name();
}



void Bric::Terminal::connectInputToInner(Bric &bric, Name inputName, PropPath::Fragment sourcePath) {
	if (!sourcePath.empty()) throw runtime_error("Couldn't resolve source path %s during input lookup, terminal \"%s\" has no inner components"_format(sourcePath, absolutePath()));
	else bric.connectOwnInputTo(inputName, *this);
}



const Name Bric::s_defaultInputName("input");
const Name Bric::s_defaultOutputName("output");
const Name Bric::s_bricTypeKey("type");


void Bric::registerComponent(BricComponent* component) {
	if (component->name() == s_bricTypeKey) throw invalid_argument("Can't add component with reserved name \"%s\" to bric \"%s\""_format(component->name(), absolutePath()));
	if (component->name().empty()) throw invalid_argument("Can't register BricComponent with empty name in bric \"%s\""_format(absolutePath()));

	auto r = m_components.find(component->name());
	if (r != m_components.end()) throw invalid_argument("Can't add duplicate component with name \"%s\" to bric \"%s\""_format(component->name(), absolutePath()));
	m_components[component->name()] = component;
	component->setParent(this);
}


void Bric::registerBric(Bric* bric) {
	dbrx_log_trace("Registering inner bric \"%s\" in bric \"%s\""_format(bric->name(), absolutePath()));
	registerComponent(bric);
	m_brics[bric->name()] = bric;
}

void Bric::registerTerminal(Terminal* terminal)
	{ registerComponent(terminal); m_terminals[terminal->name()] = terminal; }

void Bric::registerParam(ParamTerminal* param) {
	dbrx_log_trace("Registering param terminal \"%s\" of type \"%s\" in bric \"%s\""_format(
		param->name(), param->value().typeInfo().name(), absolutePath()));
	registerTerminal(param);
	m_params[param->name()] = param;
}

void Bric::registerOutput(OutputTerminal* output) {
	dbrx_log_trace("Registering output terminal \"%s\" of type \"%s\" in bric \"%s\""_format(
		output->name(), output->value().typeInfo().name(), absolutePath()));
	if (!canHaveOutputs()) throw invalid_argument("Bric \"%s\" cannot have outputs"_format(absolutePath()));
	registerTerminal(output);
	m_outputs[output->name()] = output;
}

void Bric::registerInput(InputTerminal* input) {
	dbrx_log_trace("Registering input terminal \"%s\" of type \"%s\" in bric \"%s\""_format(
		input->name(), input->value().typeInfo().name(), absolutePath()));
	if (!canHaveInputs()) throw invalid_argument("Bric \"%s\" cannot have inputs"_format(absolutePath()));
	registerTerminal(input);
	m_inputs[input->name()] = input;
}


bool Bric::isBricConfig(const PropVal& config) {
	if (!config.isProps()) return false;
	Props props = config.asProps();
	const auto& bricType = props.find(s_bricTypeKey);
	if (bricType == props.end()) return false;
	return bricType->second.isString() ? true : false;
}


void Bric::addDynBric(Name bricName, const PropVal& config) {
	if (!isBricConfig(config)) throw invalid_argument("Invalid configuration format for dynamic sub-bric \"%s\" in bric \"%s\""_format(bricName, absolutePath()));
	dbrx_log_debug("Creating dynamic bric \"%s\" inside bric \"%s\""_format(bricName, absolutePath()));
	Props subBricProps = config.asProps();
	const std::string className = config.at(s_bricTypeKey).asString();

	// For some reason, objects created via "newInstance<Bric>" are unstable
	// and produce segfaults. May be some problem with virtual tables and may
	// be related to Bric virtual inheritance hierarchy. As a workaround,
	// use the bottom types of the Bric hierarchy directly:

	unique_ptr<Bric> dynBric;
	TypeReflection dynBricTR(className);
	if (TypeReflection(typeid(ImportBric)).isAssignableFrom(dynBricTR)) {
		dynBric = dynBricTR.newInstance<ImportBric>();
	} else if (TypeReflection(typeid(TransformBric)).isAssignableFrom(dynBricTR)) {
		dynBric = dynBricTR.newInstance<TransformBric>();
	} else if (TypeReflection(typeid(MapperBric)).isAssignableFrom(dynBricTR)) {
		dynBric = dynBricTR.newInstance<MapperBric>();
	} else if (TypeReflection(typeid(ReducerBric)).isAssignableFrom(dynBricTR)) {
		dynBric = dynBricTR.newInstance<ReducerBric>();
	} else if (TypeReflection(typeid(AsyncReducerBric)).isAssignableFrom(dynBricTR)) {
		dynBric = dynBricTR.newInstance<AsyncReducerBric>();
	} else {
		throw runtime_error("Dynamic generation of bric of class \"%s\" not supported, does not derive from any standard bric type"_format(className.c_str()));
	}

	dynBric->name() = bricName;
	Bric* dynBricPtr = dynBric.get();
	registerBric(dynBricPtr);
	m_dynBrics[dynBric->name()] = std::move(dynBric);
	dynBricPtr->applyConfig(config);
}


void Bric::delDynBric(Name bricName) {
	m_dynBrics.erase(m_dynBrics.find(bricName));
	m_components.erase(m_components.find(bricName));
}


void Bric::connectInputToInner(Bric &bric, Name inputName, PropPath::Fragment sourcePath) {
	if (sourcePath.empty()) {
		auto found = m_components.find(s_defaultOutputName);
		if (found != m_components.end()) found->second->connectInputToInner(bric, inputName, sourcePath);
	} else {
		auto found = m_components.find(sourcePath.front().asName());
		if (found != m_components.end()) found->second->connectInputToInner(bric, inputName, sourcePath.tail());
		else throw runtime_error("Couldn't resolve source path \"%s\" during input lookup, no such component in bric \"%s\""_format(sourcePath, absolutePath()));
	}
}


void Bric::connectInputToSiblingOrUp(Bric &bric, Name inputName, PropPath::Fragment sourcePath) {
	if (sourcePath.empty()) throw runtime_error("Empty source path during input lookup in bric \"%s\""_format(absolutePath()));
	Name siblingName = sourcePath.front().asName();
	if (siblingName == name()) connectInputToInner(bric, inputName, sourcePath.tail());
	else {
		if (hasParent()) {
			auto found = parent().m_brics.find(siblingName);
			if (found != parent().m_brics.end()) {
				Bric* sibling = found->second;
				sibling->connectInputToInner(bric, inputName, sourcePath.tail());
				addSource(sibling);
			}
		} else throw runtime_error("Reached top-level Bric \"%s\" during input lookup"_format(absolutePath()));
	}
}


void Bric::connectOwnInputTo(Name inputName, const Terminal& terminal) {
	auto found = m_inputs.find(inputName);
	if (found != m_inputs.end()) found->second->connectTo(terminal);
	else throw invalid_argument("Can't connect non-existing input \"%s\" to terminal \"%s\""_format(inputName, terminal.absolutePath()));
}


void Bric::applyConfig(const PropVal& config) {
	dbrx_log_debug("Applying config to bric \"%s\""_format(absolutePath()));
	for (const auto& entry: config.asProps()) {
		Name componentName = entry.first.asName();
		const PropVal& componentConfig = entry.second;
		if (componentName != s_bricTypeKey) {
			const auto& foundDynBric = m_dynBrics.find(componentName);
			if (foundDynBric != m_dynBrics.end()) {
				if (componentConfig.isNone()) delDynBric(componentName);
				else {
					try {
						foundDynBric->second->applyConfig(componentConfig);
					} catch (const NotReconfigurable&) {
						delDynBric(componentName);
						addDynBric(componentName, componentConfig);						
					}
				}
			} else {
				const auto& foundComponent = m_components.find(componentName);
				if (foundComponent != m_components.end()) {
					foundComponent->second->applyConfig(componentConfig);
				} else if (isBricConfig(componentConfig)) {
					addDynBric(componentName, componentConfig);
				} else throw runtime_error("Invalid configuration, bric \"%s\" doesn't have a component named \"%s\""_format(absolutePath(), componentName));
			}
		}
	}
}


PropVal Bric::getConfig() const  {
	Props props;
	for (const auto& entry: m_components) {
		PropVal componentConfig = entry.second->getConfig();
		if (! componentConfig.isNone())	props[entry.second->name()] = std::move(componentConfig);
	}
	props[s_bricTypeKey] = TypeReflection(typeid(this)).name();
	return PropVal(std::move(props));
}


const Bric::Terminal& Bric::getTerminal(Name terminalName) const {
	auto r = m_terminals.find(terminalName);
	if (r == m_terminals.end()) throw invalid_argument("No terminal \"%s\" found in bric \"%s\""_format(terminalName, absolutePath()));
	else return *r->second;
}

Bric::Terminal& Bric::getTerminal(Name terminalName) {
	auto r = m_terminals.find(terminalName);
	if (r == m_terminals.end()) throw invalid_argument("No terminal \"%s\" found in bric \"%s\""_format(terminalName, absolutePath()));
	else return *r->second;
}


const Bric::Terminal& Bric::getTerminal(Name terminalName, const std::type_info& typeInfo) const {
	const Bric::Terminal& terminal = getTerminal(terminalName);
	if (terminal.value().typeInfo() != typeInfo)
		throw runtime_error("Type of terminal \"%s\" doesn't match requested type \"%s\""_format(terminal.absolutePath(), terminal.typeInfo().name()));
	return terminal;
}

Bric::Terminal& Bric::getTerminal(Name terminalName, const std::type_info& typeInfo) {
	Bric::Terminal& terminal = getTerminal(terminalName);
	if (terminal.value().typeInfo() != typeInfo)
		throw runtime_error("Type of terminal \"%s\" doesn't match requested type \"%s\""_format(terminal.absolutePath(), terminal.typeInfo().name()));
	return terminal;
}


const Bric::OutputTerminal& Bric::getOutput(Name outputName) const
	{ return dynamic_cast<const Bric::OutputTerminal&>(getTerminal(outputName)); }

Bric::OutputTerminal& Bric::getOutput(Name outputName)
	{ return dynamic_cast<Bric::OutputTerminal&>(getTerminal(outputName)); }

const Bric::OutputTerminal& Bric::getOutput(Name outputName, const std::type_info& typeInfo) const
	{ return dynamic_cast<const Bric::OutputTerminal&>(getTerminal(outputName, typeInfo)); }

Bric::OutputTerminal& Bric::getOutput(Name outputName, const std::type_info& typeInfo)
	{ return dynamic_cast<Bric::OutputTerminal&>(getTerminal(outputName, typeInfo)); }


const Bric::InputTerminal& Bric::getInput(Name inputName) const
	{ return dynamic_cast<const Bric::InputTerminal&>(getTerminal(inputName)); }

Bric::InputTerminal& Bric::getInput(Name inputName)
	{ return dynamic_cast<Bric::InputTerminal&>(getTerminal(inputName)); }

const Bric::InputTerminal& Bric::getInput(Name inputName, const std::type_info& typeInfo) const
	{ return dynamic_cast<const Bric::InputTerminal&>(getTerminal(inputName, typeInfo)); }

Bric::InputTerminal& Bric::getInput(Name inputName, const std::type_info& typeInfo)
	{ return dynamic_cast<Bric::InputTerminal&>(getTerminal(inputName, typeInfo)); }


const Bric::ParamTerminal& Bric::getParam(Name paramName) const
	{ return dynamic_cast<const Bric::ParamTerminal&>(getTerminal(paramName)); }

Bric::ParamTerminal& Bric::getParam(Name paramName)
	{ return dynamic_cast<Bric::ParamTerminal&>(getTerminal(paramName)); }

const Bric::ParamTerminal& Bric::getParam(Name paramName, const std::type_info& typeInfo) const
	{ return dynamic_cast<const Bric::ParamTerminal&>(getTerminal(paramName, typeInfo)); }

Bric::ParamTerminal& Bric::getParam(Name paramName, const std::type_info& typeInfo)
	{ return dynamic_cast<Bric::ParamTerminal&>(getTerminal(paramName, typeInfo)); }


void Bric::connectInputs() {
	m_sources.clear();
	for (const auto& input: m_inputs)
		connectInputToSiblingOrUp(*this, input.second->name(), input.second->source());
	for (const auto& brics: m_brics)
		brics.second->connectInputs();
}


std::ostream & Bric::printInfo(std::ostream &os) const {
	os << "Bric " << name() << ":" << endl;

	if (! m_inputs.empty()) {
		os << "  Inputs: ";
		for (auto const &x: m_inputs) os << " " << x.second->name() << "(" << x.second->value().typeInfo().name() << ")";
		os << endl;
	}

	if (! m_outputs.empty()) {
		os << "  Outputs: ";
		for (auto const &x: m_outputs) os << " " << x.second->name() << "(" << x.second->value().typeInfo().name() << ")";
		os << endl;
	}

	if (! m_params.empty()) {
		os << "  Params: ";
		for (auto const &x: m_params) os << " " << x.second->name() << "(" << x.second->value().typeInfo().name() << ")";
		os << endl;
	}

	return os;
}


} // namespace dbrx
