/**********************************************************\
Original Author: Richard Bateman (taxilian)

Created:    Sep 14, 2010
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#ifndef H_FB_CROSSTHREADCALL
#define H_FB_CROSSTHREADCALL

#include <vector>
#include <string>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "APITypes.h"
#include "JSObject.h"
#include "BrowserHost.h"
#include <boost/weak_ptr.hpp>

namespace FB {

    class FunctorCall
    {
        virtual void call() = 0;
        friend class CrossThreadCall;
    };

    template<class Functor, class C, class RT = typename Functor::result_type>
    class FunctorCallImpl : public FunctorCall
    {
    public:
        FunctorCallImpl(const boost::shared_ptr<C> &cls, const Functor &func) : ref(true), reference(cls), func(func) { }
        FunctorCallImpl(const Functor &func) : ref(false), func(func) {}
        void call() {
            boost::shared_ptr<C> tmp;
            if (ref) tmp = reference.lock();
            if (!ref || tmp)
                retVal = func();
        }
        RT getResult() { return retVal; }

    protected:
        bool ref;
        boost::weak_ptr<C> reference;
        Functor func;
        RT retVal;
    };

    template<class Functor, class C>
    class FunctorCallImpl<Functor, C, void> : public FunctorCall
    {
    public:
        FunctorCallImpl(const boost::shared_ptr<C> &cls, const Functor &func) : func(func), ref(true), reference(cls) { }
        FunctorCallImpl(const Functor &func) : func(func), ref(false) {}
        void call() {
            boost::shared_ptr<C> tmp;
            if (ref) tmp = reference.lock();
            if (!ref || tmp)
                func();
        }

    protected:
        Functor func;
        bool ref;
        boost::weak_ptr<C> reference;
    };

    class CrossThreadCall
    {
    public:
        template<class Functor>
        static typename Functor::result_type syncCall(const FB::BrowserHostPtr &host, Functor func);

        template<class Functor>
        static typename Functor::result_type syncCallHelper(const FB::BrowserHostPtr &host, Functor func, boost::true_type /* is void */);
        template<class Functor>
        static typename Functor::result_type syncCallHelper(const FB::BrowserHostPtr &host, Functor func, boost::false_type /* is void */);

        template<class C, class Functor>
        static void asyncCall(const FB::BrowserHostPtr &host, boost::shared_ptr<C> obj, Functor func);

    protected:
        CrossThreadCall(FunctorCall* funct) : funct(funct), m_returned(false) { }

        static void asyncCallbackFunctor(void *userData);
        static void syncCallbackFunctor(void *userData);

        FunctorCall* funct;
        variant m_result;
        bool m_returned;

        boost::condition_variable m_cond;
        boost::mutex m_mutex;
    };

    template<class C, class Functor>
    void CrossThreadCall::asyncCall(const FB::BrowserHostPtr &host, boost::shared_ptr<C> obj, Functor func)
    {
        FunctorCallImpl<Functor, C> *funct = new FunctorCallImpl<Functor, C>(obj, func);
        CrossThreadCall *call = new CrossThreadCall(funct);
        host->ScheduleAsyncCall(&CrossThreadCall::asyncCallbackFunctor, call);
    }

    template<class Functor>
    typename Functor::result_type CrossThreadCall::syncCall(const FB::BrowserHostPtr &host, Functor func)
    {
        typedef boost::is_same<void, typename Functor::result_type> is_void;
        return syncCallHelper(host, func, is_void());
    }

    template <class Functor>
    typename Functor::result_type CrossThreadCall::syncCallHelper(const FB::BrowserHostPtr &host, Functor func, boost::true_type /* return void */)
    {
        FB::variant varResult;

        FunctorCallImpl<Functor, bool> *funct = new FunctorCallImpl<Functor, bool>(func);
        if (!host->isMainThread())
        {
            CrossThreadCall *call = new CrossThreadCall(funct);
            {
                boost::unique_lock<boost::mutex> lock(call->m_mutex);
                host->ScheduleAsyncCall(&CrossThreadCall::syncCallbackFunctor, call);

                while (!call->m_returned) {
                    call->m_cond.wait(lock);
                }
                varResult = call->m_result;
            }
            delete call;
        } else {
            funct->call();
        }
        delete funct;
        if (varResult.get_type() == typeid(FB::script_error)) {
            throw FB::script_error(varResult.cast<const FB::script_error>().what());
        }
    }

    template <class Functor>
    typename Functor::result_type CrossThreadCall::syncCallHelper(const FB::BrowserHostPtr &host, Functor func, boost::false_type /* return not void */)
    {
        typename Functor::result_type result;
        FB::variant varResult;

        FunctorCallImpl<Functor, bool> *funct = new FunctorCallImpl<Functor, bool>(func);
        if (!host->isMainThread())
        {
            CrossThreadCall *call = new CrossThreadCall(funct);
            {
                boost::unique_lock<boost::mutex> lock(call->m_mutex);
                host->ScheduleAsyncCall(&CrossThreadCall::syncCallbackFunctor, call);

                while (!call->m_returned) {
                    call->m_cond.wait(lock);
                }
                result = funct->getResult();
                varResult = call->m_result;
            }
            delete call;
        } else {
            funct->call();
            result = funct->getResult();
        }
        delete funct;
        if (varResult.get_type() == typeid(FB::script_error)) {
            throw FB::script_error(varResult.cast<const FB::script_error>().what());
        }
        return result;
    }

    
    template <class Functor>
    typename Functor::result_type BrowserHost::CallOnMainThread(Functor func)
    {
        return CrossThreadCall::syncCall(shared_ptr(), func);
    }
    
    template <class C, class Functor>
    void BrowserHost::ScheduleOnMainThread(boost::shared_ptr<C> obj, Functor func)
    {
        CrossThreadCall::asyncCall(shared_ptr(), obj, func);
    }    
};

#endif // H_FB_CROSSTHREADCALL