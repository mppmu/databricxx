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


#ifndef DBRX_PROPVALBRICS_H
#define DBRX_PROPVALBRICS_H

#include <functional>

#include "Bric.h"


namespace dbrx {


class FromJSON: public TransformBric {
public:
	Input<std::string> input{this};
	Output<PropVal> output{this};

	void processInput() { output = PropVal::fromJSON(input); }

	using TransformBric::TransformBric;
};



class ToJSON: public TransformBric {
public:
	Input<PropVal> input{this};
	Output<std::string> output{this};

	void processInput() { output = input->toJSON(); }

	using TransformBric::TransformBric;
};



class PropsSplitter: public TransformBric {
public:
	class ContentGroup final: public DynOutputGroup {
	protected:
		InputTerminal* connectInputToInner(Bric &bric, PropKey inputName, PropPath::Fragment sourcePath) override;

		ContentGroup& subGroup(PropKey name);

	public:
		virtual void splitPropVal(const PropVal& from) final;

		using DynOutputGroup::DynOutputGroup;
	};

	Input<PropVal> input{this};

	ContentGroup output{this, "output"};

	void processInput() override;

	using TransformBric::TransformBric;
};



class PropsBuilder: public TransformBric {
public:
	class ContentGroup final: public DynInputGroup {
	protected:
		PropsBuilder *m_builder = 0;
		Props m_config;

		void disconnectInputs() override;
		void connectInputs() override;

		virtual void createAndConnectInputs(const Props& config) final;
		virtual void createAndConnectInputs(Bric &sourceBric, const PropPath &sourceBricPath) final;
		virtual void createAndConnectInput(PropKey sourceName, const PropPath &sourcePath) final;

		ContentGroup& subGroup(PropKey name);

	public:
		void applyConfig(const PropVal& config) final override;
		PropVal getConfig() const final override;

		virtual PropVal createPropVal() final;

		void processInput() final {}

		ContentGroup() {}

		ContentGroup(PropsBuilder *builder, Bric *parentBric, PropKey groupName)
			: DynInputGroup(parentBric, groupName), m_builder(builder) {}
	};

	ContentGroup input{this, this, "input"};

	Output<PropVal> output{this};

	void processInput() override;

	using TransformBric::TransformBric;
};


} // namespace dbrx

#endif // DBRX_PROPVALBRICS_H
