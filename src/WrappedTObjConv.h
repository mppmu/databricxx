// Copyright (C) 2015 Oliver Schulz <oschulz@mpp.mpg.de>

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


#ifndef DBRX_WRAPPEDTOBJCONV_H
#define DBRX_WRAPPEDTOBJCONV_H

#include "Bric.h"
#include "WrappedTObj.h"


namespace dbrx {


class WrappedTObjConv : public TransformBric {
public:
	using TransformBric::TransformBric;
};



template<typename T> class TObjWrapper final: public WrappedTObjConv {
public:
	Input<T> input{this};
	Output<WrappedTObj<T>> output{this};

	void processInput() override {
		output = WrappedTObj<T>(std::unique_ptr<T>(dynamic_cast<T*>(input->Clone())));
	}

	using WrappedTObjConv::WrappedTObjConv;
};



template<typename T> class TObjUnwrapper final: public WrappedTObjConv {
public:
	Input<WrappedTObj<T>> input{this};
	Output<T> output{this};

	void processInput() override {
		output = std::unique_ptr<T>(dynamic_cast<T*>(input->get().Clone()));
	}

	using WrappedTObjConv::WrappedTObjConv;
};


} // namespace dbrx

#endif // DBRX_WRAPPEDTOBJCONV_H
