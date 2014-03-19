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


using namespace std;


namespace dbrx {


const Bric::OutputTerminal& Bric::getOutput(const Name &outputName) const {
	throw invalid_argument("Bric has no outputs");
}


Bric::OutputTerminal& Bric::getOutput(const Name &outputName) {
	throw invalid_argument("Bric has no outputs");
}


const Bric::InputTerminal& Bric::getInput(const Name &inputName) const {
	throw invalid_argument("Bric has no outputs");
}


Bric::InputTerminal& Bric::getInput(const Name &inputName) {
	throw invalid_argument("Bric has no outputs");
}


std::ostream & Bric::printInfo(std::ostream &os) const {
	os << "Bric " << name() << ":" << endl;
	return os;
}



void BricWithOutputs::addOutput(OutputTerminal* output) {
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



void BricWithInputs::addInput(InputTerminal *input) {
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
