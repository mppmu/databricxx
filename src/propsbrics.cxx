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


#include "propsbrics.h"

#include "logging.h"

using namespace std;


namespace dbrx {


Bric::InputTerminal* PropsSplitter::ContentGroup::connectInputToInner(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath) {
	if (sourcePath.size() >= 2) subGroup(sourcePath.front());
	return Bric::connectInputToInner(bric, inputName, sourcePath);
}


PropsSplitter::ContentGroup& PropsSplitter::ContentGroup::subGroup(PropKey name) {
	if (hasComponent(name)) {
		return dynamic_cast<ContentGroup&>(getBric(name));
	} else {
		dbrx_log_trace("Creating new sub-group \"%s\" in content group \"%s\""_format(name, absolutePath()));
		return dynamic_cast<ContentGroup&>(*addDynBric(unique_ptr<ContentGroup>(new ContentGroup(this, name))));
	}
}


void PropsSplitter::ContentGroup::splitPropVal(const PropVal& from) {
	for (auto &elem: m_outputs) {
		OutputTerminal &output = *elem.second;
		output.value().fromPropVal(from[output.name()]);
	}

	for (auto &entry: m_brics) {
		ContentGroup& group = *dynamic_cast<ContentGroup*>(entry.second);
		group.splitPropVal(from[group.name()]);
	}
}


void PropsSplitter::processInput() {
	output.splitPropVal(input);
}



void PropsBuilder::ContentGroup::applyConfig(const PropVal& config) {
	// Just save the configuration, config for sub-groups stays empty:
	preConfig();
	m_config = config.asProps();
	postConfig();
}


PropVal PropsBuilder::ContentGroup::getConfig() const {
	// Includes configuration for sub-groups:
	return m_config;
}



void PropsBuilder::ContentGroup::disconnectInputs() {
	// Implicitly also removes all sub-groups:
	removeDynamicComponents();
}



void PropsBuilder::ContentGroup::connectInputs() {
	createAndConnectInputs(m_config);
}



void PropsBuilder::ContentGroup::createAndConnectInputs(const Props& config) {
	assert(m_components.empty());

	for (const auto& entry: config) {
		PropKey propKey = entry.first;
		const PropVal &propVal = entry.second;

		if (propVal.isProps()) {
			subGroup(propKey).createAndConnectInputs(propVal.asProps());
		} else if (BCReference::isReference(propVal)) {
			PropPath sourcePath(BCReference(propVal).path());
			if (sourcePath.empty()) throw invalid_argument("Invalid empty source in configuration for \"%s\""_format(absolutePath()));
			createAndConnectInput(propKey, sourcePath);
		} else {
			std::unique_ptr<Bric::InputTerminal> fixedInput(new Input<PropVal>(nullptr, propKey));
			InputTerminal* fixedInputPtr = fixedInput.get();
			addDynInput(std::move(fixedInput));
			fixedInputPtr->applyConfig(propVal);
		}
	}
}


void PropsBuilder::ContentGroup::createAndConnectInputs(Bric &sourceBric, const PropPath &sourceBricPath) {
	auto &sourceOutputs = sourceBric.outputs();
	dbrx_log_trace("Adding all outputs of \"%s\" to content group \"%s\"", sourceBric.absolutePath(), absolutePath());
	if (sourceOutputs.size() >= 1) {
		for (auto& elem: sourceOutputs) {
			PropKey outputName = elem.second->name();
			createAndConnectInput(outputName, sourceBricPath % outputName);
		}
	} else {
		dbrx_log_warn("Source \"%s\" for content group \"%s\" has no outputs", sourceBric.absolutePath(), absolutePath());
	}
}


void PropsBuilder::ContentGroup::createAndConnectInput(PropKey sourceName, const PropPath &sourcePath) {
	BricComponent* sourceComponent = &m_builder->getComponentRelToSiblings(sourcePath);

	if (dynamic_cast<Bric*>(sourceComponent)) {
		Bric &sourceBric = dynamic_cast<Bric&>(*sourceComponent);
		subGroup(sourceName).createAndConnectInputs(sourceBric, sourcePath);
	} else if (dynamic_cast<Bric::Terminal*>(sourceComponent)) {
		connectInputToSiblingOrUp(*this, sourceName, sourcePath);
	} else {
		throw logic_error("BricComponent \"%s\" is neither Bric nor Terminal"_format(sourceComponent->absolutePath()));
	}
}


PropsBuilder::ContentGroup& PropsBuilder::ContentGroup::subGroup(PropKey name) {
	if (hasComponent(name)) {
		return dynamic_cast<ContentGroup&>(getBric(name));
	} else {
		dbrx_log_trace("Creating new sub-group \"%s\" in content group \"%s\""_format(name, absolutePath()));
		return dynamic_cast<ContentGroup&>(*addDynBric(unique_ptr<ContentGroup>(new ContentGroup(m_builder, this, name))));
	}
}


PropVal PropsBuilder::ContentGroup::createPropVal() {
	Props props;

	for (auto &elem: m_inputs) {
		InputTerminal &input = *elem.second;
		props[input.name()] = input.value().toPropVal();
	}

	for (auto &entry: m_brics) {
		ContentGroup& group = *dynamic_cast<ContentGroup*>(entry.second);
		props[group.name()] = group.createPropVal();
	}

	return PropVal(std::move(props));
}


void PropsBuilder::processInput() {
	output = input.createPropVal();
}


} // namespace dbrx
