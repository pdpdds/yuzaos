#pragma once

#include "bind.hpp"

namespace std {


	namespace _functional {

		template<typename R, typename S>
		struct core_base
		{
			virtual ~core_base() {}
			virtual core_base* clone() const = 0;
			virtual R CallFunction(const S& s) const = 0;
			virtual void SetObject(void* p) {}
		};

		template<typename R, typename S, typename F>
		struct core_function : core_base< R, S >
		{
			const F     _f;

			explicit core_function(const F& f) :_f(f) {}

			virtual core_base<R, S>* clone() const
			{
				return new core_function(_f);
			}

			R CallFunction(const S& s) const
			{
				return s.Do(type<R>(), _f);
			}
		};

		template<typename R, typename F, typename C>
		struct member_function
		{
			explicit member_function(const member_function& other) :pFunction(other.pFunction), pObject(other.pObject) {}
			explicit member_function(const F& f) :pFunction(f), pObject(0) {}
			explicit member_function() :pFunction(0), pObject(0) {}

			const F     pFunction;
			C* pObject;

			inline void set_this(void* app)
			{
				pObject = static_cast<C*>(app);
			}

			inline R operator()() const
			{
				return (pObject->*pFunction)();
			}

			template<typename A1>
			inline R operator()(A1 a1) const
			{
				return (pObject->*pFunction)(a1);
			}

			template<typename A1, typename A2>
			inline R operator()(A1 a1, A2 a2) const
			{
				return (pObject->*pFunction)(a1, a2);
			}

			template<typename A1, typename A2, typename A3>
			inline R operator()(A1 a1, A2 a2, A3 a3) const
			{
				return (pObject->*pFunction)(a1, a2, a3);
			}

			template<typename A1, typename A2, typename A3, typename A4>
			inline R operator()(A1 a1, A2 a2, A3 a3, A4 a4) const
			{
				return (pObject->*pFunction)(a1, a2, a3, a4);
			}
		};

		template<typename R, typename S, typename F, typename C>
		struct core_member_function : core_base< R, S >
		{
			F       _f;

			explicit core_member_function(const F& f) :_f(f) {}

			virtual core_base<R, S>* clone() const
			{
				return new core_member_function(_f);
			}
			R CallFunction(const S& s) const
			{
				return s.Do(type<R>(), _f);
			}
			void SetObject(void* app)
			{
				_f.set_this(app);
			}
		};

		template<typename T, typename S>
		struct core_bind : core_base< typename T::result_type, S >
		{
			typedef typename T::result_type result_type;

			const T     obj;

			explicit core_bind(const T& b) :obj(b) {}

			virtual core_base<result_type, S>* clone() const
			{
				return new core_bind(obj);
			}

			virtual result_type CallFunction(const S& s) const
			{
				return obj.eval(s);
			}
		};

	}

	template<typename R, typename S> struct function_base
	{
		function_base() :pCore(0) {}
		~function_base() { delete pCore; }

		operator bool() const
		{
			if (pCore)
			{
				return true;
			}

			return false;
		}

		_functional::core_base<R, S>* pCore;

		template<typename C>
		inline void set(C* c)
		{
			pCore->SetObject((void*)(c));
		}

		function_base(const function_base& other)
		{
			if (other.pCore)
			{
				pCore = other.pCore->clone();
			}

		}

		function_base& operator=(const function_base& other)
		{
			delete pCore;
			pCore = other.pCore ? other.pCore->clone() : other.pCore;
			return *this;
		}

	};

	template<typename S> struct function {};


	template<typename R>
	struct function<R()> : function_base<R, storage0>
	{
		typedef R(*Fn)();
		typedef storage0 St;
		using function_base<R, St>::pCore;

		function() {}

		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)())
		{
			typedef _functional::member_function<R, R(C::*)(), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)())
		{
			typedef _functional::member_function<R, R(C::*)(), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}
		
		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}

		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()() const
		{
			return pCore->CallFunction(St());
		}
	};

	template<typename R, typename P1>
	struct function<R(P1)> : function_base<R, storage1<P1> >
	{
		typedef R(*Fn)(P1);
		typedef storage1<P1> St;
		using function_base<R, St>::pCore;

		function() {}

		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1))
		{
			typedef _functional::member_function<R, R(C::*)(P1), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1))
		{
			typedef _functional::member_function<R, R(C::*)(P1), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1) const
		{
			return pCore->CallFunction(St(p1));
		}
	};

	template<typename R, typename P1, typename P2>
	struct function<R(P1, P2)> : function_base<R, storage2<P1, P2> >
	{
		typedef R(*Fn)(P1, P2);
		typedef storage2<P1, P2> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2) const
		{
			return pCore->CallFunction(St(p1, p2));
		}
	};

	template<typename R, typename P1, typename P2, typename P3>
	struct function<R(P1, P2, P3)> : function_base<R, storage3<P1, P2, P3> >
	{
		typedef R(*Fn)(P1, P2, P3);
		typedef storage3<P1, P2, P3> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2, P3))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2, P3), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2, P3))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3) const
		{
			return pCore->CallFunction(St(p1, p2, p3));
		}
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4>
	struct function<R(P1, P2, P3, P4)> : function_base<R, storage4<P1, P2, P3, P4> >
	{
		typedef R(*Fn)(P1, P2, P3, P4);
		typedef storage4<P1, P2, P3, P4> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2, P3, P4))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3, P4 p4) const
		{
			return pCore->CallFunction(St(p1, p2, p3, p4));
		}
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	struct function<R(P1, P2, P3, P4, P5)> : function_base<R, storage5<P1, P2, P3, P4, P5> >
	{
		typedef R(*Fn)(P1, P2, P3, P4, P5);
		typedef storage5<P1, P2, P3, P4, P5> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2, P3, P4, P5))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
		{
			return pCore->CallFunction(St(p1, p2, p3, p4, p5));
		}
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	struct function<R(P1, P2, P3, P4, P5, P6)> : function_base<R, storage6<P1, P2, P3, P4, P5, P6> >
	{
		typedef R(*Fn)(P1, P2, P3, P4, P5, P6);
		typedef storage6<P1, P2, P3, P4, P5, P6> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2, P3, P4, P5, P6))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const
		{
			return pCore->CallFunction(St(p1, p2, p3, p4, p5, p6));
		}
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	struct function<R(P1, P2, P3, P4, P5, P6, P7)> : function_base<R, storage7<P1, P2, P3, P4, P5, P6, P7> >
	{
		typedef R(*Fn)(P1, P2, P3, P4, P5, P6, P7);
		typedef storage7<P1, P2, P3, P4, P5, P6, P7> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6, P7))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6, P7), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2, P3, P4, P5, P6, P7))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) const
		{
			return pCore->CallFunction(St(p1, p2, p3, p4, p5, p6, p7));
		}
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	struct function<R(P1, P2, P3, P4, P5, P6, P7, P8)> : function_base<R, storage8<P1, P2, P3, P4, P5, P6, P7, P8> >
	{
		typedef R(*Fn)(P1, P2, P3, P4, P5, P6, P7, P8);
		typedef storage8<P1, P2, P3, P4, P5, P6, P7, P8> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6, P7, P8))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7, P8), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6, P7, P8), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7, P8), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2, P3, P4, P5, P6, P7, P8))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7, P8), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) const
		{
			return pCore->CallFunction(St(p1, p2, p3, p4, p5, p6, p7, p8));
		}
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
	struct function<R(P1, P2, P3, P4, P5, P6, P7, P8, P9)> : function_base<R, storage9<P1, P2, P3, P4, P5, P6, P7, P8, P9> >
	{
		typedef R(*Fn)(P1, P2, P3, P4, P5, P6, P7, P8, P9);
		typedef storage9<P1, P2, P3, P4, P5, P6, P7, P8, P9> St;
		using function_base<R, St>::pCore;

		function() {}
		function(Fn f)
		{
			pCore = new _functional::core_function<R, St, Fn>(f);
		}
		function operator=(Fn f)
		{
			delete pCore;
			pCore = new _functional::core_function<R, St, Fn>(f);
			return *this;
		}

		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6, P7, P8, P9))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
		}
		template<typename C>
		function(R(C::* f)(P1, P2, P3, P4, P5, P6, P7, P8, P9), C* c)
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9), C> F;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			pCore->SetObject((void*)(c));
		}
		template<typename C>
		function operator=(R(C::* f)(P1, P2, P3, P4, P5, P6, P7, P8, P9))
		{
			typedef _functional::member_function<R, R(C::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9), C> F;
			delete pCore;
			pCore = new _functional::core_member_function<R, St, F, C>(F(f));
			return *this;
		}

		template<typename A, typename B, typename C>
		function(const _bind::bind_t<A, B, C>& b)
		{
			pCore = new _functional::core_bind<bind_t<A, B, C>, St >(b);
		}
		template<typename A, typename B, typename C>
		function operator=(const _bind::bind_t<A, B, C>& b)
		{
			delete pCore;
			pCore = new _functional::core_bind<_bind::bind_t<A, B, C>, St >(b);
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9) const
		{
			return pCore->CallFunction(St(p1, p2, p3, p4, p5, p6, p7, p8, p9));
		}
	};

}
