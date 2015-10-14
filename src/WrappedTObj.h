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


#ifndef DBRX_WRAPPEDTOBJ_H
#define DBRX_WRAPPEDTOBJ_H

#include <memory>

#include <TClass.h>
#include <TObject.h>
#include <TDirectory.h>


#include <TH1.h>
#include <TH1I.h>
#include <TH1F.h>
#include <TH1D.h>

#include <TH2.h>
#include <TH2I.h>
#include <TH2F.h>
#include <TH2D.h>

#include <TGraph.h>

#include <TF1.h>


namespace dbrx {


class AbstractWrappedTObj {
protected:
	static void releaseFromTDirIfAutoAdded(TObject *obj);

public:
	virtual const std::type_info& typeInfo() const = 0;

	virtual bool empty() const = 0;

	virtual const TObject& get() const = 0;
	virtual TObject& get() = 0;

	virtual const TObject* getPtr() const = 0;
	virtual TObject* getPtr() = 0;

	virtual bool canWrapTObj(const TObject* obj) = 0;

	virtual void wrapTObj(std::unique_ptr<TObject>&& obj) = 0;

	virtual std::unique_ptr<TObject> releaseTObj() = 0;

	virtual ~AbstractWrappedTObj() {}
};


template<typename T/*, typename = decltype(std::unique_ptr<TObject*>(std::declval<T*>))*/>
class WrappedTObj: public AbstractWrappedTObj {
protected:
	std::unique_ptr<T> m_wrapped;

public:
	template<typename ...Args>
	static WrappedTObj<T> create(Args&&... args) {
		T* newObj = new T(std::forward<Args>(args)...);
		releaseFromTDirIfAutoAdded(newObj);
		return WrappedTObj<T>(std::unique_ptr<T>(newObj));
	}


	const std::type_info& typeInfo() const final override { return typeid(T); }

	bool empty() const final override { return m_wrapped.get() == nullptr; }

	virtual std::unique_ptr<T> release() final {
		std::unique_ptr<T> result;
		std::swap(m_wrapped, result);
		return result;
	}

	const T& get() const final override { return *m_wrapped; }
	T& get() final override { return *m_wrapped; }

	const T* getPtr() const final override { return m_wrapped.get(); }
	T* getPtr() final override { return m_wrapped.get(); }


	bool canWrapTObj(const TObject* obj) final override {
		return dynamic_cast<const T*>(obj) != nullptr;
	}

	void wrapTObj(std::unique_ptr<TObject>&& obj) final override {
		if (canWrapTObj(obj.get())) *this = std::unique_ptr<T>(dynamic_cast<T*>(obj.release()));
		else throw std::bad_cast();
	}

	virtual std::unique_ptr<TObject> releaseTObj() final {
		return std::unique_ptr<TObject>(m_wrapped.release());
	}


	virtual const T* operator->() const final { return getPtr(); }
	virtual T* operator->() final { return getPtr(); }

	virtual const T& operator*() const final { return *getPtr(); }
	virtual T& operator*() final { return *getPtr(); }


	virtual WrappedTObj<T>& operator=(const WrappedTObj<T>& that) final { return *this = that.get(); }

	virtual WrappedTObj<T>& operator=(WrappedTObj<T>&& that) final {
		m_wrapped = that.release();
		return *this;
	}

	template<typename U, typename = decltype(std::unique_ptr<T>(std::declval<U*>()))>
	WrappedTObj<T>& operator=(const U& obj) {
		U* newObj = dynamic_cast<U*>(obj.Clone());
		releaseFromTDirIfAutoAdded(newObj);
		m_wrapped = std::unique_ptr<T>(newObj);
		return *this;
	}

	template<typename U, typename = decltype(std::unique_ptr<T>(std::declval<U*>()))>
	WrappedTObj<T>& operator=(const WrappedTObj<U>& that) { return *this = that.get(); }

	template<typename U, typename = decltype(std::unique_ptr<T>(std::declval<U*>()))>
	WrappedTObj<T>& operator=(WrappedTObj<U>&& that) {
		m_wrapped = that.release();
		return *this;
	}


	WrappedTObj() {}

	WrappedTObj(const WrappedTObj<T> &that) { *this = that; }

	WrappedTObj(WrappedTObj<T>&& that) : m_wrapped(std::move(that.release())) {}

	template<typename U, typename = decltype(std::unique_ptr<T>(std::declval<U*>()))>
	WrappedTObj(const U& obj) { *this = obj; }

	template<typename U, typename = decltype(std::unique_ptr<T>(std::declval<U*>()))>
	WrappedTObj(const WrappedTObj<U> &that) { *this = that; }

	template<typename U, typename = decltype(std::unique_ptr<T>(std::declval<U*>()))>
	WrappedTObj(WrappedTObj<U>&& that) : m_wrapped(std::move(that.release())) {}

	template<typename U, typename = decltype(std::unique_ptr<T>(std::declval<U*>()))>
	WrappedTObj(std::unique_ptr<U>&& ptr) : m_wrapped(std::move(ptr)) {}

	virtual ~WrappedTObj() {}
};


using WTH1 = WrappedTObj<TH1>;
using WTH1I = WrappedTObj<TH1I>;
using WTH1F = WrappedTObj<TH1F>;
using WTH1D = WrappedTObj<TH1D>;

using WTH2 = WrappedTObj<TH2>;
using WTH2I = WrappedTObj<TH2I>;
using WTH2F = WrappedTObj<TH2F>;
using WTH2D = WrappedTObj<TH2D>;

using WTGraph = WrappedTObj<TGraph>;


} // namespace dbrx

#endif // DBRX_WRAPPEDTOBJ_H
