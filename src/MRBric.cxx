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


#include "MRBric.h"

#include <iostream>

#include "format.h"
#include "funcprog.h"


using namespace std;


namespace dbrx {


std::unordered_map<Bric*, size_t> MRBric::calcBricGraphLayers(const std::vector<Bric*> &brics) {
	// Translation between bric lingo and graph lingo

	using Node = Bric*;
	using Nodes = std::vector<Node>;
	using Layer = size_t;

	const Nodes &nodes = brics;

	auto deps = [&](Node node) -> const Nodes& { return node->sources(); };

	dbrx_log_debug("Topological sort of %s nodes in execution graph"_format(brics.size()));
	for (Node node: nodes) {
		dbrx_log_trace("Node %s, deps: %s"_format(
			node->name(),
			mkstring(mapped(deps(node), [](Node dep){ return dep->name(); }), ", ")
		));
	}


	// Topological sorting by depth-first search

	enum class State : int { UNVISITED = 0, VISITING = 1, VISITED = 2 };

	struct NodeInfo {
		Layer layer;
		State state;
	};
	std::unordered_map<Node, NodeInfo> nodeInfo;

	auto layer = [&](Node node) -> Layer& { return nodeInfo[node].layer; };
	auto state = [&](Node node) -> State& { return nodeInfo[node].state; };

	auto compareLayer = [&](Node a, Node b) { return layer(a) < layer(b); };

	Nodes nodesToVisit = nodes;

	while (! nodesToVisit.empty()) {
		// dbrx_log_trace("Nodes to visit: %s"_format(
		//	mkstring(mapped(nodesToVisit, [&](Node node){ return "%s (%s)"_format(node->name(), int(state(node))).c_str(); }), ", ")
		// ));

		Node node = nodesToVisit.back();
		dbrx_log_trace("Visiting node %s, current state %s"_format(node->name(), int(state(node))));
		auto& nodeDeps = deps(node);

		if (state(node) == State::VISITED) {
			// Visited node instance may be left on stack from initialization
			nodesToVisit.pop_back();
		} else if (nodeDeps.empty()) {
			// Top-level node
			assert(state(node) != State::VISITING); // Sanity check
			layer(node) = 0;
			state(node) = State::VISITED;
			dbrx_log_trace("Assigned node %s to layer %s"_format(node->name(), layer(node)));
			assert(node == nodesToVisit.back()); // Sanity check
			nodesToVisit.pop_back();
		} else {
			// Not a top-level node

			bool allDepsVisited = true;
			for (Node dep: nodeDeps) {
				if (state(dep) == State::VISITING) {
					// Detected circle in graph
					throw invalid_argument("Not a DAG");
				} else if (state(dep) == State::UNVISITED) {
					allDepsVisited = false;
					nodesToVisit.push_back(dep);
				}
			}

			if (allDepsVisited) {
				layer(node) = 1 + layer(*max_element(nodeDeps.begin(), nodeDeps.end(), compareLayer));
				state(node) = State::VISITED;
				dbrx_log_trace("Assigned node %s to layer %s"_format(node->name(), layer(node)));
				assert(node == nodesToVisit.back()); // Sanity check
				nodesToVisit.pop_back();
			} else if (state(node) != State::VISITING) {
				// Keeps node on nodesToVisit, all deps should have been
				// visited the next time we encounter it
				state(node) = State::VISITING;
			} else {
				// Already visiting this node but still not all deps visited,
				// this shouldn't happen.
				assert(false);
				throw logic_error("Internal error in topological sort");
			}
		}
	}


	// Output node to layer map

	std::unordered_map<Node, Layer> result;
	for (Node node: nodes) result[node] = layer(node);
	return result;
}


void MRBric::init() {
	std::vector<Bric*> execBrics;
	execBrics.reserve(m_brics.size());
	for (auto &entry: m_brics) execBrics.push_back(entry.second);

	dbrx_log_debug("Initializing processing layers for bric \"%s\"", absolutePath());
	clear();

	auto gLayers = calcBricGraphLayers(execBrics);

	using BL = decltype(*gLayers.begin());
	size_t nLayers = 1 + max_element( gLayers.begin(), gLayers.end(),
		[](const BL& a, const BL&b){ return a.second < b.second; } )->second;

	dbrx_log_debug("Creating %s execution layers in bric \"%s\"", nLayers, absolutePath());
	m_execLayers.resize(nLayers);

	for (Bric *bric: execBrics) m_execLayers.at(gLayers.at(bric)).brics.push_back(bric);
	for (auto& layer: m_execLayers) sortBricsByName(layer.brics);

	for (size_t i = 0; i < m_execLayers.size(); ++i) {
		dbrx_log_debug("Exec layer %s: %s"_format(
			i,
			mkstring(mapped(m_execLayers[i].brics, [&](Bric* bric){ return bric->name(); }), ", ")
		));
	}

	resetExec();
}


bool MRBric::processingStep() {
	assert(m_currentLayer >= m_topLayer); // Sanity check
	assert(m_currentLayer <= m_bottomLayer); // Sanity check

	if (!m_innerExecFinished) {
		bool execResult = m_currentLayer->nextExecStep();
		if (m_currentLayer->execFinished()) m_topLayer = m_currentLayer;

		if (m_currentLayer == m_bottomLayer) {
			m_runningDown = false;
			if (m_bottomLayer->execFinished()) {
				dbrx_log_trace("Processing finished for bric \"%s\" (all inner brics in bottom exec layer finished)", absolutePath());
				m_innerExecFinished = true;
			}
			else moveUpOneLayer();
		} else {
			if (m_runningDown) {
				moveDownOneLayer();
			} else {
				if (execResult) {
					// Layer produced output or all brics in layer are finished
					m_runningDown = true;
					moveDownOneLayer();
				} else {
					// No output from layer and layer not finished
					m_runningDown = false;
					if (m_currentLayer != m_topLayer) moveUpOneLayer();
					else {
						m_innerExecFinished = true;
						throw std::logic_error("Internal error during processing of bric \"%s\", top exec layer has no output but is not finished"_format(absolutePath()));
					} 
				}
			}
		} 
	}
	return m_innerExecFinished;
}


void MRBric::resetExec() {
	dbrx_log_debug("Resetting processing for MR bric \"%s\"", absolutePath());
	SyncedInputBric::resetExec();

	if (!m_execLayers.empty()) {
		m_topLayer = m_execLayers.begin();
		m_currentLayer = m_topLayer;
		m_bottomLayer = m_execLayers.end() - 1;
		m_innerExecFinished = false;
		m_runningDown = true;
		for (auto &layer: m_execLayers) layer.resetExec();
	} else {
		m_innerExecFinished = true;
	}
}


void MRBric::processInput() {
	while(!m_innerExecFinished) processingStep();
}


} // namespace dbrx
