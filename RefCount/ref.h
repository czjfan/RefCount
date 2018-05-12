#pragma once

#include <iostream>

using std::cout;
using std::endl;

#define SAFE_DELETE(P)		{if(P) {delete (P); (P) = NULL;}}
#define SAFE_RELEASE(P)		{if(P) {(P)->Release(); (P) = NULL;}}

// 计数器（用于引用计数）
class Ref
{
protected:
	int count;
public:
	Ref();
	~Ref();

	inline void addref();
	inline void unref();
	inline void SetCount(int c);
	inline int GetCount() const;
};

void Ref::addref()
{
	count++;
}
void Ref::unref()
{
	count--;
	if (count < 0)
		count = 0;
}
void Ref::SetCount(int c)
{
	if (c >= 0)
		count = c;
}
int Ref::GetCount() const
{
	return count;
}

// 带数据指针的引用计数器
template <typename T>
class RefPTR :public Ref
{
protected:
	T *p;
public:
	RefPTR();
	RefPTR(T *ptr);
	~RefPTR();

	inline void unref();

	void Release();
	void ReleaseData();
	void SetData(T *data);
	const T* GetDataConst() const;
	T* GetData() const;
	inline bool IsNull() const;
};

template<typename T>
RefPTR<T>::RefPTR() :p(NULL)
{
	count = 0;// 此处设为0、1都有道理，都应该不会崩溃（建议初始化为0，以0表示无引用状态）
}

template<typename T>
RefPTR<T>::RefPTR(T * ptr) :p(ptr)
{
	count = 1;
}

template<typename T>
RefPTR<T>::~RefPTR()
{
	Release();
}

template<typename T>
inline void RefPTR<T>::unref()
{
	Ref::unref();
	if (count == 0)
		Release();
}

template<typename T>
void RefPTR<T>::Release()
{
	SAFE_DELETE(p);// TODO:数据通用清除
	count = 0;
}

template<typename T>
void RefPTR<T>::ReleaseData()
{
	SAFE_DELETE(p);
}

template<typename T>
inline void RefPTR<T>::SetData(T * data)
{
	p = data;
}

template<typename T>
inline const T * RefPTR<T>::GetDataConst() const
{
	return p;
}

template<typename T>
inline T * RefPTR<T>::GetData() const
{
	return p;
}

template<typename T>
inline bool RefPTR<T>::IsNull() const
{
	// 数据指针为空，且引用计数为 0
	return (p == NULL && count == 0);
}


//引用计数"示例类"
//【对类内封装的指针进行引用计数，计数规则如下】
//【同理可适用于对类对象指针进行引用计数（不使用类封装计数逻辑，仅封装计数值和指针等内容，用户代码操作计数）的情况，如LPDIRECT3DDEVICE9】
//【注意：两种引用计数实现方法不可混用！除非类内设置不同处理模式】
//01.空参数构造函数：引用计数为0（或1），指针p=NULL（建议初始化为0，以便与02带指针参数构造函数情况区分开）
//02.带指针参数构造函数：引用计数为1，指针赋值（如果参数为NULL，暂不处理）
//1.拷贝构造函数：引用计数+1，如果是1再+1(适应初始化0情况)，浅拷贝变量
//2.赋值函数：参数引用计数+1，如果是1再+1(适应初始化0情况)，自身引用-1（Release），浅拷贝变量
//3.指针赋值函数：[写时拷贝]如果引用计数>1：引用计数-1，申请新空间，引用计数+1，深拷贝；
//									=1(仅自身一份拷贝)：释放变量，新建指针，赋值
//			     [覆盖赋值]释放变量，新建指针，赋值
//4.SetData函数：同一个T*指针只能给一个对象设置，不能重复使用
//SMARTPOINTER直接封装了目标对象指针（而不是封装在Ref中形成总体2级指针），所以在SetData时只能建立副本（写时拷贝），无法修改全体同源引用的数据
//而SMARTPOINTER2在SetData时既能写时拷贝，又可以全局赋值，因为它将目标对象指针和引用计数封装在一个类内指针里。
//
//【对于SMARTPOINTER两个类的使用】，有两种方法：1，直接用类对象。2，使用指针形式
//第一种方法优点：简单明了，自身不需内存操作。
//		   缺点：使用完释放后，没有明显已释放标志，需要添加成员函数判断或释放后新建引用成员变量。
//第二种方法优点：使用完后释放，指针设为NULL，可避免读脏数据
//		   缺点：内存操作麻烦。赋值操作麻烦。
//		   注意：不能直接指针赋值，会出错！
template <typename T>
class SMARTPOINTER
{
private:
	Ref *pRef;	// 计数器（设为指针，使对引用同一指针的多个类对象可共享）
	T *p;		// 引用计数的指针对象
	// 可添加其他成员
public:
					SMARTPOINTER();
					SMARTPOINTER(T *ptr);
					SMARTPOINTER(const SMARTPOINTER<T>&);
					~SMARTPOINTER();

	void			Release();
	SMARTPOINTER&	operator = (const SMARTPOINTER<T>&);
	T&				operator * ();
	T*				operator -> ();

	inline int		GetRefCount();
	inline const T* GetDataConst();//获得计数的指针
	inline T*		GetData();//获得计数的指针
	bool			SetData(T *pinfo);
};

#define	REF_DEBUGINFO(STR)	{								\
							cout << this << " ";			\
							cout << (STR);					\
							cout << ": ref:";				\
							if (pRef != NULL)				\
								cout << pRef->GetCount();	\
							else							\
								cout << "NULL PTR";			\
							cout << endl;					\
							}

template<typename T>
SMARTPOINTER<T>::SMARTPOINTER() :pRef(new Ref()), p(NULL)
{
#ifdef _DEBUG
	REF_DEBUGINFO("Ref ctor");
#endif
}

template<typename T>
inline SMARTPOINTER<T>::SMARTPOINTER(T * ptr)
{
	pRef = new Ref();
	if (pRef != NULL)
	{
		pRef->addref();
		p = ptr;
	}

#ifdef _DEBUG
	REF_DEBUGINFO("Ref param ctor");
#endif
}

template<typename T>
SMARTPOINTER<T>::SMARTPOINTER(const SMARTPOINTER<T>& obj)
{
	// 添加判断，防止外部将类对象本身作为拷贝构造函数参数
	if (&obj != NULL && (this != &obj))// TODO:后半个判断是否需要？
	{
		// 右操作数的引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			// 正确处理计数值。引用计数 0~2
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		// ref、数据指针 同源化
		// 拷贝函数不考虑 ref 等指针变量空情况，类内部维护
		pRef = obj.pRef;
		p = obj.p;
	}
	else
	{
		// 外部没提供，自己构造
		pRef = new Ref();
		p = NULL;
	}

#ifdef _DEBUG
	REF_DEBUGINFO("Ref Copy ctor");
#endif
}

template<typename T>
SMARTPOINTER<T>::~SMARTPOINTER()
{
	Release();

#ifdef _DEBUG
	REF_DEBUGINFO("Ref dtor");
#endif
}

template<typename T>
void SMARTPOINTER<T>::Release()
{
	if (pRef != NULL)
	{
		pRef->unref();

		// 舍弃的过程：封装到 Ref 类中
		//if (pRef->GetCount() == 0)
		//{
		//	SAFE_DELETE(p);
		//}

		// 解除原引用绑定，同时设为 NULL
		pRef = NULL;
		p = NULL;
	}
	//TOCATCHEXCEPTION：pRef==NULL
}

template<typename T>
SMARTPOINTER<T> & SMARTPOINTER<T>::operator = (const SMARTPOINTER<T>& obj)
{
	if (this != &obj)
	{
		// 右操作数的引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			// 正确处理计数值。引用计数 0~2
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		// 自身引用释放
		Release();

		// ref、数据指针 同源化
		// 拷贝函数不考虑ref等指针变量空情况，类内部维护
		pRef = obj.pRef;
		p = obj.p;
	}

#ifdef _DEBUG
	REF_DEBUGINFO("Ref =");
#endif

	return *this;
}

template<typename T>
inline T & SMARTPOINTER<T>::operator * ()
{
	return *(p);
}

template<typename T>
inline T * SMARTPOINTER<T>::operator -> ()
{
	return p;
}

template<typename T>
inline const T * SMARTPOINTER<T>::GetDataConst()
{
	return p;
}

template<typename T>
inline T * SMARTPOINTER<T>::GetData()
{
	return p;
}

template<typename T>
inline int SMARTPOINTER<T>::GetRefCount()
{
	if (pRef != NULL)
		return pRef->GetCount();
	else
		return -1;
}

template<typename T>
inline bool SMARTPOINTER<T>::SetData(T * newp)
{
	if (pRef == NULL)//ref没建立TODO：是否需要
	{
		pRef = new Ref();
		if (pRef == NULL)
			return false;
	}

	if (newp != p)
	{
		if (pRef->GetCount() > 1)
		{
			// T *p是SMARTPOINTER类内一维指针封装，只能进行写时拷贝
			// 解除原引用，创建新引用，计数回归1
			Release();

			pRef = new Ref(1);
			if (pRef！ = NULL)
				p = newp;
			else
				return false;
		}
		else if (pRef->GetCount() == 1)// 仅自身一个引用
		{
			// 释放原数据，引用新数据
			SAFE_DELETE(p);
			p = newp;
		}
		else// pRef->count == 0
		{
			pRef->SetCount(1);
				
			p = newp;
		}

		return true;
	}
	else
	{
		if (pRef->GetCount() == 0)
		{// 0 引用提升
			pRef->SetCount(1);
		}
	}
	
	return false;
}


// 智能指针
template <typename T>
class SMARTPOINTER2
{
private:
	RefPTR<T> *pRef;// 计数器（设为指针，使各同源对象共享 RefPTR）
public:
	SMARTPOINTER2();
	SMARTPOINTER2(T *ptr);
	SMARTPOINTER2(const SMARTPOINTER2<T>&);
	~SMARTPOINTER2();

	void			Release();// 解除自身引用（可内部、外部调用）
	SMARTPOINTER2&	Assign(const SMARTPOINTER2<T>&);
	SMARTPOINTER2&	Assign(const SMARTPOINTER2<T>*);
	SMARTPOINTER2&	operator = (const SMARTPOINTER2<T>&);
	T&				operator *();
	T*				operator -> ();

	inline int		GetRefCount() const;
	inline bool		IsRefNull() const;	// 判断引用为空
	inline bool		IsDataNull() const;	// 判断数据为空
	inline const T*	GetDataConst() const;// 获得数据指针
	inline T*		GetData() const;	// 获得数据指针
	bool			SetData(T *pinfo);	// 设置数据指针
};

template<typename T>
inline SMARTPOINTER2<T>::SMARTPOINTER2() :pRef(new RefPTR<T>())
{
	;
}

template<typename T>
inline SMARTPOINTER2<T>::SMARTPOINTER2(T * ptr) :pRef(new RefPTR<T>(ptr))
{
	;
}

template<typename T>
inline SMARTPOINTER2<T>::SMARTPOINTER2(const SMARTPOINTER2<T>& obj)
{
	// 添加判断，防止外部将类对象本身作为拷贝构造函数参数
	if (&obj != NULL && this != &obj)// TODO:后半个判断是否不需要？
	{
		// 右操作数的引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			// 正确处理计数值。引用计数 0~2
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		// ref 同源化
		pRef = obj.pRef;
	}
	else
	{
		// 外部没提供，自己构造
		pRef = new RefPTR<T>();
	}
}

template<typename T>
inline SMARTPOINTER2<T>::~SMARTPOINTER2()
{
	Release();
}

template<typename T>
inline void SMARTPOINTER2<T>::Release()
{
	if (pRef != NULL)
	{
		pRef->unref();
		
		// 解除原引用绑定，同时设为 NULL
		pRef = NULL;
	}
}

template<typename T>
inline SMARTPOINTER2<T> & SMARTPOINTER2<T>::Assign(const SMARTPOINTER2<T>& obj)
{
	if (&obj != NULL && this != &obj)// TODO:前半个判断是否需要？
	{
		// 右操作数的引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			// 正确处理计数值。引用计数 0~2
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		// 自身引用释放
		Release();

		// ref 同源化
		pRef = obj.pRef;
	}
	return *this;
}

template<typename T>
inline SMARTPOINTER2<T> & SMARTPOINTER2<T>::Assign(const SMARTPOINTER2<T>* obj)
{
	return Assign(*obj);
}

template<typename T>
inline SMARTPOINTER2<T> & SMARTPOINTER2<T>::operator = (const SMARTPOINTER2<T>& obj)
{
	return Assign(obj);
}

template<typename T>
inline T & SMARTPOINTER2<T>::operator * ()
{
	return *(pRef->GetData());
}

template<typename T>
inline T * SMARTPOINTER2<T>::operator -> ()
{
	return pRef->GetData();
}

template<typename T>
inline int SMARTPOINTER2<T>::GetRefCount() const
{
	if (pRef != NULL)
		return pRef->GetCount();
	else
		return -1;
}

template<typename T>
inline bool SMARTPOINTER2<T>::IsRefNull() const
{
	return (pRef == NULL || pRef->IsNull());
}

template<typename T>
inline bool SMARTPOINTER2<T>::IsDataNull() const
{
	return (pRef == NULL || pRef->GetData() == NULL) ;
}

template<typename T>
inline const T * SMARTPOINTER2<T>::GetDataConst() const
{
	if (pRef != NULL)
		return pRef->GetDataConst();
	
	return NULL;
}

template<typename T>
inline T * SMARTPOINTER2<T>::GetData() const
{
	if (pRef != NULL)
		return pRef->GetData();

	return NULL;
}

template<typename T>
inline bool SMARTPOINTER2<T>::SetData(T * newp)
{
	if (pRef == NULL)// ref 没建立
	{
		pRef = new RefPTR<T>();// 重新建立 ref
		if (pRef == NULL)
			return false;
	}

	if (newp != pRef->GetData())
	{
		if (pRef->GetCount() > 1)
		{
			// 如果用写时拷贝，必须进行 newp != pRef->GetData() 判断
			// 否则，存在不同引用计数源包含同一数据源，一个计数源销毁数据而另一个计数访扔保持数据指针，危险情况
			// 写时拷贝，自身拷贝出一份新值，引用为 1
			/*Release();
			pRef = new RefPTR<T>(newp);
			if (pRef == NULL)
				return false;*/

			// 修改全局值，对所有引用者更改
			pRef->ReleaseData();
			pRef->SetData(newp);
		}
		else if (pRef->GetCount() == 1)// 仅自身一个引用
		{
			// 释放原数据，引用新数据
			pRef->ReleaseData();
			pRef->SetData(newp);
		}
		else// pRef->count == 0
		{
			pRef->SetCount(1);

			pRef->SetData(newp);
		}

		return true;
	}
	else
	{
		if (pRef->GetCount() == 0)
		{// 0 引用提升
			pRef->SetCount(1);
		}
	}

	return false;
}
