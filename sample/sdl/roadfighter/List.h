#include <stdlib.h>
#include <assert.h>
#ifndef GENERIC_LIST
#define GENERIC_LIST

/* 
Funciones para LISTAS: 

	void Delete()
	void Instance(List<T> &l)
	void Rewind(void)
	void Forward(void)
	void Next(void)
	void Prev(void)
	T *GetObj(void)
	void SetObj(T *o)
	LLink<T> *GetPos(void)
	bool EmptyP()
	bool EndP()
	bool LastP()
	bool BeginP()
	void Insert(T *o)
	void Add(T *o)
	void AddAfter(LLink<T> *pos,T *o)
	void AddBefore(LLink<T> *pos,T *o)
	bool Iterate(T *&o)
	T *ExtractIni(void)
	T *Extract(void)
	bool MemberP(T *o)
	T *MemberGet(T *o)
	bool MemberRefP(T *o)
	int Length()
	void Copy(List l)
	void Synchronize(List *l);
	void Append(List l) 
	bool DeleteElement(T *o)
	T *GetRandom(void)
	int SearchObjRef(T *o)
	int SearchObj(T *o)
	void SetNoOriginal(void)
	void SetOriginal(void)
	void Sort(bool *p(T *o1,T *o2))


	bool operator==(List<T> &l)
	T *operator[](int index)
*/ 

template <class T> class LLink {
public:
	LLink<T>(T *o,LLink<T> *n=0) {
		obj=o;next=n;
	};
	~LLink<T>() {delete obj;
				 if (next!=0) delete next;};
	inline LLink<T> *Getnext() {return next;};
	inline void Setnext(LLink<T> *n) {next=n;};
	inline T *GetObj() {return obj;};
	inline void SetObj(T *o) {obj=o;};

	void Anade(T *o) {
		if (next==0) {
			LLink<T> *node=new LLink<T>(o);
			next=node;
		} else {
			next->Anade(o);	
		}
		};

private:
	T *obj;
	LLink<T> *next;
};

template <class T> class List {
public:
	List<T>() {list=0;act=0;top=0;original=true;};
	~List<T>() {
		if (original) {
			T *o;
			while(!EmptyP()) {
				o=ExtractIni();
				delete o;
			} /* while */ 
			delete list;
		} /* if */ 
	};
	List<T>(List<T> &l) {list=l.list;act=list;top=l.top;original=false;};

	void Delete() {
		if (original) {
			T *o;
			while(!EmptyP()) {
				o=ExtractIni();
				delete o;
			} /* while */ 
			delete list;
		} /* if */ 
		list=0;
		act=0;
		top=0;
	};

	void Instance(List<T> &l) {list=l.list;act=list;top=l.top;original=false;};
	void Rewind(void) {act=list;};
	void Forward(void) {act=top;};
	void Next(void) {
		if (act!=0) act=act->Getnext();
	};

	void Prev(void) {
		LLink<T> *tmp;

		if (act!=list) {
			tmp=list;
			while(tmp->Getnext()!=act) tmp=tmp->Getnext();
			act=tmp;
		} /* if */ 
	};

	T *GetObj(void) {return act->GetObj();};
	void SetObj(T *o) {act->SetObj(o);};

	LLink<T> *GetPos(void) {return act;};
	bool EmptyP() {return list==0;};
	bool EndP() {return act==0;};
	bool LastP() {return act==top;};
	bool BeginP() {return act==list;};

	void Insert(T *o) {
		if (list==0) {
			list=new LLink<T>(o);
			top=list;
		} else {
			list=new LLink<T>(o,list);
		} /* if */ 
	};

	void Add(T *o) {
		if (list==0) {
			list=new LLink<T>(o);
			top=list;
		} else {
			top->Anade(o);
			top=top->Getnext();
		} /* if */ 
	};

	void AddAfter(LLink<T> *pos,T *o)
	{
		if (pos==0) {
			if (list==0) {
				list=new LLink<T>(o);
				top=list;
			} else {
				list=new LLink<T>(o,list);
			} /* if */ 
		} else {
			LLink<T> *nl=new LLink<T>(o);
		
			nl->Setnext(pos->Getnext());
			pos->Setnext(nl);
			if (nl->Getnext()==0) top=nl;
		} /* if */ 
	} /* AddAfter */ 

	void AddBefore(LLink<T> *pos,T *o)
	{
		if (pos==list) {
			if (list==0) {
				list=new LLink<T>(o);
				top=list;
			} else {
				list=new LLink<T>(o,list);
			} /* if */ 
		} else {
			LLink<T> *l,*nl=new LLink<T>(o);

			l=list;
			while(l->Getnext()!=pos) l=l->Getnext();
			l->Setnext(nl);
			nl->Setnext(pos);
		
			if (pos==0) top=nl;
		} /* if */ 
	} /* AddBefore */ 

	T *operator[](int index) {
		LLink<T> *tmp=list;
		while(tmp!=0 && index>0) {
			tmp=tmp->Getnext();
			index--;
		} /* while */ 
		if (tmp == 0)
		{
			assert(0);
			//throw;
		}
		return tmp->GetObj();
	};

	bool Iterate(T *&o) {
		if (EndP()) return false;
		o=act->GetObj();
		act=act->Getnext();
		return true;
	} /* Iterate */ 

	T *ExtractIni(void) {
		LLink<T> *tmp;
		T *o;

		if (list==0) return 0;
		o=list->GetObj();
		tmp=list;
		list=list->Getnext();
		tmp->Setnext(0);
		if (act==tmp) act=list;
		if (top==act) top=0;
		tmp->SetObj(0);
		delete tmp;
		return o;
	} /* ExtractIni */ 

	T *Extract(void) {
		LLink<T> *tmp,*tmp2=0;
		T *o;

		if (list==0) return 0;
		tmp=list;
		while(tmp->Getnext()!=0) {
			tmp2=tmp;
			tmp=tmp->Getnext();
		} /* while */ 
		o=tmp->GetObj();
		if (tmp2==0) {
			list=0;
			top=0;
			act=0;
		} else {
			tmp2->Setnext(0);
			top=tmp2;
		} /* if */ 

		if (act==tmp) act=top;
		tmp->SetObj(0);
		delete tmp;
		return o;
	} /* Extract */ 

	bool MemberP(T *o) {
		LLink<T> *tmp;
		tmp=list;
		while(tmp!=0) {
			if (*(tmp->GetObj())==*o) return true;
			tmp=tmp->Getnext();
		} /* while */ 
		return false;
	} /* MemberP */ 

	T *MemberGet(T *o) {
		LLink<T> *tmp;
		tmp=list;
		while(tmp!=0) {
			if (*(tmp->GetObj())==*o) return tmp->GetObj();
			tmp=tmp->Getnext();
		} /* while */ 
		return 0;
	} /* MemberGet */ 
	
	bool MemberRefP(T *o) {
		LLink<T> *tmp;
		tmp=list;
		while(tmp!=0) {
			if (tmp->GetObj()==o) return true;
			tmp=tmp->Getnext();
		} /* while */ 
		return false;
	} /* MemberRefP */ 

	int Length() {
		LLink<T> *tmp;
		int count=0;

		tmp=list;
		while(tmp!=0) {
			tmp=tmp->Getnext();
			count++;
		} /* while */ 
		return count;
	};

	void Copy(List l) {
		List<T> ltmp;
		T *o;
		Delete();
		original=true;

		ltmp.Instance(l);
		ltmp.Rewind();
		while(ltmp.Iterate(o)) {
			o=new T(*o);
			Add(o);
		} /* while */ 
		Synchronize(&l);
	} /* Copy */ 


	void Synchronize(List *l)
	{
		LLink<T> *ll;

		ll=l->list;
		act=list;
		while(ll!=0 && ll!=l->act) {
			ll=ll->Getnext();
			if (act!=0) act=act->Getnext();
		} /* while */ 
	} /* Synchronize */ 


	void Append(List l) {
		T *o;

		l.Rewind();
		while(l.Iterate(o)) {
			o=new T(*o);
			Add(o);
		} /* while */ 
	} /* Append */ 


	bool DeleteElement(T *o)
	{
		LLink<T> *tmp1,*tmp2;

		tmp1=list;
		tmp2=0;
		while(tmp1!=0 && tmp1->GetObj()!=o) {
			tmp2=tmp1;
			tmp1=tmp1->Getnext();
		} /* while */ 

		if (tmp1!=0) {
			if (tmp2==0) {
				/* Eliminar el primer elemento de la lista: */ 
				list=list->Getnext();
				tmp1->Setnext(0);
				if (act==tmp1) act=list;
				tmp1->SetObj(0);
				delete tmp1;
			} else {
				/* Eliminar un elemento intermedio: */ 
				tmp2->Setnext(tmp1->Getnext());
				if (act==tmp1) act=tmp1->Getnext();
				if (top==tmp1) top=tmp2;
				tmp1->Setnext(0);
				tmp1->SetObj(0);
				delete tmp1;
			} /* if */ 
			return true;
		} else {
			return false;
		} /* if */ 

	} /* DeleteElement */ 

	T *GetRandom(void) {
		int i,l=Length();
		i=((rand()*l)/RAND_MAX);
		if (i==l) i=l-1;

		return operator[](i);
	} /* GetRandom */ 

	bool operator==(List<T> &l) {
		LLink<T> *tmp1,*tmp2;

		tmp1=list;
		tmp2=l.list;
		while(tmp1!=0 && tmp2!=0) {
			if (!((*(tmp1->GetObj()))==(*(tmp2->GetObj())))) return false;
			tmp1=tmp1->Getnext();
			tmp2=tmp2->Getnext();
		} /* while */ 
		return tmp1==tmp2;
	} /* == */ 


	int SearchObjRef(T *o)
	{
		LLink<T> *tmp;
		int pos=0;

		tmp=list;
		while(tmp!=0) {
			if ((tmp->GetObj())==o) return pos;
			tmp=tmp->Getnext();
			pos++;
		} /* while */ 
		return -1;
	} /* SearchObj */ 

	int SearchObj(T *o)
	{
		LLink<T> *tmp;
		int pos=0;

		tmp=list;
		while(tmp!=0) {
			if (*(tmp->GetObj())==*o) return pos;
			tmp=tmp->Getnext();
			pos++;
		} /* while */ 
		return -1;
	} /* SearchObj */ 


	void Sort(bool (*p)(T *o1,T *o2))
	{
		LLink<T> *l1,*l2;
		T* tmp;

		l1=0;
		l2=list;
		while(l2!=0) {
			if (l1!=0 && l2!=0) {
				if (!(*p)(l1->GetObj(),l2->GetObj())) {
					tmp=l1->GetObj();
					l1->SetObj(l2->GetObj());
					l2->SetObj(tmp);
				} /* if */ 
			} /* if */ 
			l1=l2;
			l2=l2->Getnext();
		} /* while */ 
	} /* Sort */ 


	void SetNoOriginal(void) {original=false;}
	void SetOriginal(void) {original=true;}

private:
	bool original;
	LLink<T> *list,*top;
	LLink<T> *act;
};



#endif

