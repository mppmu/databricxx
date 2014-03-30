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

#include <TString.h>


using namespace std;


namespace dbrx {


const Name Bric::s_defaultInputName("input");

const Name Bric::s_defaultOutputName("output");


void Bric::addTerminal(Terminal* terminal) {
	auto r = m_terminals.find(terminal->name());
	if (r != m_terminals.end()) throw invalid_argument(TString::Format("Can't add duplicate terminal with name \"%s\" to bric \"%s\"", terminal->name().c_str(), name().c_str()).Data());
	m_terminals[terminal->name()] = terminal;
}


void Bric::addParam(ParamTerminal* param) {
	addTerminal(param);
	m_params[param->name()] = param;
}


const Bric::Terminal& Bric::getTerminal(const Name &terminalName) const {
	auto r = m_terminals.find(terminalName);
	if (r == m_terminals.end()) throw invalid_argument(TString::Format("No terminal \"%s\" found in bric \"%s\"", terminalName.c_str(), name().c_str()).Data());
	else return *r->second;
}

Bric::Terminal& Bric::getTerminal(const Name &terminalName) {
	auto r = m_terminals.find(terminalName);
	if (r == m_terminals.end()) throw invalid_argument(TString::Format("No terminal \"%s\" found in bric \"%s\"", terminalName.c_str(), name().c_str()).Data());
	else return *r->second;
}


const Bric::Terminal& Bric::getTerminal(const Name &terminalName, const std::type_info& typeInfo) const {
	const Bric::Terminal& terminal = getTerminal(terminalName);
	if (terminal.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of terminal \"%s\" doesn't match requested type \"%s\"", terminal.name().c_str(), terminal.typeInfo().name()).Data());
	return terminal;
}

Bric::Terminal& Bric::getTerminal(const Name &terminalName, const std::type_info& typeInfo) {
	Bric::Terminal& terminal = getTerminal(terminalName);
	if (terminal.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of terminal \"%s\" doesn't match requested type \"%s\"", terminal.name().c_str(), terminal.typeInfo().name()).Data());
	return terminal;
}


const Bric::OutputTerminal& Bric::getOutput(const Name &outputName) const {
	throw runtime_error("Bric has no outputs");
}

Bric::OutputTerminal& Bric::getOutput(const Name &outputName) {
	throw runtime_error("Bric has no outputs");
}


const Bric::OutputTerminal& Bric::getOutput(const Name &outputName, const std::type_info& typeInfo) const {
	const Bric::OutputTerminal& output = getOutput(outputName);
	if (output.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of output \"%s\" doesn't match requested type \"%s\"", output.name().c_str(), output.typeInfo().name()).Data());
	return output;
}

Bric::OutputTerminal& Bric::getOutput(const Name &outputName, const std::type_info& typeInfo) {
	Bric::OutputTerminal& output = getOutput(outputName);
	if (output.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of output \"%s\" doesn't match requested type \"%s\"", output.name().c_str(), output.typeInfo().name()).Data());
	return output;
}


const Bric::InputTerminal& Bric::getInput(const Name &inputName) const {
	throw runtime_error("Bric has no outputs");
}

Bric::InputTerminal& Bric::getInput(const Name &inputName) {
	throw runtime_error("Bric has no outputs");
}


const Bric::InputTerminal& Bric::getInput(const Name &inputName, const std::type_info& typeInfo) const {
	const Bric::InputTerminal& input = getInput(inputName);
	if (input.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of input \"%s\" doesn't match requested type \"%s\"", input.name().c_str(), input.typeInfo().name()).Data());
	return input;
}

Bric::InputTerminal& Bric::getInput(const Name &inputName, const std::type_info& typeInfo) {
	Bric::InputTerminal& input = getInput(inputName);
	if (input.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of input \"%s\" doesn't match requested type \"%s\"", input.name().c_str(), input.typeInfo().name()).Data());
	return input;
}


const Bric::ParamTerminal& Bric::getParam(const Name &paramName) const {
	auto r = m_params.find(paramName);
	if (r == m_params.end()) throw invalid_argument(TString::Format("No param \"%s\" found in bric \"%s\"", paramName.c_str(), name().c_str()).Data());
	else return *r->second;
}


Bric::ParamTerminal& Bric::getParam(const Name &paramName) {
	auto r = m_params.find(paramName);
	if (r == m_params.end()) throw invalid_argument(TString::Format("No param \"%s\" found in bric \"%s\"", paramName.c_str(), name().c_str()).Data());
	else return *r->second;
}


const Bric::ParamTerminal& Bric::getParam(const Name &paramName, const std::type_info& typeInfo) const {
	const Bric::ParamTerminal& param = getParam(paramName);
	if (param.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of param \"%s\" doesn't match requested type \"%s\"", param.name().c_str(), param.typeInfo().name()).Data());
	return param;
}

Bric::ParamTerminal& Bric::getParam(const Name &paramName, const std::type_info& typeInfo) {
	Bric::ParamTerminal& param = getParam(paramName);
	if (param.value().typeInfo() != typeInfo)
		throw runtime_error(TString::Format("Type of param \"%s\" doesn't match requested type \"%s\"", param.name().c_str(), param.typeInfo().name()).Data());
	return param;
}


std::ostream & Bric::printInfo(std::ostream &os) const {
	os << "Bric " << name() << ":" << endl;
	return os;
}



void BricWithOutputs::addOutput(OutputTerminal* output) {
	addTerminal(output);
	m_outputs[output->name()] = output;
}


std::ostream & BricWithOutputs::printOutputInfo(std::ostream &os) const {
	os << "  Outputs: ";
	for (auto x: m_outputs) os << " " << x.second->name() << "(" << x.second->value().typeInfo().name() << ")";
	os << endl;
	return os;
}


const Bric::OutputTerminal& BricWithOutputs::getOutput(const Name &outputName) const {
	auto r = m_outputs.find(outputName);
	if (r == m_outputs.end()) throw invalid_argument(TString::Format("No output \"%s\" found in bric \"%s\"", outputName.c_str(), name().c_str()).Data());
	else return *r->second;
}


Bric::OutputTerminal& BricWithOutputs::getOutput(const Name &outputName) {
	auto r = m_outputs.find(outputName);
	if (r == m_outputs.end()) throw invalid_argument(TString::Format("No output \"%s\" found in bric \"%s\"", outputName.c_str(), name().c_str()).Data());
	else return *r->second;
}


std::ostream & BricWithOutputs::printInfo(std::ostream &os) const {
	Bric::printInfo(os);
	printOutputInfo(os);
	return os;
}



void BricWithInputs::addInput(InputTerminal* input) {
	addTerminal(input);
	m_inputs[input->name()] = input;
}


std::ostream & BricWithInputs::printInputInfo(std::ostream &os) const {
	os << "  Inputs: ";
	for (auto x: m_inputs) os << " " << x.second->name() << "(" << x.second->value().typeInfo().name() << ")";
	os << endl;
	return os;
}


const Bric::InputTerminal& BricWithInputs::getInput(const Name &inputName) const {
	auto r = m_inputs.find(inputName);
	if (r == m_inputs.end()) throw invalid_argument(TString::Format("No input \"%s\" found in bric \"%s\"", inputName.c_str(), name().c_str()).Data());
	else return *r->second;
}


Bric::InputTerminal& BricWithInputs::getInput(const Name &inputName) {
	auto r = m_inputs.find(inputName);
	if (r == m_inputs.end()) throw invalid_argument(TString::Format("No input \"%s\" found in bric \"%s\"", inputName.c_str(), name().c_str()).Data());
	else return *r->second;
}


std::ostream & BricWithInputs::printInfo(std::ostream &os) const {
	Bric::printInfo(os);
	printInputInfo(os);
	return os;
}



std::ostream & BricWithInOut::printInfo(std::ostream &os) const {
	Bric::printInfo(os);
	printInputInfo(os);
	printOutputInfo(os);

	return os;
}


} // namespace dbrx
