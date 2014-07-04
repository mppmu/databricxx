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

#include <memory>
#include <atomic>
#include <stdexcept>
#include <map>
#include <iosfwd>

#include <TDirectory.h>

#include "Props.h"
#include "HasValue.h"
#include "logging.h"


namespace dbrx {


class Bric;
class BricWithOutputs;
class BricWithInputs;



class BricComponent: public virtual Configurable {
protected:
	struct NotReconfigurable : public std::runtime_error { using std::runtime_error::runtime_error; };

	virtual void setParent(Bric *parentBric) = 0;

public:
	virtual PropKey name() const = 0;

	virtual void setName(PropKey componentName) = 0;

	virtual PropPath absolutePath() const final;

	virtual size_t hierarchyLevel() const final;

	virtual bool hasParent() const = 0;

	virtual const Bric& parent() const = 0;
	virtual Bric& parent() = 0;

	virtual bool siblingOf(BricComponent &other) const final
		{ return hasParent() && other.hasParent() && (&parent() == &other.parent()); }

	virtual const std::string& title() const = 0;
	virtual void setTitle(std::string newTitle) = 0;

	virtual bool isInside(const Bric& other) const final;

	BricComponent& operator=(const BricComponent& v) = delete;
	BricComponent& operator=(BricComponent &&v) = delete;

	friend class Bric;
};


class BricComponentImpl: public virtual BricComponent {
protected:
	PropKey m_key;
	Bric *m_parent = nullptr;

	std::string m_title;

	void setParent(Bric *parentBric) final override;

public:
	PropKey name() const final override { return m_key; }

	void setName(PropKey componentName) final override {
		if (!hasParent()) m_key = componentName;
		else throw std::logic_error("Can't change name for component \"%s\" because it already has a parent"_format(absolutePath()));
	}

	bool hasParent() const final override { return m_parent != nullptr; }

	const std::string& title() const final override { return m_title; }
	void setTitle(std::string newTitle) final override { m_title = std::move(newTitle); }

	const Bric& parent() const final override { return *m_parent; }
	Bric& parent() final override { return *m_parent; }

	BricComponentImpl() {}

	BricComponentImpl(PropKey componentName): m_key(componentName) {}

	BricComponentImpl(PropKey componentName, std::string componentTitle)
		: BricComponentImpl(componentName) { m_title = std::move(componentTitle); }
};



class Bric: public virtual BricComponent {
public:
	class OutputTerminal;
	class InputTerminal;
	class ParamTerminal;


	class Terminal: public virtual BricComponent, public virtual HasValue {
	public:
		virtual OutputTerminal* createMatchingDynOutput(Bric* outputBric,
			PropKey outputName, std::string outputTitle = "") = 0;

		virtual InputTerminal* createMatchingDynInput(Bric* inputBric,
			PropKey inputName, std::string inputTitle = "") = 0;
	};


	class OutputTerminal: public virtual Terminal, public virtual HasWritableValue {};


	class InputTerminal: public virtual Terminal, public virtual HasConstValueRef {
	protected:
		virtual void setSrcTerminal(const Terminal* terminal) = 0;
		virtual void setEffSrcBric(const Bric* bric) = 0;

	public:
		virtual const PropPath& source() const = 0;

		virtual const Terminal* srcTerminal() const = 0;

		virtual const Bric* effSrcBric() const = 0;

		virtual void connectTo(Terminal &other) final;
	};


	class ParamTerminal : public virtual Terminal, public virtual HasWritableValue {};

protected:
	class TempChangeOfTDirectory final {
	protected:
		TDirectory* m_prev = nullptr;

	public:
		void release() {
			if (m_prev != nullptr) {
				gDirectory = m_prev;
				m_prev = nullptr;
			}
		}

		TempChangeOfTDirectory() {}

		TempChangeOfTDirectory(TDirectory* newCurrentDir) {
			m_prev = newCurrentDir;
			if (m_prev != nullptr) { std::swap(m_prev, gDirectory); }
		}

		~TempChangeOfTDirectory() { release(); }
	};


	static const PropKey s_defaultInputName;
	static const PropKey s_defaultOutputName;
	static const PropKey s_bricTypeKey;

	static std::unique_ptr<Bric> createBricFromTypeName(const std::string &className);


	std::map<PropKey, BricComponent*> m_components;
	std::map<PropKey, Bric*> m_brics;
	std::map<PropKey, Terminal*> m_terminals;
	std::map<PropKey, ParamTerminal*> m_params;
	std::map<PropKey, OutputTerminal*> m_outputs;
	std::map<PropKey, InputTerminal*> m_inputs;

	std::map<PropKey, std::unique_ptr<Bric>> m_dynBrics;
	std::map<PropKey, std::string> m_dynBricClassNames;

	std::map<PropKey, std::unique_ptr<Terminal>> m_dynTerminals;

	std::unique_ptr<TDirectory> m_tDirectory;


	virtual bool isBricConfig(const PropVal& config) final;

	virtual void addDynBric(PropKey bricName, const PropVal& config);
	virtual void delDynBric(PropKey bricName);

	virtual InputTerminal* connectInputToInner(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath) final;

	virtual InputTerminal* connectInputToSiblingOrUp(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath);
	virtual InputTerminal* connectOwnInputTo(PropKey inputName, Terminal& source);

	// Must always be called for a whole set of interdependent sibling brics
	virtual void disconnectInputs() final;

	virtual void connectInputs() final;
	virtual void updateDeps() final;

	virtual void initRecursive() final;

public:
	template <typename T> class TypedTerminal
		: public virtual Terminal, public virtual HasTypedValue<T>
	{
		OutputTerminal* createMatchingDynOutput(Bric* outputBric,
			PropKey outputName, std::string outputTitle = "") override;

		InputTerminal* createMatchingDynInput(Bric* inputBric,
			PropKey inputName, std::string inputTitle = "") override;
	};

	template <typename T> class TypedOutputTerminal
		: public virtual TypedTerminal<T>, public virtual OutputTerminal, public virtual HasTypedWritableValue<T>
	{
	public:
		void applyConfig(const PropVal& config) final override
			{ if (!config.isNone()) throw std::invalid_argument("Output is not configurable"); }

		PropVal getConfig() const final override { return PropVal(); }

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


	template <typename T> class Param final
		: public virtual TypedParamTerminal<T>, public BricComponentImpl, public HasTypedPrimaryValueImpl<T>
	{
	public:
		using HasTypedPrimaryValueImpl<T>::value;
		using HasTypedPrimaryValueImpl<T>::typeInfo;

		void applyConfig(const PropVal& config) final override {
			PropVal currCfg = getConfig();
			if (currCfg.isProps() && config.isProps())
				assign_from(value().get(), currCfg.asProps() + config.asProps());
			else
				assign_from(value().get(), config);
		}

		PropVal getConfig() const final override
			{ PropVal config; assign_from(config, value().get()); return config; }

		Param<T>& operator=(const Param<T>& v) = delete;

		Param<T>& operator=(const T &v)
			{ TypedParamTerminal<T>::operator=(v); return *this; }

		Param<T>& operator=(T &&v) noexcept
			{ TypedParamTerminal<T>::operator=(std::move(v)); return *this; }

		Param<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ TypedParamTerminal<T>::operator=(std::move(v)); return *this; }


		Param(Bric *parentBric, PropKey paramName, std::string paramTitle = "", T defaultValue = T())
			: BricComponentImpl(paramName, std::move(paramTitle))
		{
			value() = std::move(defaultValue);
			setParent(parentBric);
		}

		Param(const Param &other) = delete;

		~Param() override { setParent(nullptr); }
	};


	void applyConfig(const PropVal& config) override;
	PropVal getConfig() const override;

	virtual void registerComponent(BricComponent* component);
	virtual void unregisterComponent(BricComponent* component);

	virtual bool canHaveInputs() const { return false; }
	virtual bool canHaveOutputs() const { return false; }


	virtual const Bric& getBric(PropKey bricName) const final;
	virtual Bric& getBric(PropKey bricName) final;


	virtual const Terminal& getTerminal(PropKey terminalName) const final;
	virtual Terminal& getTerminal(PropKey terminalName) final;

	virtual const Terminal& getTerminal(PropKey terminalName, const std::type_info& typeInfo) const final;
	virtual Terminal& getTerminal(PropKey terminalName, const std::type_info& typeInfo) final;


	virtual const OutputTerminal& getOutput(PropKey outputName) const final;
	virtual OutputTerminal& getOutput(PropKey outputName) final;

	virtual const OutputTerminal& getOutput(PropKey outputName, const std::type_info& typeInfo) const final;
	virtual OutputTerminal& getOutput(PropKey outputName, const std::type_info& typeInfo) final;


	virtual const InputTerminal& getInput(PropKey outputName) const final;
	virtual InputTerminal& getInput(PropKey outputName) final;

	virtual const InputTerminal& getInput(PropKey outputName, const std::type_info& typeInfo) const final;
	virtual InputTerminal& getInput(PropKey outputName, const std::type_info& typeInfo) final;


	virtual const ParamTerminal& getParam(PropKey outputName) const final;
	virtual ParamTerminal& getParam(PropKey outputName) final;

	virtual const ParamTerminal& getParam(PropKey outputName, const std::type_info& typeInfo) const final;
	virtual ParamTerminal& getParam(PropKey outputName, const std::type_info& typeInfo) final;

	virtual bool canHaveDynOutputs() const { return false; }
	virtual void addDynOutput(std::unique_ptr<OutputTerminal> terminal) final;

	virtual bool canHaveDynInputs() const { return false; }
	virtual void addDynInput(std::unique_ptr<InputTerminal> terminal) final;


	virtual TDirectory* localTDirectory() final { return m_tDirectory.get(); }
	virtual const TDirectory* localTDirectory() const final { return m_tDirectory.get(); }

	virtual std::ostream & printInfo(std::ostream &os) const;

	// Recursively initialize this bric and all brics inside it. Can only be
	// called on top brics (brics without a parent).
	virtual void initBricHierarchy() final;

	// User overload, executed, undefined if executed before or after sub-bric
	// init (possibly in parallel?):
	virtual void init() {};

	// Maybe later:
	// User overload, executed before init_parentFirst for sub-brics:
	// virtual void init_parentFirst() {};

	// Maybe later:
	// User overload, executed  after init_childrenFirst for sub-brics:
	// virtual void init_childrenFirst() {};


// Sources //

protected:
	std::vector<Bric*> m_sources;
	bool m_hasExternalSources = false;
	bool m_inputsConnected = false;

	std::atomic<size_t> m_nSourcesAvailable;

	std::atomic<size_t> m_nSourcesFinished;


	virtual Bric* addSource(Bric *source) final;

	virtual bool hasSources() const final { return ! m_sources.empty(); }
	virtual size_t nSources() const final { return m_sources.size(); }

	virtual void incNSourcesAvailable() final { atomic_fetch_add(&m_nSourcesAvailable, size_t(1)); }
	virtual void decNSourcesAvailable() final { atomic_fetch_sub(&m_nSourcesAvailable, size_t(1)); }
	virtual void clearNSourcesAvailable() final { atomic_store(&m_nSourcesAvailable, size_t(0)); }

	virtual size_t nSourcesAvailable() const final {
		size_t nAvail = atomic_load(&m_nSourcesAvailable);
		assert(nAvail <= nSources()); // Sanity check
		return nAvail;
	}

	virtual bool externalSourcesAvailable() const final { return hasExternalSources() && execCounter() == 0; }

	virtual bool allSourcesAvailable() const final
		{ return nSourcesAvailable() == nSources() || externalSourcesAvailable(); }

	virtual bool anySourceAvailable() const final
		{ return nSourcesAvailable() > 0 || externalSourcesAvailable(); }


	virtual void incSourcesFinished() final { atomic_fetch_add(&m_nSourcesFinished, size_t(1)); }

	virtual size_t nSourcesFinished() const final {
		size_t nFinished = atomic_load(&m_nSourcesFinished);
		assert(nFinished <= nSources()); // Sanity check
		return nFinished;
	}

	virtual bool allSourcesFinished() const final { return nSourcesFinished() == nSources(); }

	virtual bool hasExternalSources() const final { return m_hasExternalSources; }

public:
	virtual const std::vector<Bric*>& sources() final { return m_sources; }


// Dests //

protected:
	std::vector<Bric*> m_dests;

	std::atomic<size_t> m_nDestsReadyForInput;

	size_t m_outputCounter;


	virtual bool hasDests() const final { return ! m_dests.empty(); }
	virtual size_t nDests() const final { return m_dests.size(); }

	virtual void incNDestsReadyForInput() final { atomic_fetch_add(&m_nDestsReadyForInput, size_t(1)); }
	virtual void clearNDestsReadyForInput() final { atomic_store(&m_nDestsReadyForInput, size_t(0)); }

	virtual size_t nDestsReadyForInput() const final {
		size_t nReady = atomic_load(&m_nDestsReadyForInput);
		assert(nReady <= nDests()); // Sanity check
		return nReady;
	}

	virtual bool allDestsReadyForInput() const final { return nDestsReadyForInput() == nDests(); }


	virtual void announceNewOutput() final {
		for (auto &dest: m_dests) dest->incNSourcesAvailable();
		clearNDestsReadyForInput();
		++m_outputCounter;
	}

	virtual void setExecFinished() final {
		assert(m_execFinished == false); // Sanity check
		dbrx_log_trace("Execution of bric %s finished", absolutePath());
		m_execFinished = true;
		for (auto &dest: m_dests) dest->incSourcesFinished();
	}

public:
	virtual const std::vector<Bric*>& dests() final { return m_dests; }


// Execution //

protected:
	bool m_execFinished = false;
	size_t m_execCounter = 0;


	// See nextExecStep for guarantees on behaviour and return value.
	virtual bool nextExecStepImpl() = 0;

	virtual void setOutputsToErrorState() final {
		dbrx_log_info("Due to an error, setting outputs of bric \"%s\" to default values", absolutePath());
		for (auto& output: m_outputs) output.second->value().setToDefault();
	}


public:
	// Must always be called for a whole set of interdependent sibling brics
	virtual void resetExec() {
		dbrx_log_trace("Resetting execution for bric \"%s\"", absolutePath());
		atomic_store(&m_nSourcesAvailable, size_t(0));
		atomic_store(&m_nSourcesFinished, size_t(0));
		atomic_store(&m_nDestsReadyForInput, nDests());
		m_execFinished = false;
		m_execCounter = false;
	}

	// Returns true if execution is finished or new output was produced.
	// A repeated call, before any changes in higher/lower processing layers,
	// is guaranteed not to change the state of the bric and to return
	// true if bric execution is finished and false if not.
	virtual bool nextExecStep() final {
		if (!execFinished()) {
			TempChangeOfTDirectory tDirChange(localTDirectory());
			bool result = nextExecStepImpl();
			++m_execCounter;
			return result;
		} else return true;
	}

	virtual bool execFinished() const final { return m_execFinished; }

	virtual size_t execCounter() const final { return m_execCounter; }


public:
	friend class BricImpl;
	friend class BricWithInputs;
	friend class BricWithOutputs;
	friend class SyncedInputBric;
	friend class AsyncInputBric;
	friend class ProcessingBric;
	friend class ImportBric;
	friend class TransformBric;
	friend class MapperBric;
	friend class AbstractReducerBric;
	friend class ReducerBric;
	friend class AsyncReducerBric;
};



class BricImpl: public virtual Bric, public BricComponentImpl {
public:
	BricImpl() {}
	BricImpl(PropKey bricName): BricComponentImpl(bricName) {}
	BricImpl(Bric *parentBric, PropKey bricName, std::string bricTitle = "")
		: BricComponentImpl(bricName, bricTitle) { setParent(parentBric); }

	~BricImpl() override { setParent(nullptr); }
};



inline bool BricComponent::isInside(const Bric& other) const
	{ return hasParent() && (&parent() == &other || parent().isInside(other)); }


inline size_t BricComponent::hierarchyLevel() const
	{ return hasParent() ? parent().hierarchyLevel() + 1 : 0; }



inline void BricComponentImpl::setParent(Bric *parentBric) {
	if (m_parent != nullptr) m_parent->unregisterComponent(this);
	if (parentBric != nullptr) {
		m_parent = parentBric;
		m_parent->registerComponent(this);
	}
}



class BricWithOutputs: public virtual Bric  {
public:
	template <typename T> class Output
		: public virtual TypedOutputTerminal<T>, public BricComponentImpl, public HasTypedPrimaryValueImpl<T>
	{
	public:
		using HasTypedPrimaryValueImpl<T>::value;
		using HasTypedPrimaryValueImpl<T>::typeInfo;

		Output<T>& operator=(const Output<T>& v) = delete;

		Output<T>& operator=(const T &v)
			{ TypedOutputTerminal<T>::operator=(v); return *this; }

		Output<T>& operator=(T &&v) noexcept
			{ TypedOutputTerminal<T>::operator=(std::move(v)); return *this; }

		Output<T>& operator=(std::unique_ptr<T> &&v) noexcept
			{ TypedOutputTerminal<T>::operator=(std::move(v)); return *this; }


		Output(BricWithOutputs *parentBric, PropKey outputName = PropKey(), std::string outputTitle = "")
			: BricComponentImpl(outputName, std::move(outputTitle))
		{
			if (name() == PropKey()) m_key = s_defaultOutputName;
			setParent(parentBric);
		}

		template<typename U> Output(BricWithOutputs *parentBric, PropKey outputName,
			std::string outputTitle, U&& defaultValue) : Output(parentBric, outputName, outputTitle)
			{ value() = std::forward<U>(defaultValue); }

		Output(const Output &other) = delete;

		~Output() override { setParent(nullptr); }
	};

	bool canHaveOutputs() const override { return true; }
};



class BricWithInputs: public virtual Bric  {
public:
	template <typename T> class Input
		: public virtual TypedInputTerminal<T>, public BricComponentImpl, public HasTypedConstValueRefImpl<T>
	{
	protected:
		PropPath m_source;
		const Bric* m_effSrcBric;
		const Terminal *m_srcTerminal;

		virtual void setSrcTerminal(const Terminal* terminal) final { m_srcTerminal = terminal; }
		virtual void setEffSrcBric(const Bric* bric) final { m_effSrcBric = bric; }

	public:
		using HasTypedConstValueRefImpl<T>::value;
		using HasTypedConstValueRefImpl<T>::typeInfo;

		void applyConfig(const PropVal& config) override { m_source = config; }
		PropVal getConfig() const override { return m_source; }

		virtual const PropPath& source() const final { return m_source; }

		virtual const Terminal* srcTerminal() const final { return m_srcTerminal; }

		virtual const Bric* effSrcBric() const final { return m_effSrcBric; }

		Input(BricWithInputs *parentBric, PropKey inputName = PropKey(), std::string inputTitle = "")
			: BricComponentImpl(inputName, std::move(inputTitle))
		{
			if (name() == PropKey()) m_key = s_defaultInputName;
			setParent(parentBric);
		}

		Input(const Input &other) = delete;

		~Input() override { setParent(nullptr); }
	};

protected:
	virtual void tryProcessInput() final {
		try{ processInput(); }
		catch(const std::exception &e) {
			dbrx_log_error("Processing input failed in bric \"%s\": %s", absolutePath(), e.what());
			setOutputsToErrorState();
			setExecFinished();
		}
	}

public:
	bool canHaveInputs() const final override { return true; }

	// User overload. Allowed to change output values.
	virtual void processInput() = 0;
};



template <typename T> Bric::OutputTerminal* Bric::TypedTerminal<T>::createMatchingDynOutput (
	Bric* outputBric, PropKey outputName, std::string outputTitle
) {
	std::unique_ptr<Bric::OutputTerminal> terminal (
		new typename BricWithOutputs::Output<T>(nullptr, outputName, outputTitle)
	);
	OutputTerminal* termPtr = terminal.get();
	outputBric->addDynOutput(std::move(terminal));
	return termPtr;
}


template <typename T> Bric::InputTerminal* Bric::TypedTerminal<T>::createMatchingDynInput (
	Bric* inputBric, PropKey inputName, std::string inputTitle
) {
	std::unique_ptr<Bric::InputTerminal> terminal (
		new typename BricWithInputs::Input<T>(nullptr, inputName, inputTitle)
	);
	InputTerminal* termPtr = terminal.get();
	inputBric->addDynInput(std::move(terminal));
	return termPtr;
}



class SyncedInputBric: public virtual BricWithInputs {
protected:
	bool m_announcedReadyForInput = false;

	virtual void announceReadyForInput() final {
		if (!m_announcedReadyForInput && !allSourcesFinished()) {
			for (auto &source: m_sources) source->incNDestsReadyForInput();
			m_announcedReadyForInput = true;
		}
	}

	virtual void consumeInput() final {
		clearNSourcesAvailable();
		m_announcedReadyForInput = false;
	}

public:
	void resetExec() override {
		Bric::resetExec();
		m_announcedReadyForInput = true;
	}
};



class AsyncInputBric: public virtual BricWithInputs {};


class ProcessingBric: public virtual BricWithInputs, public virtual BricWithOutputs {};



class ImportBric: public virtual BricWithOutputs, public BricImpl {
protected:
	bool m_importDone = false;

public:
	// User overload. Allowed to change output values.
	virtual void import() = 0;

	bool nextExecStepImpl() override {
		if (!m_importDone) {
			dbrx_log_trace("Importer %s, running import", absolutePath());

			try{ import(); }
			catch(const std::exception &e) {
				dbrx_log_error("Running import failed in bric \"%s\": %s", absolutePath(), e.what());
				setOutputsToErrorState();
			}

			announceNewOutput();
			setExecFinished();

			// Import only once in the entire lifetime of the bric:
			m_importDone = true;
		}
		return true;
	}

public:
	using BricImpl::BricImpl;
};



// Is an export bric a useful concept? Maybe add later, if there is a need.
// class ExportBric: public virtual BricWithInputs { }



class TransformBric: public virtual ProcessingBric, public virtual SyncedInputBric, public BricImpl {
protected:
	bool nextExecStepImpl() override {
		bool producedOutput = false;

		if (allDestsReadyForInput()) {
			announceReadyForInput();

			if (anySourceAvailable()) {
				consumeInput();
				tryProcessInput();
				if (hasDests()) announceNewOutput();
				else announceReadyForInput();
				producedOutput = true;
			}
		}

		if (allSourcesFinished()) setExecFinished();

		return producedOutput || execFinished();
	}

public:
	using BricImpl::BricImpl;
};



class MapperBric: public virtual ProcessingBric, public virtual SyncedInputBric, public BricImpl {
protected:
	bool m_readyForNextOutput = false;

	bool nextExecStepImpl() override {
		bool producedOutput = false;

		if (allDestsReadyForInput()) {
			if (! m_readyForNextOutput) {
				announceReadyForInput();

				if (anySourceAvailable()) {
					consumeInput();
					tryProcessInput();
					m_readyForNextOutput = true;
				}
			}

			if (m_readyForNextOutput) {
				try{ producedOutput = nextOutput(); }
				catch(const std::exception &e) {
					dbrx_log_error("Producing next output failed in bric \"%s\": %s", absolutePath(), e.what());
					setOutputsToErrorState();
				}

				if (producedOutput) {
					if (hasDests()) announceNewOutput();
					else announceReadyForInput();
					m_readyForNextOutput = true;
				} else {
					announceReadyForInput();
					m_readyForNextOutput = false;
				}
			}

			if (!producedOutput && allSourcesFinished()) setExecFinished();
		}

		return producedOutput || execFinished();
	}

public:
	// User overload. Allowed to change output values.
	virtual bool nextOutput() = 0;

	using BricImpl::BricImpl;
};



class AbstractReducerBric: public virtual ProcessingBric {
protected:
	bool m_reductionStarted = false;

	virtual void beginReduction() final {
		try {
			newReduction();
		}
		catch(const std::exception &e) {
			dbrx_log_error("Initialization of reduction failed in bric \"%s\": %s", absolutePath(), e.what());
			setOutputsToErrorState();
			setExecFinished();
		}

		m_reductionStarted = true;
	}

	virtual bool reductionStarted() const final { return m_reductionStarted; }

	virtual void endReduction() final {
		try { finalizeReduction(); }
		catch(const std::exception &e) {
			dbrx_log_error("Finalization of reduction failed in bric \"%s\": %s", absolutePath(), e.what());
			setOutputsToErrorState();
			setExecFinished();
		}

		announceNewOutput();
		setExecFinished();
	}

public:
	// User overload. Allowed to change output values.
	virtual void newReduction() {}

	// User overload. Allowed to change output values.
	virtual void finalizeReduction() {}
};



class ReducerBric: public virtual AbstractReducerBric, public virtual SyncedInputBric, public BricImpl {
protected:
	void resetExec() override {
		SyncedInputBric::resetExec();
		m_reductionStarted = false;
	}

	bool nextExecStepImpl() override {
		if (m_reductionStarted == true) assert(allDestsReadyForInput()); // Sanity check

		if (allDestsReadyForInput()) {
			if (! m_reductionStarted) {
				beginReduction();
				if (execFinished()) return true;
			}

			announceReadyForInput();

			if (anySourceAvailable()) {
				consumeInput();
				tryProcessInput();
				announceReadyForInput();
			}

			if (allSourcesFinished()) endReduction();
		}

		return execFinished();
	}

public:
	using BricImpl::BricImpl;
};



class AsyncReducerBric: public virtual AbstractReducerBric, public virtual AsyncInputBric, public BricImpl {
protected:
	std::vector<size_t> m_inputCounter;

	void resetExec() override {
		ProcessingBric::resetExec();
		m_reductionStarted = false;
		m_inputCounter.clear();
		m_inputCounter.resize(m_sources.size());
	}

	bool nextExecStepImpl() override {
		if (m_reductionStarted == true) assert(allDestsReadyForInput()); // Sanity check

		if (allDestsReadyForInput()) {
			if (! m_reductionStarted) {
				beginReduction();
				if (execFinished()) return true;
			}

			bool gotSiblingInput = false;
			if (anySourceAvailable()) {
				tryProcessInput();
				for (size_t i = 0; i < m_sources.size(); ++i) {
					auto &source = m_sources[i];
					if (m_inputCounter[i] < source->m_outputCounter) {
						decNSourcesAvailable();
						m_inputCounter[i] = source->m_outputCounter;
						source->incNDestsReadyForInput();
						gotSiblingInput = true;
					}
				}
				assert(gotSiblingInput || externalSourcesAvailable()); // Sanity check
			}

			if (allSourcesFinished()) endReduction();
		}

		return execFinished();
	}
public:
	using BricImpl::BricImpl;
};



class TerminalGroup: public virtual Bric {
protected:
	bool nextExecStepImpl() override {
		// Terminal groups don't do anything when executed
		setExecFinished();
		return false;
	}
};



class OutputTerminalGroup: public virtual TerminalGroup, public virtual BricWithOutputs {
};


class OutputGroup: public virtual OutputTerminalGroup, public BricImpl {
	using BricImpl::BricImpl;
};


class DynOutputGroup: public virtual OutputTerminalGroup, public BricImpl {
public:
	bool canHaveDynOutputs() const final override { return true; }
	using BricImpl::BricImpl;
};



class InputTerminalGroup: public virtual TerminalGroup, public virtual BricWithInputs {
public:
	void processInput() override {}
};


class InputGroup: public virtual InputTerminalGroup, public BricImpl {
	using BricImpl::BricImpl;
};


class DynInputGroup: public virtual InputTerminalGroup, public BricImpl {
public:
	bool canHaveDynInputs() const { return true; }
	using BricImpl::BricImpl;
};


} // namespace dbrx


#endif // DBRX_BRIC_H
