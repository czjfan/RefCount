#pragma once

#include <iostream>

using std::cout;
using std::endl;

#define SAFE_DELETE(P)		{if(P) {delete (P); (P) = NULL;}}
#define SAFE_RELEASE(P)		{if(P) {(P)->Release(); (P) = NULL;}}

//引用计数器
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

//带指针变量的引用计数器
template <typename T>
class RefPTR :public Ref
{
protected:
	T *p;
public:
	RefPTR();
	RefPTR(T *ptr);
	~RefPTR();

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
	count = 0;//此处设为0、1都有道理，都应该不会崩溃（建议初始化为0，以0表示无引用状态）
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
void RefPTR<T>::Release()
{
	SAFE_DELETE(p);// TODO:数据通用清除
	count = 0;// TODO:是否需要
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
	return (p == NULL && count == 0);
}


//引用计数"示例类"
//用法：对SMARTPOINTER模板特例化
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
//由于SMARTPOINTER直接封装了目标对象指针，在SetData时只能建立副本（写时拷贝），无法共享对象数据（可以转为2级指针）
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
	Ref *pRef;	//计数器（必须设为指针，使对引用同一指针的多个类对象可共享）
	T *p;		//引用计数的指针对象
	//可添加其他成员
public:
					SMARTPOINTER();
					SMARTPOINTER(T *ptr);
					SMARTPOINTER(const SMARTPOINTER<T>&);
					~SMARTPOINTER();

	void			Release();
	SMARTPOINTER&	operator = (const SMARTPOINTER<T>&);
	T&				operator*();
	T*				operator->();

	inline int		GetRefCount();
	inline const T* GetDataConst();//获得计数的指针
	inline T*		GetData();//获得计数的指针
	bool			SetData(T *pinfo);
};

#define	REF_DEBUGINFO(STR)	{								\
							cout << this << " ";			\
							cout << (STR);					\
							cout << ": ref:";				\
							if (pRef)						\
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
	if (pRef)
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
	//添加判断，防止外部将类对象本身作为拷贝构造函数参数
	if (&obj != NULL && (this != &obj))//TODO:后半个判断是否需要？
	{
		//右值引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//正确处理计数值。防止初始化对象引用为0，对象拷贝后release产生ref野指针
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//拷贝变量
		//拷贝函数不考虑ref等指针变量空情况，类内部维护
		pRef = obj.pRef;
		p = obj.p;
	}
	else
	{
		//外部没提供，自己构造
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

		if (pRef->GetCount() == 0)
		{
			SAFE_DELETE(p);
		}
		else
		{
			pRef = new Ref<T>();// 新建空ref，p 设为 NULL，脱离原来的引用
			p = NULL;
		}
	}
	//TOCATCHEXCEPTION：pRef==NULL
}

template<typename T>
SMARTPOINTER<T> & SMARTPOINTER<T>::operator=(const SMARTPOINTER<T>& obj)
{
	if (this != &obj)
	{
		//右值引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//正确处理计数值。防未初始化对象引用为0，对象拷贝后release产生ref野指针
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//自身引用释放（左值引用计数减一）
		Release();

		//拷贝变量
		//拷贝函数不考虑ref等指针变量空情况，类内部维护
		pRef = obj.pRef;
		p = obj.p;
	}

#ifdef _DEBUG
	REF_DEBUGINFO("Ref =");
#endif

	return *this;
}

template<typename T>
inline T & SMARTPOINTER<T>::operator*()
{
	return *(p);
}

template<typename T>
inline T * SMARTPOINTER<T>::operator->()
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
	if (pRef)
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
			//T *p是SMARTPOINTER类内一维指针封装，只能进行写时拷贝，此时与其余引用分离开，引用回归1
			pRef->unref();
			pRef = new Ref();
			if (pRef)
			{
				pRef->addref();

				p = newp;
			}
			else
				return false;
		}
		else if (pRef->GetCount() == 1)//仅自身一份拷贝
		{
			//清除旧的，赋值新的，其他变量不动
			SAFE_DELETE(p);
			p = newp;
		}
		else//ref.count == 0
		{
			pRef->SetCount(1);
				
			p = newp;
		}

		return true;
	}
	else
	{
		if (pRef->GetCount() == 0)//引用提升
		{
			pRef->SetCount(1);
		}
	}
	
	return false;
}


//智能指针：
//#1处理内存泄漏
//#2处理空悬指针
//#3数据共享、保留1份资源
//即控制对象生命周期，不在有引用的情况下被释放，并且保证引用为0时或智能指针类对象释放时被释放

template <typename T>
class SMARTPOINTER2
{
private:
	RefPTR<T> *pRef;//计数器（必须设为指针，使对引用同一指针的多个类对象可共享）
	//可添加其他成员
public:
	SMARTPOINTER2();
	SMARTPOINTER2(T *ptr);
	SMARTPOINTER2(const SMARTPOINTER2<T>&);
	~SMARTPOINTER2();

	void			Release();
	SMARTPOINTER2&	Assign(const SMARTPOINTER2<T>&);
	SMARTPOINTER2&	Assign(const SMARTPOINTER2<T>*);
	SMARTPOINTER2&	operator = (const SMARTPOINTER2<T>&);
	T&				operator*();
	T*				operator->();

	inline int		GetRefCount() const;
	inline bool		IsRefNull() const;//pRef是NULL或者引用计数=0，数据T* p=NULL
	inline bool		IsDataNull() const;
	inline const T*	GetDataConst() const;//获得计数的指针
	inline T*		GetData() const;//获得计数的指针
	bool			SetData(T *pinfo);//安全起见，一个T*指针只能给1个对象赋值
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
	//添加判断，防止外部将类对象本身作为拷贝构造函数参数
	if (&obj != NULL && this != &obj)//TODO:后半个判断是否不需要？
	{
		//右值引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//正确处理计数值。防止未初始化对象引用为0，对象拷贝后release产生ref野指针
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//拷贝变量
		pRef = obj.pRef;
	}
	else
	{
		//外部没提供，自己构造
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

		if (pRef->GetCount() == 0)
		{
			pRef->Release();
		}
		else
			pRef = new RefPTR<T>();// 新建空 ref，解除原来引用的绑定
	}
}

template<typename T>
inline SMARTPOINTER2<T> & SMARTPOINTER2<T>::Assign(const SMARTPOINTER2<T>& obj)
{
	if (&obj != NULL && this != &obj)// TODO:前半个判断是否需要？
	{
		// 右值的引用计数加一
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//正确处理计数值。防止未初始化对象引用为0，对象拷贝后release产生ref野指针
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//自身引用释放（左值引用计数减一）
		Release();

		//拷贝变量
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
inline SMARTPOINTER2<T> & SMARTPOINTER2<T>::operator=(const SMARTPOINTER2<T>& obj)
{
	return Assign(obj);
}

template<typename T>
inline T & SMARTPOINTER2<T>::operator*()
{
	return *(pRef->GetData());
}

template<typename T>
inline T * SMARTPOINTER2<T>::operator->()
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
	if (pRef == NULL)//ref没建立
	{
		pRef = new RefPTR<T>();//重新建立
		if (pRef == NULL)
			return false;
	}

	if (newp != pRef->GetData())
	{
		if (pRef->GetCount() > 1)
		{
			//写时拷贝，自身拷贝出一份新值，引用为1
			/*pRef->unref();
			pRef = new RefPTR<T>(newp);
			if (pRef == NULL)
				return false;*/

			//[不使用拷贝]修改全局值，对所有引用者更改
			pRef->ReleaseData();
			pRef->SetData(newp);
		}
		else if (pRef->GetCount() == 1)//仅自身一份拷贝
		{
			//清除旧的，赋值新的，其他变量不动
			pRef->ReleaseData();
			pRef->SetData(newp);
		}
		else//pRef->count == 0
		{
			pRef->SetCount(1);

			pRef->SetData(newp);
		}

		return true;
	}
	else
	{
		if (pRef->GetCount() == 0)//引用提升
		{
			pRef->SetCount(1);
		}
	}

	return false;
}
