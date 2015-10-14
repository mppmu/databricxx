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


#include "rootiobrics.h"

#include <TH1.h>

#include "logging.h"
#include "RootIO.h"
#include "TypeReflection.h"
#include "WrappedTObj.h"

using namespace std;


namespace dbrx {


void RootTreeReader::Entry::connectBranches(Bric* contextBric, TTree* inputTree) {
	for (auto &elem: m_outputs) {
		OutputTerminal *terminal = elem.second;
		dbrx_log_debug("Connecting TTree branch \"%s\" in \"%s\"", terminal->name(), absolutePath());
		RootIO::inputValueFrom(terminal->value(), inputTree, terminal->name().toString());
	}
}


void RootTreeReader::processInput() {
	auto inputTChain = dynamic_cast<const TChain*>(input.value().ptr());
	if (inputTChain != nullptr) {
		// If input is a TChain, we can simply clone it
		m_chain = std::unique_ptr<TChain>(dynamic_cast<TChain*>(inputTChain->Clone()));
	} else {
		// If input is a "real" TTree, we have to reopen it as the input is
		// read-only and as there may even be multiple readers:
		string inputPath = "%s/%s"_format(input->GetDirectory()->GetPath(), input->GetName());
		size_t colonPos = inputPath.find_first_of(':');
		if (colonPos == inputPath.npos) throw runtime_error("Can't reopen TTree with path \"%s\", unknown path format"_format(inputPath));
		string fileName = inputPath.substr(0, colonPos);
		string treeName = inputPath.substr(colonPos+1, inputPath.npos);
		dbrx_log_debug("Creating input TChain with file \"%s\" for tree \"%s\" in bric \"%s\""_format(fileName, treeName, absolutePath()));

		m_chain = std::unique_ptr<TChain>(new TChain(treeName.c_str()));
		m_chain->Add(fileName.c_str());
	}

	m_chain->SetCacheSize(cacheSize);
	m_chain->SetBranchStatus("*", false);

	entry.connectBranches(this, m_chain.get());

	index = firstEntry - 1;
	size = m_chain->GetEntries() - firstEntry.get();
	if (ssize_t(nEntries) > 0) size = std::min(ssize_t(nEntries), size.get());
}


bool RootTreeReader::nextOutput() {
	if (index.get() + 1 < firstEntry.get() + size.get()) {
		++index;
		m_chain->GetEntry(index);
		return true;
	} else return false;
}



TTree* RootTreeWriter::newTree(TDirectory *directory) {
	TempChangeOfTDirectory outTDir(directory);
	return new TTree(treeName.get().c_str(), treeTitle.get().c_str());
}


void RootTreeWriter::Entry::applyConfig(const PropVal& config) {
	Props configProps = config.asProps();
	m_inputSources.clear(); m_inputSources.reserve(configProps.size());
	for (const auto &e: config.asProps())
		m_inputSources.push_back({e.first, BCReference(e.second).path()});
}


PropVal RootTreeWriter::Entry::getConfig() const {
	Props configProps;
	for (const auto &br: m_inputSources) configProps[br.first] = BCReference(br.second);
	return PropVal(std::move(configProps));
}


void RootTreeWriter::Entry::connectInputs() {
	dbrx_log_trace("Creating and connecting dynamic inputs of bric \"%s\"", absolutePath());
	if (m_inputsConnected) throw logic_error("Can't connect already connected inputs in bric \"%s\""_format(absolutePath()));

	for (const auto &br: m_inputSources)
		connectInputToSiblingOrUp(*this, br.first, br.second);
}


void RootTreeWriter::Entry::disconnectInputs() {
	m_dynBrics.clear();
}


void RootTreeWriter::Entry::createOutputBranches(TTree *tree) {
	std::map<std::string, const InputTerminal*> sortedInputs;
	for (const auto &in: inputs()) sortedInputs[in.first.toString()] = in.second;
	for (const auto &in: sortedInputs) {
		const string& branchName = in.first;
		const InputTerminal* branchInput = in.second;
		dbrx_log_trace("Creating output branch \"%s\" for input \"%s\" of \"%s\"", branchName, branchInput->name(), absolutePath());
		RootIO::outputValueTo(branchInput->value(), tree, branchName);
	}
}


RootTreeWriter::Entry::Entry(RootTreeWriter *writer, PropKey entryName)
	: DynInputGroup(writer, entryName), m_writer(writer) {}


void RootTreeWriter::newReduction() {
	// Dummy output tree:
	output.value() = unique_ptr<TTree>(newTree(localTDirectory()));

	// Actual output trees, created directly inside TDirectories of consumers:
	m_trees.clear();
	for (auto &getDir: m_outputDirProviders) {
		TDirectory* targetDirectory = getDir();
		dbrx_log_debug("Creating new TTree \"%s\" as output of bric \"%s\" in TDirectory \"%s\" ", treeName.get(), absolutePath(), targetDirectory->GetPath());
		TTree* tree = newTree(targetDirectory);
		entry.createOutputBranches(tree);
		m_trees.push_back(tree);
	}
}


void RootTreeWriter::processInput() {
	for (auto tree: m_trees) {
		dbrx_log_trace("Filling output tree \"%s/%s\" of \"%s\"", tree->GetDirectory()->GetPath(), tree->GetName(), absolutePath());
		tree->Fill();
	}
}


void RootTreeWriter::finalizeReduction() {
}



Bric::InputTerminal* RootFileReader::ContentGroup::connectInputToInner(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath) {
	if (sourcePath.size() >= 2) subGroup(sourcePath.front());
	return Bric::connectInputToInner(bric, inputName, sourcePath);
}


RootFileReader::ContentGroup& RootFileReader::ContentGroup::subGroup(PropKey name) {
	if (hasComponent(name)) {
		return dynamic_cast<ContentGroup&>(getBric(name));
	} else {
		dbrx_log_trace("Creating new sub-group \"%s\" in content group \"%s\""_format(name, absolutePath()));
		return dynamic_cast<ContentGroup&>(*addDynBric(unique_ptr<ContentGroup>(new ContentGroup(m_reader, this, name))));
	}
}


void RootFileReader::ContentGroup::releaseOutputValues() {
	for (auto &elem: m_outputs) if (! elem.second->value().empty()) {
		if (elem.second->value().isPtrAssignableTo(typeid(AbstractWrappedTObj))) {
			AbstractWrappedTObj *outputWrappedTObj = elem.second->value().typedPtr<AbstractWrappedTObj>();
			outputWrappedTObj->releaseTObj().release();
		} else {
			OutputTerminal &output = *elem.second;
			output.value().untypedRelease();
		}
	}
}


void RootFileReader::ContentGroup::addDynOutput(std::unique_ptr<Bric::OutputTerminal> terminal) {
	// Ensure output is empty - values will be owned by the input TFile, so
	// must be able to release output at any time.
	terminal->value().clear();

	Bric::addDynOutput(std::move(terminal));
}


void RootFileReader::ContentGroup::readObjects() {
	// Release output values, they're either empty or we don't really own them:
	releaseOutputValues();

	if (isTopGroup()) {
		m_inputDir = m_reader->m_inputFile.get();
	} else {
		TDirectory *parentInputDir = dynamic_cast<ContentGroup&>(parent()).m_inputDir;
		dbrx_log_trace("Looking up sub-directory \"%s\" inside \"%s\" for content group \"%s\" "_format(name(), parentInputDir->GetPath(), absolutePath()));
		m_inputDir = parentInputDir->GetDirectory(name().toString().c_str());
		if (m_inputDir == nullptr) throw runtime_error("Could not find sub-directory \"%s\" in TDirectory \"%s\""_format(name(), parentInputDir->GetPath()));
	}

	for (auto &elem: m_outputs) {
		OutputTerminal &output = *elem.second;
		dbrx_log_trace("Reading object \"%s\" from \"%s\" in content group \"%s\"", output.name(), m_inputDir->GetPath(), absolutePath());
		TObject *obj = m_inputDir->Get(output.name().toString().c_str());
		if (obj == nullptr) throw runtime_error("Could not read object \"%s\" from TDirectory \"%s\""_format(output.name(), m_inputDir->GetPath()));

		TypeReflection outputType(output.value().typeInfo());
		TypeReflection objectType(obj->IsA());

		if (output.value().isPtrAssignableTo(typeid(AbstractWrappedTObj))) {
			if (output.value().empty()) output.value().setToDefault();
			AbstractWrappedTObj *outputWrappedTObj = output.value().typedPtr<AbstractWrappedTObj>();
			if (outputWrappedTObj->canWrapTObj(obj) ) {
				// Wrap output value around obj - obj is really owned by input
				// TFile, not by us, so we'll have to release it again later:
				outputWrappedTObj->wrapTObj(unique_ptr<TObject>(obj));
			} else {
				TypeReflection outputWrappedType(outputWrappedTObj->typeInfo());
				throw logic_error("Wrapped type %s of WrappedTObj output terminal \"%s\" is incompatible with type %s of object \"%s\" read from \"%s\""_format(outputWrappedType.name(), output.absolutePath(), objectType.name(), output.name(), m_inputDir->GetPath()));				
			}
		} else {
			if (outputType.isPtrAssignableFrom(objectType) ) {
				// Point output value to obj - obj is really owned by input
				// TFile, not by us, so we'll have to release it again later:
				output.value().untypedOwn(obj);
			} else {
				throw logic_error("Type %s of output terminal \"%s\" is incompatible with type %s of object \"%s\" read from \"%s\""_format(outputType.name(), output.absolutePath(), objectType.name(), output.name(), m_inputDir->GetPath()));
			}
		}
	}

	for (auto &entry: m_brics) dynamic_cast<ContentGroup*>(entry.second)->readObjects();
}


RootFileReader::ContentGroup::ContentGroup(RootFileReader *reader, Bric *parentBric, PropKey groupName)
	: DynOutputGroup(parentBric, groupName), m_reader(reader) {}


RootFileReader::ContentGroup::~ContentGroup() {
	// Release output values, they're either empty or we don't really own them:
	releaseOutputValues();
}


void RootFileReader::processInput() {
	dbrx_log_debug("Opening TFile \"%s\" for read in bric \"%s\""_format(input->c_str(), absolutePath()));
	m_inputFile = unique_ptr<TFile>(new TFile(input->c_str(), "read"));
	content.readObjects();
}





const PropKey RootFileWriter::s_thisDirName(".");


void RootFileWriter::ContentGroup::connectInputs() {
	dbrx_log_trace("Creating and connecting dynamic inputs of bric \"%s\" and all inner brics", absolutePath());
	if (m_inputsConnected) throw logic_error("Can't connect already connected inputs in bric \"%s\""_format(absolutePath()));

	size_t nextInputIdx = 0;
	m_sourceInfos.clear();

	for (const PropPath &sourcePath: m_content) {
		InputTerminal *input = connectInputToSiblingOrUp(*this, int64_t(nextInputIdx++), sourcePath);
		dbrx_log_trace("Triggering input from terminal \"%s\" on output of bric \"%s\" in \"%s\"", input->srcTerminal()->absolutePath(), input->effSrcBric()->absolutePath(), absolutePath());

		auto constTDirOutBric = dynamic_cast<const RootTreeWriter*>(&input->srcTerminal()->parent());
		if (constTDirOutBric) {
			// TODO: Find a way to eliminate this const_cast:
			auto tdirOutBric = const_cast<RootTreeWriter*>(constTDirOutBric);

			dbrx_log_trace("trigger Adding TDirectory of bric \"%s\" to output directories of bric \"%s\"", absolutePath(), tdirOutBric->absolutePath());
			tdirOutBric->addOutputDirProvider([&]() {
				m_writer->openOutputForWrite();
				return m_outputDir;
			} );
		}

		if (input->value().isPtrAssignableTo(typeid(AbstractWrappedTObj))) {
			const AbstractWrappedTObj *inputWrappedTObj = input->value().typedPtr<AbstractWrappedTObj>();
			if (! TypeReflection(typeid(TNamed)).isPtrAssignableFrom(inputWrappedTObj->typeInfo()) )
				throw logic_error("Source terminal \"%s\" used for input in \"%s\" is of type WrappedTObj, but does not wrap a TNamed"_format(input->srcTerminal()->absolutePath(), absolutePath()));
		} else {
			if (! TypeReflection(typeid(TNamed)).isPtrAssignableFrom(input->value().typeInfo()) )
				throw logic_error("Source terminal \"%s\" used for input in \"%s\" is not of type TNamed"_format(input->srcTerminal()->absolutePath(), absolutePath()));
		}

		m_sourceInfos[input->effSrcBric()].inputs.push_back(input);
	}

	for (const auto& brics: m_brics) connectInputsOn(*brics.second);
	for (const auto& brics: m_brics) updateDepsOn(*brics.second);
}


void RootFileWriter::ContentGroup::initTDirectory() {
	// Nothing to do, TFiles and TDirectories will be created later
}


RootFileWriter::ContentGroup& RootFileWriter::ContentGroup::subGroup(PropKey name) {
	if (hasComponent(name)) {
		return dynamic_cast<ContentGroup&>(getBric(name));
	} else {
		dbrx_log_trace("Creating new sub-group \"%s\" in content group \"%s\""_format(name, absolutePath()));
		return dynamic_cast<ContentGroup&>(*addDynBric(unique_ptr<ContentGroup>(new ContentGroup(m_writer, this, name))));
	}
}


void RootFileWriter::ContentGroup::processInput() {
	TempChangeOfTDirectory tDirChange(m_outputDir);

	for (auto &si: m_sourceInfos) {
		const Bric* source = si.first;
		SourceInfo &info = si.second;
		if (info.inputCounter < outputCounterOn(*source)) {
			info.inputCounter = outputCounterOn(*source);
			for (const Terminal* input: info.inputs) {

				const TNamed *inputObject = nullptr;
				if (input->value().isPtrAssignableTo(typeid(AbstractWrappedTObj))) {
					const AbstractWrappedTObj *inputWrappedTObj = input->value().typedPtr<AbstractWrappedTObj>();
					inputObject = (const TNamed*)inputWrappedTObj->getPtr();
				} else {
					inputObject = (const TNamed*)input->value().untypedPtr();
				}

				if (typeid(*inputObject) == typeid(TTree)) {
					// TTree is special - it or a clone of it should already be inside m_outputDir:
					dbrx_log_trace("No further action necessary for output of TTree \"%s\" to content group \"%s\"", inputObject->GetName(), absolutePath());
				} else {
					// Need to clone inputObject to own it:
					dbrx_log_trace("Cloning object \"%s\" to content group \"%s\"", inputObject->GetName(), absolutePath());
					TNamed *outputObject = (TNamed*) inputObject->Clone();
					writeObject(outputObject);
				}
			}
		}
	}
	for (auto &entry: m_brics) dynamic_cast<ContentGroup*>(entry.second)->processInput();
}


void RootFileWriter::ContentGroup::addContent(const PropVal &content) {
	if (content.isProps()) for (const auto &entry: content.asProps()) {
		const auto &dirName = entry.first;
		const auto &dirContent = entry.second;
		if (dirName == s_thisDirName) addContent(dirContent);
		else subGroup(dirName).addContent(dirContent);
	} else {
		for (const auto pv: content) {
			PropPath sourcePath(BCReference(pv).path());
			if (sourcePath.empty()) throw invalid_argument("Invalid empty source in configuration for \"%s\""_format(absolutePath()));
			addContent(sourcePath);
		}
	}
}


void RootFileWriter::ContentGroup::addContent(const PropPath &sourcePath) {
	BricComponent* sourceComponent = &m_writer->getComponentRelToSiblings(sourcePath);

	if (dynamic_cast<Bric*>(sourceComponent)) {
		Bric &sourceBric = dynamic_cast<Bric&>(*sourceComponent);
		auto &sourceOutputs = sourceBric.outputs();
		dbrx_log_trace("Adding all outputs of Bric \"%s\" to content group \"%s\"", sourceBric.absolutePath(), absolutePath());
		if (sourceOutputs.size() >= 1) {
			for (auto& elem: sourceOutputs) {
				addContent(sourcePath % elem.second->name());
			}
		} else {
			dbrx_log_warn("Source \"%s\" for content group \"%s\" has no outputs", sourceBric.absolutePath(), absolutePath());
		}
	} else if (dynamic_cast<Bric::Terminal*>(sourceComponent)) {
		m_content.push_back(sourcePath);
		dbrx_log_trace("Added source terminal \"%s\" to content group \"%s\"", sourcePath, absolutePath());
	} else {
		throw logic_error("BricComponent \"%s\" is neither Bric nor Terminal"_format(sourceComponent->absolutePath()));
	}
}


void RootFileWriter::ContentGroup::newOutput() {
	for (auto &entry: m_sourceInfos) { entry.second.inputCounter = 0; }

	if (isTopGroup()) {
		m_outputDir = m_writer->outputFile.value().ptr();
	} else {
		TDirectory* parentOutputDir = dynamic_cast<ContentGroup&>(parent()).m_outputDir;

		const char *subDirName = name().toString().c_str();
		dbrx_log_trace("Creating new sub-directory \"%s/%s\" in output file in content group \"%s\""_format(parentOutputDir->GetPath(), subDirName, absolutePath()));

		// Note: Have to use TDirectory::mkdir to create subdirs inside TFiles, "new TDirectory" doesn't work:
		m_outputDir = parentOutputDir->mkdir(subDirName, subDirName);
	}

	for (auto &entry: m_brics) dynamic_cast<ContentGroup*>(entry.second)->newOutput();
}


RootFileWriter::ContentGroup::ContentGroup(RootFileWriter *writer, Bric *parentBric, PropKey groupName)
	: DynInputGroup(parentBric, groupName), m_writer(writer) {}



void RootFileWriter::writeObject(TNamed *obj) {
	if (string(obj->GetName()).empty())
		throw invalid_argument("Refusing to add object with empty name to TDirectory");

	bool autoAdded = (
		TH1::AddDirectoryStatus() &&
			(dynamic_cast<TH1*>(obj) != 0)
	) || (
		TDirectory::AddDirectoryStatus() && (obj->IsA()->GetDirectoryAutoAdd() != nullptr)
	);

	if (!autoAdded) obj->Write();
}


void RootFileWriter::connectInputs() {
	dbrx_log_trace("Setting up content groups for bric \"%s\"", absolutePath());
	inputs.addContent(content);

	AsyncReducerBric::connectInputs();
}


void RootFileWriter::newReduction() {
	openOutputForWrite();
}


void RootFileWriter::processInput() {
	inputs.processInput();
}


void RootFileWriter::finalizeReduction() {
	finalizeOutput();
	output = outputFile->GetName();
}


void RootFileWriter::openOutputForWrite() {
	// Legal to call when already open:
	if (m_outputReadyForWrite) return;

	const char *outFileName = fileName->c_str();
	const char *outFileTitle = title->c_str();
	dbrx_log_debug("Creating TFile \"%s\" with title \"%s\" in bric \"%s\""_format(outFileName, outFileTitle, absolutePath()));
	TFile *tfile = TFile::Open(outFileName, "RECREATE", outFileTitle);
	if (tfile == nullptr) throw runtime_error("Could not create TFile \"%s\""_format(outFileName));
	outputFile.value() = unique_ptr<TFile>(tfile);

	inputs.newOutput();

	m_outputReadyForWrite = true;
}


void RootFileWriter::finalizeOutput() {
	// Legal to call when already closed:
	if (! m_outputReadyForWrite) return;

	dbrx_log_debug("Writing TFile \"%s\" in bric \"%s\""_format(outputFile->GetName(), absolutePath()));
	outputFile->Write();

	outputFile->ReOpen("READ");

	m_outputReadyForWrite = false;
}


RootFileWriter::~RootFileWriter() {
	finalizeOutput();
}


} // namespace dbrx
