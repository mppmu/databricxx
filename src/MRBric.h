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


#ifndef DBRX_MRBRIC_H
#define DBRX_MRBRIC_H

#include <stdexcept>
#include <unordered_map>

#include "logging.h"
#include "Bric.h"


namespace dbrx {


class MRBric: public TransformBric {
protected:
	struct ExecLayer final {
		std::vector<Bric*> brics;
		bool m_execFinished = false;

		void resetExec() {
			m_execFinished = false;
			for (Bric *bric: brics) bric->resetExec();
		}

		bool execFinished() const { return m_execFinished; }

		bool nextExecStep() {
			if (!m_execFinished) {
				bool allBricExecsTrue = true;
				bool allBricsFinished = true;
				for (Bric* bric: brics) {
					allBricExecsTrue &= bric->nextExecStep();
					allBricsFinished &= bric->execFinished();
				}
				m_execFinished = allBricsFinished;
				return allBricExecsTrue || m_execFinished;
			} else return true;
		}
	};


	static std::unordered_map<Bric*, size_t> calcBricGraphLayers(const std::vector<Bric*> &brics);

	static void sortBricsByName(std::vector<Bric*> &brics) {
		using namespace std;
		sort(brics.begin(), brics.end(), [](Bric *a, Bric *b) { return a->name() < b->name(); });
	}


	bool m_innerExecFinished = false;
	bool m_runningDown = true;

	std::vector<ExecLayer> m_execLayers;

	using LIter = decltype(m_execLayers.begin());
	LIter m_topLayer;
	LIter m_currentLayer;
	LIter m_bottomLayer;

	virtual size_t currentLayerNo() final { return  m_currentLayer - m_execLayers.begin(); }

	virtual void moveUpOneLayer() final {
		--m_currentLayer;
		dbrx_log_trace("Moving up to exec layer %s in bric \"%s\"",currentLayerNo(), absolutePath());
	}

	virtual void moveDownOneLayer() final {
		++m_currentLayer;
		dbrx_log_trace("Moving down to exec layer %s in bric \"%s\"", currentLayerNo(), absolutePath());
	}


	void init() override;

	virtual bool processingStep() final;

public:
	void resetExec() override;

	void processInput() override;

	virtual void clear() final { m_execLayers.clear(); }

	virtual void run() final;

	using TransformBric::TransformBric;
};


} // namespace dbrx

#endif // DBRX_MRBRIC_H
