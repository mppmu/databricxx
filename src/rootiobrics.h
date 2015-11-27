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


#ifndef DBRX_ROOTIOBRICS_H
#define DBRX_ROOTIOBRICS_H

#include <functional>

#include "Bric.h"

#include <TNamed.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>

#include <sys/time.h>
#include <memory>

namespace dbrx {


class RootTreeReader: public MapperBric {
protected:
	std::unique_ptr<TChain> m_chain;

	// Additional members for progress informations
	//! Previously printed progress
	double m_previousProgress;
	//! CPU time at begin of reading
	struct timeval m_StartTime;
	//! CPU time from last event
	struct timeval m_LastTime;
	//! Current CPU time
	struct timeval m_CurrentTime;

public:
	class Entry final: public DynOutputGroup {
	public:
		void connectBranches(Bric* contextBric, TTree* inputTree);
		using DynOutputGroup::DynOutputGroup;
	};

	Input<TTree> input{this};

	Param<int64_t> cacheSize{this, "cacheSize", "Input read-ahead cache size (-1 for default)", -1};
	Param<int64_t> nEntries{this, "nEntries", "Number of entries to read (-1 for all)", -1};
	Param<int64_t> firstEntry{this, "firstEntry", "First entry to read", 0};

	// Parmeters for printing the progress informations while looping over a tree/chain
	Param<int> printProgress{this, "printProgress", "Print progress", false};
	Param<int> printETA{this, "printETA", "Print ETA", false};
	Param<double> progressPrecent{this, "progressPrecent", "Precentage when progress info is printed", 1000 };


	Entry entry{this, "entry"};

	Output<ssize_t> size{this, "size", "Number of entries"};
	Output<ssize_t> index{this, "index", "Number of entries"};

	void processInput() override;

	bool nextOutput() override;

	using MapperBric::MapperBric;
};



class RootTreeWriter: public ReducerBric {
protected:
	std::vector< std::function<TDirectory*()> > m_outputDirProviders;
	std::vector< TTree* > m_trees;

	TTree* newTree(TDirectory *directory);

public:
	class Entry final: public DynInputGroup {
	protected:
		RootTreeWriter* m_writer;
		std::vector< std::pair<PropKey, PropPath> > m_inputSources;

		void connectInputs() override;
		void disconnectInputs() override;

	public:
		void applyConfig(const PropVal& config) override;
		PropVal getConfig() const override;

		void processInput() override {}

		void createOutputBranches(TTree *tree);

		Entry() {}
		Entry(RootTreeWriter *writer, PropKey entryName);
	};

	virtual void addOutputDirProvider(std::function<TDirectory*()> provider)
		{ m_outputDirProviders.push_back(std::move(provider)); }

	Entry entry{this, "entry"};

	Param<std::string> treeName{this, "treeName", "Tree Name", "tree"};
	Param<std::string> treeTitle{this, "treeTitle", "Tree Title", ""};

	Output<TTree> output{this, "", "Output Tree"};

	void newReduction() override;

	void processInput() override;

	void finalizeReduction() override;

	using ReducerBric::ReducerBric;
};



class RootFileReader: public TransformBric {
protected:
	static const PropKey s_thisDirName;

	std::unique_ptr<TFile> m_inputFile;

public:
	class ContentGroup final: public DynOutputGroup {
	protected:
		RootFileReader* m_reader = nullptr;

		TDirectory* m_inputDir = nullptr;

		InputTerminal* connectInputToInner(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath) override;

		ContentGroup& subGroup(PropKey name);

		void releaseOutputValues();

	public:
		void addDynOutput(std::unique_ptr<Bric::OutputTerminal> terminal) final override;

		virtual void readObjects() final;

		bool isTopGroup() const { return &parent() == m_reader; }

		ContentGroup() {}
		ContentGroup(RootFileReader *writer, Bric *parentBric, PropKey groupName);

		~ContentGroup();
	};

	Input<std::string> input{this, "", "File Name"};

	ContentGroup content{this, this, "content"};

	void processInput() override;

	using TransformBric::TransformBric;
};



class RootFileWriter: public AsyncReducerBric {
protected:
	static const PropKey s_thisDirName;

	// Additional argument to choose if the object is overwritten
	static void writeObject(TNamed *obj, bool overwrite = false);

	bool m_outputReadyForWrite = false;

	void connectInputs() override;

public:
	class ContentGroup final: public DynInputGroup {
	protected:
		struct SourceInfo {
			size_t inputCounter;
			std::vector<const Terminal*> inputs;
		};

		RootFileWriter* m_writer;

		TDirectory* m_outputDir = nullptr;

		std::map<const Bric*, SourceInfo> m_sourceInfos;
		std::vector<PropPath> m_content;

		void connectInputs() override;

		void initTDirectory() override;

		ContentGroup& subGroup(PropKey name);

	public:
		void processInput() override;

		bool isTopGroup() const { return &parent() == m_writer; }

		void addContent(const PropVal &content);
		void addContent(const PropPath &sourcePath);

		void newOutput();

		ContentGroup() {}
		ContentGroup(RootFileWriter *writer, Bric *parentBric, PropKey groupName);
	};

	ContentGroup inputs{this, this, "inputs"};

	Param<std::string> fileName{this, "fileName", "File Name"};
	Param<std::string> title{this, "title", "Title"};
	Param<PropVal> content{this, "content", "Content"};

	// Is used to choose the mode for opening the root file
	Param<int> recreateFile{this, "recreate", "Recreate file", true};
	// Is used to choose if the contents of the root file are overwritten
	Param<int> overwrite{this, "overwrite", "Overwrite objects in file", false};

	Output<std::string> output{this, "output", "Output File Name"};
	Output<TFile> outputFile{this, "outputFile", "Output TFile"};

	void newReduction() override;

	void processInput() override;

	void finalizeReduction() override;

	//!! bool outputIsOpenForWrite() { return m_outputReadyForWrite; }
	virtual void openOutputForWrite();
	virtual void finalizeOutput();

	~RootFileWriter() override;

	using AsyncReducerBric::AsyncReducerBric;
};


} // namespace dbrx

#endif // DBRX_ROOTIOBRICS_H
