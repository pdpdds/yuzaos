#pragma once

namespace std {

	template<typename T> struct type { typedef T value; };

	template<int I> struct Argc {};

	template<typename T>
	struct reference_wrapper
	{
	public:
		typedef T type;

		explicit reference_wrapper(T& t) : t_(&t) {}

		operator T& ()   const { return *t_; }
		T& get()         const { return *t_; }
		T* get_pointer() const { return  t_; }

	private:
		T* t_;
	};

	template<typename T> inline T* get_pointer(T* o) { return o; }
	template<typename T> inline T* get_pointer(T& o) { return &o; }


	template<typename T> struct untie_ref { typedef T type; };
	template<typename T> struct untie_ref<T&> : untie_ref<T> {};

	template<typename T> struct param_traits
	{
		typedef typename untie_ref<T>::type& type;
	};

	template<typename R> struct result_traits { typedef R type; };


	template<typename F> struct result_traits<type<F> > : result_traits<typename F::result_type> {};

	template<typename F>
	struct result_traits<type<reference_wrapper<F> > > : result_traits<typename F::result_type> {};

	template<typename T>
	inline reference_wrapper<T> ref(T& t)
	{
		return reference_wrapper<T>(t);
	}

	template<typename T>
	inline reference_wrapper<T const> cref(const T& t)
	{
		return reference_wrapper<const T>(t);
	}

	namespace placeholders {

		static inline Argc<1> _1() { return std::Argc<1>(); }
		static inline Argc<2> _2() { return std::Argc<2>(); }
		static inline Argc<3> _3() { return std::Argc<3>(); }
		static inline Argc<4> _4() { return std::Argc<4>(); }
		static inline Argc<5> _5() { return std::Argc<5>(); }
		static inline Argc<6> _6() { return std::Argc<6>(); }
		static inline Argc<7> _7() { return std::Argc<7>(); }
		static inline Argc<8> _8() { return std::Argc<8>(); }
		static inline Argc<9> _9() { return std::Argc<9>(); }

	}


	struct storage_base
	{

		template<typename T>
		inline T& operator[](T& v) const { return v; }

		template<typename T>
		inline const T& operator[](const T& v) const { return v; }


		template<typename T>
		inline T& operator[](reference_wrapper<T>& v) const { return v.get(); }

		template<typename T>
		inline T& operator[](const reference_wrapper<T>& v) const { return v.get(); }
	};


	struct storage0 : storage_base
	{
		using storage_base::operator[];


		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S&) const
		{
			return f();
		}

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const
		{
			return f();
		}
	};

	template<typename D>
	struct storage1_base : storage0
	{
		typedef storage0 base;

		using base::operator[];

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_]);
		}
	};

	template<typename A1>
	struct storage1 : storage1_base<storage1<A1> >
	{
		typedef storage1_base<storage1<A1> > base;
		typedef typename param_traits<A1>::type P1;
		typedef typename result_traits<A1>::type result_type;

		storage1(P1 p1) : a1_(p1) {}

		using base::operator[];

		inline result_type operator[](Argc<1>(*)()) const { return a1_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const
		{
			return f(a1_);
		}

		A1 a1_;
	};


	template<int I>
	struct storage1<Argc<I>(*)()> : storage1_base<storage1<Argc<I>(*)()> >
	{
		typedef typename param_traits<Argc<I>(*)()>::type P1;

		storage1(P1) {}

		Argc<I>(*a1_)(void);
	};

	template<typename D, typename A1>
	struct storage2_base : storage1<A1>
	{
		typedef storage1<A1> base;
		using base::operator[];

		storage2_base(typename base::P1 p1) : base(p1) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_]);
		}
	};
	template<typename A1, typename A2>
	struct storage2 : storage2_base<storage2<A1, A2>, A1>
	{
		typedef storage2_base<storage2<A1, A2>, A1> base;
		typedef typename param_traits<A2>::type P2;
		typedef typename result_traits<A2>::type result_type;

		storage2(typename base::P1 p1, P2 p2) : base(p1), a2_(p2) {}

		using base::operator[];
		inline result_type operator[](Argc<2>(*)()) const { return a2_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, a2_); }

		A2 a2_;
	};
	template<typename A1, int I>
	struct storage2<A1, Argc<I>(*)()> : storage2_base<storage2<A1, Argc<I>(*)()>, A1>
	{
		typedef storage2_base<storage2<A1, Argc<I>(*)()>, A1> base;
		typedef typename param_traits<Argc<I>(*)()>::type P2;

		storage2(typename base::P1 p1, P2) : base(p1) {}

		Argc<I>(*a2_)(void);
	};

	template<typename D, typename A1, typename A2>
	struct storage3_base : storage2<A1, A2>
	{
		typedef storage2<A1, A2> base;
		using base::operator[];

		storage3_base(typename base::P1 p1, typename base::P2 p2) : base(p1, p2) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_], s[d.a3_]);
		}
	};
	template<typename A1, typename A2, typename A3>
	struct storage3 : storage3_base<storage3<A1, A2, A3>, A1, A2>
	{
		typedef storage3_base<storage3<A1, A2, A3>, A1, A2> base;
		typedef typename param_traits<A3>::type P3;
		typedef typename result_traits<A3>::type result_type;

		storage3(typename base::P1 p1, typename base::P2 p2, P3 p3) : base(p1, p2), a3_(p3) {}

		using base::operator[];
		inline result_type operator[](Argc<3>(*)()) const { return a3_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, this->a2_, a3_); }

		A3 a3_;
	};
	template<typename A1, typename A2, int I>
	struct storage3<A1, A2, Argc<I>(*)()> : storage3_base<storage3<A1, A2, Argc<I>(*)()>, A1, A2>
	{
		typedef storage3_base<storage3<A1, A2, Argc<I>(*)()>, A1, A2> base;
		typedef typename param_traits<Argc<I>(*)()>::type P3;

		storage3(typename base::P1 p1, typename base::P2 p2, P3) : base(p1, p2) {}
		Argc<I>(*a3_)(void);
	};

	template<typename D, typename A1, typename A2, typename A3>
	struct storage4_base : storage3<A1, A2, A3>
	{
		typedef storage3<A1, A2, A3> base;
		using base::operator[];

		storage4_base(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3) : base(p1, p2, p3) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_], s[d.a3_], s[d.a4_]);
		}
	};
	template<typename A1, typename A2, typename A3, typename A4>
	struct storage4 : storage4_base<storage4<A1, A2, A3, A4>, A1, A2, A3>
	{
		typedef storage4_base<storage4<A1, A2, A3, A4>, A1, A2, A3> base;
		typedef typename param_traits<A4>::type P4;
		typedef typename result_traits<A4>::type result_type;

		using base::operator[];

		storage4(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, P4 p4) : base(p1, p2, p3), a4_(p4) {}

		inline result_type operator[](Argc<4>(*)()) const { return a4_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, this->a2_, this->a3_, a4_); }

		A4 a4_;
	};
	template<typename A1, typename A2, typename A3, int I>
	struct storage4<A1, A2, A3, Argc<I>(*)()> : storage4_base<storage4<A1, A2, A3, Argc<I>(*)()>, A1, A2, A3>
	{
		typedef storage4_base<storage4<A1, A2, A3, Argc<I>(*)()>, A1, A2, A3> base;
		typedef typename param_traits<Argc<I>(*)()>::type P4;

		storage4(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, P4) : base(p1, p2, p3) {}
		Argc<I>(*a4_)(void);
	};

	template<typename D, typename A1, typename A2, typename A3, typename A4>
	struct storage5_base : storage4<A1, A2, A3, A4>
	{
		typedef storage4<A1, A2, A3, A4> base;
		using base::operator[];

		storage5_base(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4) : base(p1, p2, p3, p4) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_], s[d.a3_], s[d.a4_], s[d.a5_]);
		}
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5>
	struct storage5 : storage5_base<storage5<A1, A2, A3, A4, A5>, A1, A2, A3, A4>
	{
		typedef storage5_base<storage5<A1, A2, A3, A4, A5>, A1, A2, A3, A4> base;
		typedef typename param_traits<A5>::type P5;
		typedef typename result_traits<A5>::type result_type;

		using base::operator[];

		storage5(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, P5 p5) : base(p1, p2, p3, p4), a5_(p5) {}

		inline result_type operator[](Argc<5>(*)()) const { return a5_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, this->a2_, this->a3_, this->a4_, a5_); }

		A5 a5_;
	};
	template<typename A1, typename A2, typename A3, typename A4, int I>
	struct storage5<A1, A2, A3, A4, Argc<I>(*)()> : storage5_base<storage5<A1, A2, A3, A4, Argc<I>(*)()>, A1, A2, A3, A4>
	{
		typedef storage5_base<storage5<A1, A2, A3, A4, Argc<I>(*)()>, A1, A2, A3, A4> base;
		typedef typename param_traits<Argc<I>(*)()>::type P5;

		storage5(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, P5) : base(p1, p2, p3, p4) {}
		Argc<I>(*a5_)(void);
	};

	template<typename D, typename A1, typename A2, typename A3, typename A4, typename A5>
	struct storage6_base : storage5<A1, A2, A3, A4, A5>
	{
		typedef storage5<A1, A2, A3, A4, A5> base;
		using base::operator[];

		storage6_base(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5) : base(p1, p2, p3, p4, p5) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_], s[d.a3_], s[d.a4_], s[d.a5_], s[d.a6_]);
		}
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	struct storage6 : storage6_base<storage6<A1, A2, A3, A4, A5, A6>, A1, A2, A3, A4, A5>
	{
		typedef storage6_base<storage6<A1, A2, A3, A4, A5, A6>, A1, A2, A3, A4, A5> base;
		typedef typename param_traits<A6>::type P6;
		typedef typename result_traits<A6>::type result_type;

		using base::operator[];

		storage6(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, P6 p6) : base(p1, p2, p3, p4, p5), a6_(p6) {}

		inline result_type operator[](Argc<6>(*)()) const { return a6_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, this->a2_, this->a3_, this->a4_, this->a5_, a6_); }

		A6 a6_;
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, int I>
	struct storage6<A1, A2, A3, A4, A5, Argc<I>(*)()> : storage6_base<storage6<A1, A2, A3, A4, A5, Argc<I>(*)()>, A1, A2, A3, A4, A5>
	{
		typedef storage6_base<storage6<A1, A2, A3, A4, A5, Argc<I>(*)()>, A1, A2, A3, A4, A5> base;
		typedef typename param_traits<Argc<I>(*)()>::type P6;

		storage6(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, P6) : base(p1, p2, p3, p4, p5) {}
		Argc<I>(*a6_)(void);
	};

	template<typename D, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	struct storage7_base : storage6<A1, A2, A3, A4, A5, A6>
	{
		typedef storage6<A1, A2, A3, A4, A5, A6> base;
		using base::operator[];

		storage7_base(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6) : base(p1, p2, p3, p4, p5, p6) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_], s[d.a3_], s[d.a4_], s[d.a5_], s[d.a6_], s[d.a7_]);
		}
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	struct storage7 : storage7_base<storage7<A1, A2, A3, A4, A5, A6, A7>, A1, A2, A3, A4, A5, A6>
	{
		typedef storage7_base<storage7<A1, A2, A3, A4, A5, A6, A7>, A1, A2, A3, A4, A5, A6> base;
		typedef typename param_traits<A7>::type P7;
		typedef typename result_traits<A7>::type result_type;

		using base::operator[];

		storage7(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, P7 p7) : base(p1, p2, p3, p4, p5, p6), a7_(p7) {}

		inline result_type operator[](Argc<7>(*)()) const { return a7_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, this->a2_, this->a3_, this->a4_, this->a5_, this->a6_, a7_); }

		A7 a7_;
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, int I>
	struct storage7<A1, A2, A3, A4, A5, A6, Argc<I>(*)()> : storage7_base<storage7<A1, A2, A3, A4, A5, A6, Argc<I>(*)()>, A1, A2, A3, A4, A5, A6>
	{
		typedef storage7_base<storage7<A1, A2, A3, A4, A5, A6, Argc<I>(*)()>, A1, A2, A3, A4, A5, A6> base;
		typedef typename param_traits<Argc<I>(*)()>::type P7;

		storage7(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, P7) : base(p1, p2, p3, p4, p5, p6) {}
		Argc<I>(*a7_)(void);
	};

	template<typename D, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	struct storage8_base : storage7<A1, A2, A3, A4, A5, A6, A7>
	{
		typedef storage7<A1, A2, A3, A4, A5, A6, A7> base;
		using base::operator[];

		storage8_base(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, typename base::P7 p7) : base(p1, p2, p3, p4, p5, p6, p7) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_], s[d.a3_], s[d.a4_], s[d.a5_], s[d.a6_], s[d.a7_], s[d.a8_]);
		}
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	struct storage8 : storage8_base<storage8<A1, A2, A3, A4, A5, A6, A7, A8>, A1, A2, A3, A4, A5, A6, A7>
	{
		typedef storage8_base<storage8<A1, A2, A3, A4, A5, A6, A7, A8>, A1, A2, A3, A4, A5, A6, A7> base;
		typedef typename param_traits<A8>::type P8;
		typedef typename result_traits<A8>::type result_type;

		using base::operator[];

		storage8(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, typename base::P7 p7, P8 p8) : base(p1, p2, p3, p4, p5, p6, p7), a8_(p8) {}

		inline result_type operator[](Argc<8>(*)()) const { return a8_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, this->a2_, this->a3_, this->a4_, this->a5_, this->a6_, this->a7_, a8_); }

		A8 a8_;
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, int I>
	struct storage8<A1, A2, A3, A4, A5, A6, A7, Argc<I>(*)()> : storage8_base<storage8<A1, A2, A3, A4, A5, A6, A7, Argc<I>(*)()>, A1, A2, A3, A4, A5, A6, A7>
	{
		typedef storage8_base<storage8<A1, A2, A3, A4, A5, A6, A7, Argc<I>(*)()>, A1, A2, A3, A4, A5, A6, A7> base;
		typedef typename param_traits<Argc<I>(*)()>::type P8;

		storage8(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, typename base::P7 p7, P8) : base(p1, p2, p3, p4, p5, p6, p7) {}
		Argc<I>(*a8_)(void);
	};

	template<typename D, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	struct storage9_base : storage8<A1, A2, A3, A4, A5, A6, A7, A8>
	{
		typedef storage8<A1, A2, A3, A4, A5, A6, A7, A8> base;
		using base::operator[];

		storage9_base(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, typename base::P7 p7, typename base::P8 p8) : base(p1, p2, p3, p4, p5, p6, p7, p8) {}

		template<typename R, typename F, typename S>
		inline R operator()(type<R>, F& f, const S& s) const
		{
			const D& d = static_cast<const D&>(*this);
			return d[f](s[d.a1_], s[d.a2_], s[d.a3_], s[d.a4_], s[d.a5_], s[d.a6_], s[d.a7_], s[d.a8_], s[d.a9_]);
		}
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	struct storage9 : storage9_base<storage9<A1, A2, A3, A4, A5, A6, A7, A8, A9>, A1, A2, A3, A4, A5, A6, A7, A8>
	{
		typedef storage9_base<storage9<A1, A2, A3, A4, A5, A6, A7, A8, A9>, A1, A2, A3, A4, A5, A6, A7, A8> base;
		typedef typename param_traits<A9>::type P9;
		typedef typename result_traits<A9>::type result_type;

		using base::operator[];

		storage9(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, typename base::P7 p7, typename base::P8 p8, P9 p9) : base(p1, p2, p3, p4, p5, p6, p7, p8), a9_(p9) {}

		inline result_type operator[](Argc<9>(*)()) const { return a9_; }

		template<typename R, typename F>
		inline R Do(type<R>, F& f) const { return f(this->a1_, this->a2_, this->a3_, this->a4_, this->a5_, this->a6_, this->a7_, this->a8_, a9_); }

		A9 a9_;
	};
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, int I>
	struct storage9<A1, A2, A3, A4, A5, A6, A7, A8, Argc<I>(*)()> : storage9_base<storage9<A1, A2, A3, A4, A5, A6, A7, A8, Argc<I>(*)()>, A1, A2, A3, A4, A5, A6, A7, A8>
	{
		typedef storage9_base<storage9<A1, A2, A3, A4, A5, A6, A7, A8, Argc<I>(*)()>, A1, A2, A3, A4, A5, A6, A7, A8> base;
		typedef typename param_traits<Argc<I>(*)()>::type P9;

		storage9(typename base::P1 p1, typename base::P2 p2, typename base::P3 p3, typename base::P4 p4, typename base::P5 p5, typename base::P6 p6, typename base::P7 p7, typename base::P8 p8, P9) : base(p1, p2, p3, p4, p5, p6, p7, p8) {}
		Argc<I>(*a9_)(void);
	};


	namespace _bind {


		template<typename R, typename F>
		struct f_b
		{
			typedef typename result_traits<R>::type result_type;
			explicit f_b(const F& f) : f_(f) {}
			F f_;
		};

		template<typename R, typename C, typename F>
		struct f_0 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_0(const F& f) : base(f) {}

			template<typename U>
			inline typename base::result_type operator()(U& u) const
			{
				return (get_pointer(u)->*base::f_)();
			}

			inline typename base::result_type operator()(C& c) const
			{
				return (c.*base::f_)();
			}
		};

		template<typename R, typename C, typename F>
		struct f_1 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_1(const F& f) : base(f) {}

			template<typename U, typename A1>
			inline typename base::result_type operator()(U& u, A1 a1) const
			{
				return (get_pointer(u)->*base::f_)(a1);
			}

			template<typename A1>
			inline typename base::result_type operator()(C& c, A1 a1) const
			{
				return (c.*base::f_)(a1);
			}
		};

		template<typename R, typename C, typename F>
		struct f_2 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_2(const F& f) : base(f) {}

			template<typename U, typename A1, typename A2>
			inline typename base::result_type operator()(U& u, A1 a1, A2 a2) const
			{
				return (get_pointer(u)->*base::f_)(a1, a2);
			}

			template<typename A1, typename A2>
			inline typename base::result_type operator()(C& c, A1 a1, A2 a2) const
			{
				return (c.*base::f_)(a1, a2);
			}
		};

		template<typename R, typename C, typename F>
		struct f_3 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_3(const F& f) : base(f) {}

			template<typename U, typename A1, typename A2, typename A3>
			inline typename base::result_type operator()(U& u, A1 a1, A2 a2, A3 a3) const
			{
				return (get_pointer(u)->*base::f_)(a1, a2, a3);
			}

			template<typename A1, typename A2, typename A3>
			inline typename base::result_type operator()(C& c, A1 a1, A2 a2, A3 a3) const
			{
				return (c.*base::f_)(a1, a2, a3);
			}
		};

		template<typename R, typename C, typename F>
		struct f_4 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_4(const F& f) : base(f) {}

			template<typename U, typename A1, typename A2, typename A3, typename A4>
			inline typename base::result_type operator()(U& u, A1 a1, A2 a2, A3 a3, A4 a4) const
			{
				return (get_pointer(u)->*base::f_)(a1, a2, a3, a4);
			}

			template<typename A1, typename A2, typename A3, typename A4>
			inline typename base::result_type operator()(C& c, A1 a1, A2 a2, A3 a3, A4 a4) const
			{
				return (c.*base::f_)(a1, a2, a3, a4);
			}
		};

		template<typename R, typename C, typename F>
		struct f_5 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_5(const F& f) : base(f) {}

			template<typename U, typename A1, typename A2, typename A3, typename A4, typename A5>
			inline typename base::result_type operator()(U& u, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
			{
				return (get_pointer(u)->*base::f_)(a1, a2, a3, a4, a5);
			}

			template<typename A1, typename A2, typename A3, typename A4, typename A5>
			inline typename base::result_type operator()(C& c, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
			{
				return (c.*base::f_)(a1, a2, a3, a4, a5);
			}
		};

		template<typename R, typename C, typename F>
		struct f_6 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_6(const F& f) : base(f) {}

			template<typename U, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
			inline typename base::result_type operator()(U& u, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
			{
				return (get_pointer(u)->*base::f_)(a1, a2, a3, a4, a5, a6);
			}

			template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
			inline typename base::result_type operator()(C& c, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
			{
				return (c.*base::f_)(a1, a2, a3, a4, a5, a6);
			}
		};

		template<typename R, typename C, typename F>
		struct f_7 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_7(const F& f) : base(f) {}

			template<typename U, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
			inline typename base::result_type operator()(U& u, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
			{
				return (get_pointer(u)->*base::f_)(a1, a2, a3, a4, a5, a6, a7);
			}

			template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
			inline typename base::result_type operator()(C& c, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
			{
				return (c.*base::f_)(a1, a2, a3, a4, a5, a6, a7);
			}
		};

		template<typename R, typename C, typename F>
		struct f_8 : f_b<R, F>
		{
			typedef f_b<R, F> base;

			explicit f_8(const F& f) : base(f) {}

			template<typename U, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
			inline typename base::result_type operator()(U& u, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
			{
				return (get_pointer(u)->*base::f_)(a1, a2, a3, a4, a5, a6, a7, a8);
			}

			template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
			inline typename base::result_type operator()(C& c, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
			{
				return (c.*base::f_)(a1, a2, a3, a4, a5, a6, a7, a8);
			}
		};


		template<typename R, typename F, typename S> struct bind_t
		{
		public:


			typedef typename result_traits<R>::type result_type;

			bind_t(const F& f, const S& s) : f_(f), s_(s) {}



			template<typename A>
			inline result_type eval(A& a)
			{
				return s_(type<result_type>(), f_, a);
			}
			template<typename A>
			inline result_type eval(A& a) const
			{
				return s_(type<result_type>(), f_, a);
			}


			inline result_type operator()()
			{
				typedef storage0 ll;
				return s_(type<result_type>(), f_, ll());
			}
			inline result_type operator()() const
			{
				typedef storage0 ll;
				return s_(type<result_type>(), f_, ll());
			}


			template<typename P1>
			inline result_type operator()(P1& p1)
			{
				typedef storage1<P1&> ll;
				return l_(type<result_type>(), f_, ll(p1));
			}
			template<typename P1>
			inline result_type operator()(P1& p1) const
			{
				typedef storage1<P1&> ll;
				return l_(type<result_type>(), f_, ll(p1));
			}
			template<typename P1>
			inline result_type operator()(const P1& p1)
			{
				typedef storage1<const P1&> ll;
				return l_(type<result_type>(), f_, ll(p1));
			}
			template<typename P1>
			inline result_type operator()(const P1& p1) const
			{
				typedef storage1<const P1&> ll;
				return l_(type<result_type>(), f_, ll(p1));
			}

			template<typename P1, typename P2>
			inline result_type operator()(const P1& p1, const P2& p2) const
			{
				typedef storage2<const P1&, const P2&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2));
			}

			template<typename P1, typename P2, typename P3>
			inline result_type operator()(const P1& p1, const P2& p2, const P3& p3) const
			{
				typedef storage3<const P1&, const P2&, const P3&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2, p3));
			}

			template<typename P1, typename P2, typename P3, typename P4>
			inline result_type operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4) const
			{
				typedef storage4<const P1&, const P2&, const P3&, const P4&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2, p3, p4));
			}

			template<typename P1, typename P2, typename P3, typename P4, typename P5>
			inline result_type operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5) const
			{
				typedef storage5<const P1&, const P2&, const P3&, const P4&, const P5&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2, p3, p4, p5));
			}

			template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
			inline result_type operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6) const
			{
				typedef storage6<const P1&, const P2&, const P3&, const P4&, const P5&, const P6&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2, p3, p4, p5, p6));
			}

			template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
			inline result_type operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7) const
			{
				typedef storage7<const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2, p3, p4, p5, p6, p7));
			}

			template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
			inline result_type operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7, const P8& p8) const
			{
				typedef storage8<const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&, const P8&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2, p3, p4, p5, p6, p7, p8));
			}

			template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
			inline result_type operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7, const P8& p8, const P9& p9) const
			{
				typedef storage9<const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&, const P8&, const P9&> ll;
				return s_(type<result_type>(), f_, ll(p1, p2, p3, p4, p5, p6, p7, p8, p9));
			}

		private:

			F f_;
			S s_;
		};

		template<typename R>
		bind_t<R, R(*)(), storage0> bind(R(*f)())
		{
			typedef R(*F)();
			typedef storage0 S;
			return bind_t<R, F, S>(f, S());
		};
		template<typename R, typename P1, typename A1>
		bind_t<R, R(*)(P1), storage1<A1> > bind(R(*f)(P1), A1 a1)
		{
			typedef R(*F)(P1);
			typedef storage1<A1> S;
			return bind_t<R, F, S>(f, S(a1));
		}
		template<typename R, typename P1, typename P2, typename A1, typename A2>
		bind_t<R, R(*)(P1, P2), storage2<A1, A2> > bind(R(*f)(P1, P2), A1 a1, A2 a2)
		{
			typedef R(*F)(P1, P2);
			typedef storage2<A1, A2> S;
			return bind_t<R, F, S>(f, S(a1, a2));
		}
		template<typename R, typename P1, typename P2, typename P3, typename A1, typename A2, typename A3>
		bind_t<R, R(*)(P1, P2, P3), storage3<A1, A2, A3> > bind(R(*f)(P1, P2, P3), A1 a1, A2 a2, A3 a3)
		{
			typedef R(*F)(P1, P2, P3);
			typedef storage3<A1, A2, A3> S;
			return bind_t<R, F, S>(f, S(a1, a2, a3));
		}
		template<typename R, typename P1, typename P2, typename P3, typename P4, typename A1, typename A2, typename A3, typename A4>
		bind_t<R, R(*)(P1, P2, P3, P4), storage4<A1, A2, A3, A4> > bind(R(*f)(P1, P2, P3, P4), A1 a1, A2 a2, A3 a3, A4 a4)
		{
			typedef R(*F)(P1, P2, P3, P4);
			typedef storage4<A1, A2, A3, A4> S;
			return bind_t<R, F, S>(f, S(a1, a2, a3, a4));
		}
		template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename A1, typename A2, typename A3, typename A4, typename A5>
		bind_t<R, R(*)(P1, P2, P3, P4, P5), storage5<A1, A2, A3, A4, A5> > bind(R(*f)(P1, P2, P3, P4, P5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
		{
			typedef R(*F)(P1, P2, P3, P4, P5);
			typedef storage5<A1, A2, A3, A4, A5> S;
			return bind_t<R, F, S>(f, S(a1, a2, a3, a4, a5));
		}
		template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
		bind_t<R, R(*)(P1, P2, P3, P4, P5, P6), storage6<A1, A2, A3, A4, A5, A6> > bind(R(*f)(P1, P2, P3, P4, P5, P6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
		{
			typedef R(*F)(P1, P2, P3, P4, P5, P6);
			typedef storage6<A1, A2, A3, A4, A5, A6> S;
			return bind_t<R, F, S>(f, S(a1, a2, a3, a4, a5, a6));
		}
		template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
		bind_t<R, R(*)(P1, P2, P3, P4, P5, P6, P7), storage7<A1, A2, A3, A4, A5, A6, A7> > bind(R(*f)(P1, P2, P3, P4, P5, P6, P7), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
		{
			typedef R(*F)(P1, P2, P3, P4, P5, P6, P7);
			typedef storage7<A1, A2, A3, A4, A5, A6, A7> S;
			return bind_t<R, F, S>(f, S(a1, a2, a3, a4, a5, a6, a7));
		}
		template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
		bind_t<R, R(*)(P1, P2, P3, P4, P5, P6, P7, P8), storage8<A1, A2, A3, A4, A5, A6, A7, A8> > bind(R(*f)(P1, P2, P3, P4, P5, P6, P7, P8), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
		{
			typedef R(*F)(P1, P2, P3, P4, P5, P6, P7, P8);
			typedef storage8<A1, A2, A3, A4, A5, A6, A7, A8> S;
			return bind_t<R, F, S>(f, S(a1, a2, a3, a4, a5, a6, a7, a8));
		}
		template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
		bind_t<R, R(*)(P1, P2, P3, P4, P5, P6, P7, P8, P9), storage9<A1, A2, A3, A4, A5, A6, A7, A8, A9> > bind(R(*f)(P1, P2, P3, P4, P5, P6, P7, P8, P9), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
		{
			typedef R(*F)(P1, P2, P3, P4, P5, P6, P7, P8, P9);
			typedef storage9<A1, A2, A3, A4, A5, A6, A7, A8, A9> S;
			return bind_t<R, F, S>(f, S(a1, a2, a3, a4, a5, a6, a7, a8, a9));
		}

		template<typename R, typename C, typename C1>
		bind_t<R, _bind::f_0<R, C, R(C::*)()>, storage1<C1> > bind(R(C::* f)(), C1 c1)
		{
			typedef _bind::f_0<R, C, R(C::*)()> F;
			typedef storage1<C1> S;
			return bind_t<R, F, S>(F(f), S(c1));
		}
		template<typename R, typename C, typename C1>
		bind_t<R, _bind::f_0<R, C, R(C::*)() const>, storage1<C1> > bind(R(C::* f)() const, C1 c1)
		{
			typedef _bind::f_0<R, C, R(C::*)() const> F;
			typedef storage1<C1> S;
			return bind_t<R, F, S>(F(f), S(c1));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename C1, typename P1>
		bind_t<R, _bind::f_1<R, C, R(C::*)(A1)>, storage2<C1, P1> > bind(R(C::* f)(A1), C1 c1, P1 p2)
		{
			typedef _bind::f_1<R, C, R(C::*)(A1)> F;
			typedef storage2<C1, P1> S;
			return bind_t<R, F, S>(F(f), S(c1, p2));
		}
		template<typename R, typename C, typename A1, typename C1, typename P1>
		bind_t<R, _bind::f_1<R, C, R(C::*)(A1)>, storage2<C1, P1> > bind(R(C::* f)(A1) const, C1 c1, P1 p2)
		{
			typedef _bind::f_1<R, C, R(C::*)(A1) const> F;
			typedef storage2<C1, P1> S;
			return bind_t<R, F, S>(F(f), S(c1, p2));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename A2, typename C1, typename P1, typename P2>
		bind_t<R, _bind::f_2<R, C, R(C::*)(A1, A2)>, storage3<C1, P1, P2> > bind(R(C::* f)(A1, A2), C1 c1, P1 p1, P2 p2)
		{
			typedef _bind::f_2<R, C, R(C::*)(A1, A2)> F;
			typedef storage3<C1, P1, P2> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2));
		}
		template<typename R, typename C, typename A1, typename A2, typename C1, typename P1, typename P2>
		bind_t<R, _bind::f_2<R, C, R(C::*)(A1, A2) const>, storage3<C1, P1, P2> > bind(R(C::* f)(A1, A2) const, C1 c1, P1 p1, P2 p2)
		{
			typedef _bind::f_2<R, C, R(C::*)(A1, A2) const> F;
			typedef storage3<C1, P1, P2> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename A2, typename A3, typename C1, typename P1, typename P2, typename P3>
		bind_t<R, _bind::f_3<R, C, R(C::*)(A1, A2, A3)>, storage4<C1, P1, P2, P3> > bind(R(C::* f)(A1, A2, A3), C1 c1, P1 p1, P2 p2, P3 p3)
		{
			typedef _bind::f_3<R, C, R(C::*)(A1, A2, A3)> F;
			typedef storage4<C1, P1, P2, P3> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3));
		}
		template<typename R, typename C, typename A1, typename A2, typename A3, typename C1, typename P1, typename P2, typename P3>
		bind_t<R, _bind::f_3<R, C, R(C::*)(A1, A2, A3) const>, storage4<C1, P1, P2, P3> > bind(R(C::* f)(A1, A2, A3) const, C1 c1, P1 p1, P2 p2, P3 p3)
		{
			typedef _bind::f_3<R, C, R(C::*)(A1, A2, A3) const> F;
			typedef storage4<C1, P1, P2, P3> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename C1, typename P1, typename P2, typename P3, typename P4>
		bind_t<R, _bind::f_4<R, C, R(C::*)(A1, A2, A3, A4)>, storage5<C1, P1, P2, P3, P4> > bind(R(C::* f)(A1, A2, A3, A4), C1 c1, P1 p1, P2 p2, P3 p3, P4 p4)
		{
			typedef _bind::f_4<R, C, R(C::*)(A1, A2, A3, A4)> F;
			typedef storage5<C1, P1, P2, P3, P4> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4));
		}
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename C1, typename P1, typename P2, typename P3, typename P4>
		bind_t<R, _bind::f_4<R, C, R(C::*)(A1, A2, A3, A4) const>, storage5<C1, P1, P2, P3, P4> > bind(R(C::* f)(A1, A2, A3, A4) const, C1 c1, P1 p1, P2 p2, P3 p3, P4 p4)
		{
			typedef _bind::f_4<R, C, R(C::*)(A1, A2, A3, A4) const> F;
			typedef storage5<C1, P1, P2, P3, P4> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5>
		bind_t<R, _bind::f_5<R, C, R(C::*)(A1, A2, A3, A4, A5)>, storage6<C1, P1, P2, P3, P4, P5> > bind(R(C::* f)(A1, A2, A3, A4, A5), C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
		{
			typedef _bind::f_5<R, C, R(C::*)(A1, A2, A3, A4, A5)> F;
			typedef storage6<C1, P1, P2, P3, P4, P5> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5));
		}
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5>
		bind_t<R, _bind::f_5<R, C, R(C::*)(A1, A2, A3, A4, A5) const>, storage6<C1, P1, P2, P3, P4, P5> > bind(R(C::* f)(A1, A2, A3, A4, A5) const, C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
		{
			typedef _bind::f_5<R, C, R(C::*)(A1, A2, A3, A4, A5) const> F;
			typedef storage6<C1, P1, P2, P3, P4, P5> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
		bind_t<R, _bind::f_6<R, C, R(C::*)(A1, A2, A3, A4, A5, A6)>, storage7<C1, P1, P2, P3, P4, P5, P6> > bind(R(C::* f)(A1, A2, A3, A4, A5, A6), C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
		{
			typedef _bind::f_6<R, C, R(C::*)(A1, A2, A3, A4, A5, A6)> F;
			typedef storage7<C1, P1, P2, P3, P4, P5, P6> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5, p6));
		}
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
		bind_t<R, _bind::f_6<R, C, R(C::*)(A1, A2, A3, A4, A5, A6) const>, storage7<C1, P1, P2, P3, P4, P5, P6> > bind(R(C::* f)(A1, A2, A3, A4, A5, A6) const, C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
		{
			typedef _bind::f_6<R, C, R(C::*)(A1, A2, A3, A4, A5, A6) const> F;
			typedef storage7<C1, P1, P2, P3, P4, P5, P6> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5, p6));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
		bind_t<R, _bind::f_7<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7)>, storage8<C1, P1, P2, P3, P4, P5, P6, P7> > bind(R(C::* f)(A1, A2, A3, A4, A5, A6, A7), C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
		{
			typedef _bind::f_7<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7)> F;
			typedef storage8<C1, P1, P2, P3, P4, P5, P6, P7> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5, p6, p7));
		}
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
		bind_t<R, _bind::f_7<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7) const>, storage8<C1, P1, P2, P3, P4, P5, P6, P7> > bind(R(C::* f)(A1, A2, A3, A4, A5, A6, A7) const, C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
		{
			typedef _bind::f_7<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7) const> F;
			typedef storage8<C1, P1, P2, P3, P4, P5, P6, P7> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5, p6, p7));
		}
		//------------------------------------------------------------------------------
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
		bind_t<R, _bind::f_8<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7, A8)>, storage9<C1, P1, P2, P3, P4, P5, P6, P7, P8> > bind(R(C::* f)(A1, A2, A3, A4, A5, A6, A7, A8), C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
		{
			typedef _bind::f_8<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7, A8)> F;
			typedef storage9<C1, P1, P2, P3, P4, P5, P6, P7, P8> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5, p6, p7, p8));
		}
		template<typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename C1, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
		bind_t<R, _bind::f_8<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7, A8) const>, storage9<C1, P1, P2, P3, P4, P5, P6, P7, P8> > bind(R(C::* f)(A1, A2, A3, A4, A5, A6, A7, A8) const, C1 c1, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
		{
			typedef _bind::f_8<R, C, R(C::*)(A1, A2, A3, A4, A5, A6, A7, A8) const> F;
			typedef storage9<C1, P1, P2, P3, P4, P5, P6, P7, P8> S;
			return bind_t<R, F, S>(F(f), S(c1, p1, p2, p3, p4, p5, p6, p7, p8));
		}
	};	
}