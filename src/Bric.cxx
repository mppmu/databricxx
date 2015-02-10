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
#include <sstream>

#include "TypeReflection.h"

#include "format.h"


using namespace std;


namespace dbrx {



PropPath BricComponent::absolutePath() const {
	return hasParent() ? parent().absolutePath() % name() : name();
}



bool BricComponent::BCReference::isReference(const PropVal& propVal) {
	if (! propVal.isString()) {
		return false;
	} else {
		const string &s = propVal.asString();
		if (!s.empty() && (s.front() == '&')) return true;
		else return false;
	}
}


BricComponent::BCReference& BricComponent::BCReference::operator=(const std::string& ref) {
	if (ref.empty() || (ref.front() != '&')) throw std::invalid_argument("Can't initialize BricComponent::BCReference from string \"%s\""_format(ref));
	auto pathBegin = find_if(ref.begin() + 1, ref.end(), [](char c){ return !iswspace(c); } );

	return *this = PropPath(ref.substr(pathBegin - ref.begin(), ref.npos));
}


BricComponent::BCReference& BricComponent::BCReference::operator=(const PropVal& ref) {
	if (! ref.isString()) throw std::invalid_argument("Can't initialize BricComponent::BCReference from non string-valued PropVal %s"_format(ref));
	else return *this = ref.asString();
}


std::ostream& BricComponent::BCReference::print(std::ostream &os) const {
	return path().print(os << "&");
}



void Bric::InputTerminal::connectTo(Bric::Terminal &other) {
	dbrx_log_trace("Connecting input terminal \"%s\" to terminal \"%s\"", absolutePath(), other.absolutePath());
	value().referTo(other.value());
	setSrcTerminal(&other);
	setEffSrcBric( parent().addSource(&other.parent()) );
}



const PropKey Bric::s_defaultInputName("input");
const PropKey Bric::s_defaultOutputName("output");
const PropKey Bric::s_bricTypeKey("type");


std::unique_ptr<Bric> Bric::createBricFromTypeName(const std::string &className) {
	// For some reason, objects created via "newInstance<Bric>" are unstable
	// and produce segfaults. May be some problem with virtual tables and may
	// be related to Bric virtual inheritance hierarchy. As a workaround,
	// use the bottom types of the Bric hierarchy directly:

	unique_ptr<Bric> bric;
	TypeReflection bricTR(className);
	if (TypeReflection(typeid(ImportBric)).isPtrAssignableFrom(bricTR)) {
		bric = bricTR.newInstance<ImportBric>();
	} else if (TypeReflection(typeid(TransformBric)).isPtrAssignableFrom(bricTR)) {
		bric = bricTR.newInstance<TransformBric>();
	} else if (TypeReflection(typeid(MapperBric)).isPtrAssignableFrom(bricTR)) {
		bric = bricTR.newInstance<MapperBric>();
	} else if (TypeReflection(typeid(ReducerBric)).isPtrAssignableFrom(bricTR)) {
		bric = bricTR.newInstance<ReducerBric>();
	} else if (TypeReflection(typeid(AsyncReducerBric)).isPtrAssignableFrom(bricTR)) {
		bric = bricTR.newInstance<AsyncReducerBric>();
	} else {
		throw runtime_error("Dynamic generation of bric of class \"%s\" not supported, does not derive from any standard bric type"_format(className));
	}
	
	return bric;
}


bool Bric::isBricConfig(const PropVal& config) {
	if (!config.isProps()) return false;
	Props props = config.asProps();
	const auto& bricType = props.find(s_bricTypeKey);
	if (bricType == props.end()) return false;
	return bricType->second.isString() ? true : false;
}


void Bric::registerComponent(BricComponent* component) {
	if (component->name() == s_bricTypeKey) throw invalid_argument("Can't add component with reserved name \"%s\" to bric \"%s\""_format(component->name(), absolutePath()));
	if (component->name() == PropKey()) throw invalid_argument("Can't register BricComponent with empty name in bric \"%s\""_format(absolutePath()));

	auto r = m_components.find(component->name());
	if (r != m_components.end()) throw invalid_argument("Can't add duplicate component with name \"%s\" to bric \"%s\""_format(component->name(), absolutePath()));

	if (dynamic_cast<Bric*>(component) != nullptr) {
		Bric* bric = dynamic_cast<Bric*>(component);
		dbrx_log_trace("Registering inner bric \"%s\" in bric \"%s\""_format(bric->name(), absolutePath()));
		m_brics[bric->name()] = bric;
	} else if (dynamic_cast<Terminal*>(component) != nullptr) {
		if (dynamic_cast<ParamTerminal*>(component) != nullptr) {
			ParamTerminal* param = dynamic_cast<ParamTerminal*>(component);
			dbrx_log_trace("Registering param terminal \"%s\" of type \"%s\" in bric \"%s\""_format(
				param->name(), param->value().typeInfo().name(), absolutePath()));
			m_params[param->name()] = param;
		} else if (dynamic_cast<OutputTerminal*>(component) != nullptr) {
			OutputTerminal* output = dynamic_cast<OutputTerminal*>(component);
			dbrx_log_trace("Registering output terminal \"%s\" of type \"%s\" in bric \"%s\""_format(
				output->name(), output->value().typeInfo().name(), absolutePath()));
			if (!canHaveOutputs()) throw invalid_argument("Bric \"%s\" cannot have outputs"_format(absolutePath()));
			m_outputs[output->name()] = output;
		} else if (dynamic_cast<InputTerminal*>(component) != nullptr) {
			InputTerminal* input = dynamic_cast<InputTerminal*>(component);
			dbrx_log_trace("Registering input terminal \"%s\" of type \"%s\" in bric \"%s\""_format(
				input->name(), input->value().typeInfo().name(), absolutePath()));
			if (!canHaveInputs()) throw invalid_argument("Bric \"%s\" cannot have inputs"_format(absolutePath()));
			m_inputs[input->name()] = input;
		} else {
			assert(false);
			throw logic_error("Unknown terminal type, can't register in bic");
		}

		Terminal* terminal = dynamic_cast<Terminal*>(component);
		m_terminals[terminal->name()] = terminal;
	} else {
		assert(false);
		throw logic_error("Unknown component type, can't register in bic");
	}

	m_components[component->name()] = component;
}


void Bric::unregisterComponent(BricComponent* component) {
	dbrx_log_trace("Unregistering component \"%s\" from bric \"%s\""_format(component->name(), absolutePath()));
	if (dynamic_cast<Bric*>(component) != nullptr) {
		m_brics.erase(component->name());
	} else if (dynamic_cast<Terminal*>(component) != nullptr) {
		if (dynamic_cast<ParamTerminal*>(component) != nullptr) {
			m_params.erase(component->name());
		} else if (dynamic_cast<OutputTerminal*>(component) != nullptr) {
			m_outputs.erase(component->name());
		} else if (dynamic_cast<InputTerminal*>(component) != nullptr) {
			m_inputs.erase(component->name());
		} else { assert(false);	}
		m_terminals.erase(component->name());
	} else { assert(false); }

	m_components.erase(component->name());
}


Bric* Bric::addDynBric(std::unique_ptr<Bric> dynBric) {
	Bric* dynBricPtr = dynBric.get();
	dynBricPtr->setParent(this);
	m_dynBrics[dynBricPtr->name()] = std::move(dynBric);
	return dynBricPtr;
}


Bric* Bric::addDynBric(PropKey bricName, const PropVal& config) {
	if (!isBricConfig(config)) throw invalid_argument("Invalid configuration format for dynamic sub-bric \"%s\" in bric \"%s\""_format(bricName, absolutePath()));
	dbrx_log_debug("Creating dynamic bric \"%s\" inside bric \"%s\""_format(bricName, absolutePath()));
	Props subBricProps = config.asProps();
	std::string className = config.at(s_bricTypeKey).asString();
	unique_ptr<Bric> dynBric = createBricFromTypeName(className);
	dynBric->setName(bricName);
	Bric* dynBricPtr = dynBric.get();
	dynBricPtr->setParent(this);
	m_dynBrics[dynBricPtr->name()] = std::move(dynBric);
	m_dynBricClassNames[dynBricPtr->name()] = className;
	dynBricPtr->applyConfig(config);
	return dynBricPtr;
}


void Bric::delDynBric(PropKey bricName) {
	m_dynBrics.erase(m_dynBrics.find(bricName));
	m_components.erase(m_components.find(bricName));
}


Bric::InputTerminal* Bric::connectInputToInner(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath) {
	PropPath dfltSrc(s_defaultOutputName);
	if (sourcePath.empty()) sourcePath = dfltSrc;
	auto sourceName = sourcePath.front();
	auto found = m_components.find(sourceName);
	if (found != m_components.end()) {
		BricComponent* foundComp = found->second;
		if (dynamic_cast<Bric*>(foundComp)) {
			return dynamic_cast<Bric*>(foundComp)-> connectInputToInner(bric, inputName, sourcePath.tail());
		} else if (dynamic_cast<Terminal*>(foundComp)) {
			return bric.connectOwnInputTo(inputName, *dynamic_cast<Terminal*>(foundComp));
		} else {
			assert(false);
			throw logic_error("Bric component is neither a Bric nor a Terminal");
		}
	} else if (canHaveDynOutputs()) {
		auto foundInput = bric.m_terminals.find(inputName);
		if (foundInput != m_terminals.end()) {
			Terminal *input = foundInput->second;
			dbrx_log_trace("Creating dynamic output terminal \"%s\" for input \"%s\" in bric \"%s\"", sourceName, input->name(), absolutePath());
			OutputTerminal *source = input->createMatchingDynOutput(this, sourceName);
			return bric.connectOwnInputTo(inputName, *source);
		} else throw invalid_argument("No input named \"%s\" found in bric \"%s\""_format(inputName, bric.absolutePath()));
	}
	else throw runtime_error("Couldn't resolve source path \"%s\" for input \"%s\" of bric \"%s\", no such component in bric \"%s\""_format(sourcePath, inputName, bric.absolutePath(), absolutePath()));
}


Bric::InputTerminal* Bric::connectInputToSiblingOrUp(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath) {
	if (sourcePath.empty()) throw runtime_error("Empty source path while looking up source \"%s\" for input \"%s\" of bric \"%s\" inside bric \"%s\""_format(sourcePath, inputName, bric.absolutePath(), absolutePath()));
	PropKey siblingName = sourcePath.front();
	if (siblingName == name()) return connectInputToInner(bric, inputName, sourcePath.tail());
	else {
		if (hasParent()) {
			auto found = parent().m_brics.find(siblingName);
			if (found != parent().m_brics.end()) {
				Bric* sibling = found->second;
				InputTerminal *input = sibling->connectInputToInner(bric, inputName, sourcePath.tail());
				dbrx_log_trace("Detected dependency of bric \"%s\" on bric \"%s\"", absolutePath(), sibling->absolutePath());
				return input;
			} else {
				InputTerminal *input = parent().connectInputToSiblingOrUp(bric, inputName, sourcePath);
				m_hasExternalSources = true;
				return input;
			}
		} else throw runtime_error("Reached top-level bric \"%s\" during looking up source for input \"%s\" in bric \"%s\""_format(absolutePath(), inputName, bric.absolutePath()));
	}
}


Bric::InputTerminal* Bric::connectOwnInputTo(PropKey inputName, Terminal& source) {
	auto found = m_inputs.find(inputName);
	if (found != m_inputs.end()) {
		InputTerminal* input = found->second;
		input->connectTo(source);
		return input;
	} else if (canHaveDynInputs()) {
		dbrx_log_trace("Creating dynamic input terminal \"%s\" for source \"%s\" in bric \"%s\"", inputName, source.absolutePath(), absolutePath());
		InputTerminal *input = source.createMatchingDynInput(this, inputName);
		input->connectTo(source);
		return input;
	} else throw invalid_argument("Can't connect non-existing input \"%s\" to terminal \"%s\""_format(inputName, source.absolutePath()));
}


void Bric::disconnectInputs() {
	dbrx_log_trace("Disconnecting inputs of bric \"%s\" and all inner brics", absolutePath());

	for (const auto& brics: m_brics)
		brics.second->disconnectInputs();

	m_sources.clear();
	m_hasExternalSources = false;
	m_inputsConnected = false;

	m_dests.clear();

	m_dynTerminals.clear();
}


void Bric::connectInputs() {
	dbrx_log_trace("Connecting inputs of bric \"%s\" and all inner brics", absolutePath());
	if (m_inputsConnected) throw logic_error("Can't connect already connected inputs in bric \"%s\""_format(absolutePath()));

	for (const auto& input: m_inputs)
		if (! input.second->hasFixedValue())
			connectInputToSiblingOrUp(*this, input.second->name(), input.second->source());

	for (const auto& brics: m_brics) brics.second->connectInputs();
	for (const auto& brics: m_brics) brics.second->updateDeps();
}


void Bric::updateDeps() {
	{
		auto &v = m_sources;
		std::sort(v.begin(), v.end());
		auto last = std::unique(v.begin(), v.end());
		v.erase(last, v.end());
	}
	{
		auto &v = m_dests;
		std::sort(v.begin(), v.end());
		auto last = std::unique(v.begin(), v.end());
		v.erase(last, v.end());
	}
}


void Bric::initRecursive() {
	dbrx_log_debug("Recursively initialize bric \"%s\" (%s srcs, %s dests) and all inner brics"_format(absolutePath(), nSources(), nDests()));

	initTDirectory();
	TempChangeOfTDirectory tDirChange(localTDirectory());

	for (auto &entry: m_brics) entry.second->initRecursive();
	dbrx_log_debug("Run init for bric \"%s\", sources [%s], dests [%s]"_format(
		absolutePath(),
		mkstring(mapped(m_sources, [&](Bric* bric){ return bric->name(); }), ", "),
		mkstring(mapped(m_dests, [&](Bric* bric){ return bric->name(); }), ", ")
	));
	init();
}


void Bric::initTDirectory() {
	m_tDirectory = unique_ptr<TDirectory>(new TDirectory(name().toString().c_str(), title().c_str()));
	dbrx_log_debug("Created new TDirectory for bric \"%s\" with path \"%s\""_format(absolutePath(), localTDirectory()->GetPath()));
}


void Bric::applyConfig(const PropVal& config) {
	dbrx_log_debug("Applying config to bric \"%s\""_format(absolutePath()));
	for (const auto& entry: config.asProps()) {
		PropKey componentName = entry.first;
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
		const auto& component = *entry.second;
		PropVal componentConfig = component.getConfig();
		if (! componentConfig.isNone())	props[component.name()] = std::move(componentConfig);

		const auto& dbc = m_dynBricClassNames.find(component.name());
		if (dbc != m_dynBricClassNames.end()) {
			PropVal &cfg = props[component.name()];
			if (cfg.isNone()) cfg = Props();
			cfg[s_bricTypeKey] = dbc->second;
		}
	}
	return PropVal(std::move(props));
}


const bool Bric::hasComponent(PropKey componentName) const {
	return m_components.find(componentName) != m_components.end();
}


const BricComponent& Bric::getComponent(PropKey componentName) const {
	auto r = m_components.find(componentName);
	if (r == m_components.end()) throw out_of_range("No component \"%s\" found in component \"%s\""_format(componentName, absolutePath()));
	else return *r->second;
}

BricComponent& Bric::getComponent(PropKey componentName) {
	auto r = m_components.find(componentName);
	if (r == m_components.end()) throw out_of_range("No component \"%s\" found in component \"%s\""_format(componentName, absolutePath()));
	else return *r->second;
}



const BricComponent& Bric::getComponent(PropPath::Fragment componentPath) const {
	if (componentPath.empty()) throw runtime_error("Can't resolve empty component path inside bric \"%s\""_format(absolutePath()));
	const Bric* current = this;
	PropKey pathHead = componentPath.front();
	PropPath::Fragment pathTail = componentPath.tail();
	while (current != nullptr) {
		if (current->hasComponent(pathHead)) {
			const BricComponent *foundComp = &current->getComponent(pathHead);
			if (pathTail.empty()) {
				return *foundComp;
			} else {
				current = dynamic_cast<const Bric*>(foundComp);
				pathHead = pathTail.front();
				pathTail = pathTail.tail();
			}
		} else current = nullptr;
	}
	throw runtime_error("Couldn't resolve component path \"%s\" inside bric \"%s\""_format(componentPath, absolutePath()));
}

BricComponent& Bric::getComponent(PropPath::Fragment componentPath) {
	if (componentPath.empty()) throw runtime_error("Can't resolve empty component path inside bric \"%s\""_format(absolutePath()));
	Bric* current = this;
	PropKey pathHead = componentPath.front();
	PropPath::Fragment pathTail = componentPath.tail();
	while (current != nullptr) {
		if (current->hasComponent(pathHead)) {
			BricComponent *foundComp = &current->getComponent(pathHead);
			if (pathTail.empty()) {
				return *foundComp;
			} else {
				current = dynamic_cast<Bric*>(foundComp);
				pathHead = pathTail.front();
				pathTail = pathTail.tail();
			}
		} else current = nullptr;
	}
	throw runtime_error("Couldn't resolve component path \"%s\" inside bric \"%s\""_format(componentPath, absolutePath()));
}


BricComponent& Bric::getComponentRelToSiblings(PropPath::Fragment componentPath) {
	if (componentPath.empty()) throw runtime_error("Can't resolve empty path to component relative to siblings of bric \"%s\""_format(absolutePath()));
	PropKey pathHead = componentPath.front();
	PropPath::Fragment pathTail = componentPath.tail();
	Bric* current = this;
	while (current != nullptr) {
		if (pathHead == current->name()) {
			if (pathTail.empty()) return *current;
			else return current->getComponent(pathTail);
		} else if (current->hasParent()) {
			current = &current->parent();
			if (current->hasComponent(pathHead)) {
				return current->getComponent(componentPath);
			}
		} else current = nullptr;
	}
	throw out_of_range("Can't resolve component \"%s\" in siblings or upwards of \"%s\""_format(componentPath, absolutePath()));
}


const Bric& Bric::getBric(PropKey bricName) const {
	auto r = m_brics.find(bricName);
	if (r == m_brics.end()) throw out_of_range("No bric \"%s\" found in bric \"%s\""_format(bricName, absolutePath()));
	else return *r->second;
}

Bric& Bric::getBric(PropKey bricName) {
	auto r = m_brics.find(bricName);
	if (r == m_brics.end()) throw out_of_range("No bric \"%s\" found in bric \"%s\""_format(bricName, absolutePath()));
	else return *r->second;
}


const Bric::Terminal& Bric::getTerminal(PropKey terminalName) const {
	auto r = m_terminals.find(terminalName);
	if (r == m_terminals.end()) throw out_of_range("No terminal \"%s\" found in bric \"%s\""_format(terminalName, absolutePath()));
	else return *r->second;
}

Bric::Terminal& Bric::getTerminal(PropKey terminalName) {
	auto r = m_terminals.find(terminalName);
	if (r == m_terminals.end()) throw out_of_range("No terminal \"%s\" found in bric \"%s\""_format(terminalName, absolutePath()));
	else return *r->second;
}


const Bric::OutputTerminal& Bric::getOutput(PropKey outputName) const
	{ return dynamic_cast<const Bric::OutputTerminal&>(getTerminal(outputName)); }

Bric::OutputTerminal& Bric::getOutput(PropKey outputName)
	{ return dynamic_cast<Bric::OutputTerminal&>(getTerminal(outputName)); }


const Bric::InputTerminal& Bric::getInput(PropKey inputName) const
	{ return dynamic_cast<const Bric::InputTerminal&>(getTerminal(inputName)); }

Bric::InputTerminal& Bric::getInput(PropKey inputName)
	{ return dynamic_cast<Bric::InputTerminal&>(getTerminal(inputName)); }


const Bric::ParamTerminal& Bric::getParam(PropKey paramName) const
	{ return dynamic_cast<const Bric::ParamTerminal&>(getTerminal(paramName)); }

Bric::ParamTerminal& Bric::getParam(PropKey paramName)
	{ return dynamic_cast<Bric::ParamTerminal&>(getTerminal(paramName)); }


void Bric::addDynOutput(std::unique_ptr<Bric::OutputTerminal> terminal) {
	if (canHaveDynOutputs()) {
		terminal->setParent(this);
		Terminal* termPtr = terminal.get();
		m_dynTerminals[termPtr->name()] = std::move(terminal);
	} else throw runtime_error("Bric \"%s\" cannot have dynamic outputs"_format(absolutePath()));
}


void Bric::addDynInput(std::unique_ptr<Bric::InputTerminal> terminal) {
	if (canHaveDynInputs()) {
		terminal->setParent(this);
		Terminal* termPtr = terminal.get();
		m_dynTerminals[termPtr->name()] = std::move(terminal);
	} else throw runtime_error("Bric \"%s\" cannot have dynamic Inputs"_format(absolutePath()));
}


void Bric::initBricHierarchy() {
	if (hasParent()) throw invalid_argument("Can't init bric hierarchy starting from bric \"%s\", not a top bric"_format(absolutePath()));

	disconnectInputs();
	connectInputs();
	initRecursive();
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


Bric* Bric::addSource(Bric *source) {
	Bric* dstBric = this;
	Bric* srcBric = source;

	size_t dstDepth = dstBric->hierarchyLevel();
	size_t srcDepth = srcBric->hierarchyLevel();
	for (size_t i = dstDepth; i > srcDepth; --i) dstBric = &dstBric->parent();
	for (size_t i = srcDepth; i > dstDepth; --i) srcBric = &srcBric->parent();

	assert(dstBric->hierarchyLevel() == srcBric->hierarchyLevel());

	while (dstBric->hasParent() && !srcBric->siblingOf(*dstBric)) {
		dstBric = &dstBric->parent();
		srcBric = &srcBric->parent();
	}

	if (srcBric->siblingOf(*dstBric)) {
		dbrx_log_trace("Establishing source/dest relationship between brics \"%s\" and \"%s\"", dstBric->absolutePath(), srcBric->absolutePath());
		dstBric->m_sources.push_back(srcBric);
		srcBric->m_dests.push_back(dstBric);
		return srcBric;
	} else throw invalid_argument("Can't establish source/dest relationship between unrelated brics \"%s\" and \"%s\""_format(absolutePath(), source->absolutePath()));
}


} // namespace dbrx
