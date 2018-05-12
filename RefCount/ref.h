#pragma once

#include <iostream>

using std::cout;
using std::endl;

#define SAFE_DELETE(P)		{if(P) {delete (P); (P) = NULL;}}
#define SAFE_RELEASE(P)		{if(P) {(P)->Release(); (P) = NULL;}}

//���ü�����
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

//��ָ����������ü�����
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
	count = 0;//�˴���Ϊ0��1���е�����Ӧ�ò�������������ʼ��Ϊ0����0��ʾ������״̬��
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
	SAFE_DELETE(p);// TODO:����ͨ�����
	count = 0;// TODO:�Ƿ���Ҫ
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


//���ü���"ʾ����"
//�÷�����SMARTPOINTERģ��������
//�������ڷ�װ��ָ��������ü����������������¡�
//��ͬ��������ڶ������ָ��������ü�������ʹ�����װ�����߼�������װ����ֵ��ָ������ݣ��û�����������������������LPDIRECT3DDEVICE9��
//��ע�⣺�������ü���ʵ�ַ������ɻ��ã������������ò�ͬ����ģʽ��
//01.�ղ������캯�������ü���Ϊ0����1����ָ��p=NULL�������ʼ��Ϊ0���Ա���02��ָ��������캯��������ֿ���
//02.��ָ��������캯�������ü���Ϊ1��ָ�븳ֵ���������ΪNULL���ݲ�����
//1.�������캯�������ü���+1�������1��+1(��Ӧ��ʼ��0���)��ǳ��������
//2.��ֵ�������������ü���+1�������1��+1(��Ӧ��ʼ��0���)����������-1��Release����ǳ��������
//3.ָ�븳ֵ������[дʱ����]������ü���>1�����ü���-1�������¿ռ䣬���ü���+1�������
//									=1(������һ�ݿ���)���ͷű������½�ָ�룬��ֵ
//			     [���Ǹ�ֵ]�ͷű������½�ָ�룬��ֵ
//4.SetData������ͬһ��T*ָ��ֻ�ܸ�һ���������ã������ظ�ʹ��
//����SMARTPOINTERֱ�ӷ�װ��Ŀ�����ָ�룬��SetDataʱֻ�ܽ���������дʱ���������޷�����������ݣ�����תΪ2��ָ�룩
//��SMARTPOINTER2��SetDataʱ����дʱ�������ֿ���ȫ�ָ�ֵ����Ϊ����Ŀ�����ָ������ü�����װ��һ������ָ���
//
//������SMARTPOINTER�������ʹ�á��������ַ�����1��ֱ���������2��ʹ��ָ����ʽ
//��һ�ַ����ŵ㣺�����ˣ��������ڴ������
//		   ȱ�㣺ʹ�����ͷź�û���������ͷű�־����Ҫ��ӳ�Ա�����жϻ��ͷź��½����ó�Ա������
//�ڶ��ַ����ŵ㣺ʹ������ͷţ�ָ����ΪNULL���ɱ����������
//		   ȱ�㣺�ڴ�����鷳����ֵ�����鷳��
//		   ע�⣺����ֱ��ָ�븳ֵ�������
template <typename T>
class SMARTPOINTER
{
private:
	Ref *pRef;	//��������������Ϊָ�룬ʹ������ͬһָ��Ķ�������ɹ���
	T *p;		//���ü�����ָ�����
	//�����������Ա
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
	inline const T* GetDataConst();//��ü�����ָ��
	inline T*		GetData();//��ü�����ָ��
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
	//����жϣ���ֹ�ⲿ�����������Ϊ�������캯������
	if (&obj != NULL && (this != &obj))//TODO:�����ж��Ƿ���Ҫ��
	{
		//��ֵ���ü�����һ
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//��ȷ�������ֵ����ֹ��ʼ����������Ϊ0�����󿽱���release����refҰָ��
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//��������
		//��������������ref��ָ���������������ڲ�ά��
		pRef = obj.pRef;
		p = obj.p;
	}
	else
	{
		//�ⲿû�ṩ���Լ�����
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
			pRef = new Ref<T>();// �½���ref��p ��Ϊ NULL������ԭ��������
			p = NULL;
		}
	}
	//TOCATCHEXCEPTION��pRef==NULL
}

template<typename T>
SMARTPOINTER<T> & SMARTPOINTER<T>::operator=(const SMARTPOINTER<T>& obj)
{
	if (this != &obj)
	{
		//��ֵ���ü�����һ
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//��ȷ�������ֵ����δ��ʼ����������Ϊ0�����󿽱���release����refҰָ��
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//���������ͷţ���ֵ���ü�����һ��
		Release();

		//��������
		//��������������ref��ָ���������������ڲ�ά��
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
	if (pRef == NULL)//refû����TODO���Ƿ���Ҫ
	{
		pRef = new Ref();
		if (pRef == NULL)
			return false;
	}

	if (newp != p)
	{
		if (pRef->GetCount() > 1)
		{
			//T *p��SMARTPOINTER����һάָ���װ��ֻ�ܽ���дʱ��������ʱ���������÷��뿪�����ûع�1
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
		else if (pRef->GetCount() == 1)//������һ�ݿ���
		{
			//����ɵģ���ֵ�µģ�������������
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
		if (pRef->GetCount() == 0)//��������
		{
			pRef->SetCount(1);
		}
	}
	
	return false;
}


//����ָ�룺
//#1�����ڴ�й©
//#2�������ָ��
//#3���ݹ�������1����Դ
//�����ƶ����������ڣ����������õ�����±��ͷţ����ұ�֤����Ϊ0ʱ������ָ��������ͷ�ʱ���ͷ�

template <typename T>
class SMARTPOINTER2
{
private:
	RefPTR<T> *pRef;//��������������Ϊָ�룬ʹ������ͬһָ��Ķ�������ɹ���
	//�����������Ա
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
	inline bool		IsRefNull() const;//pRef��NULL�������ü���=0������T* p=NULL
	inline bool		IsDataNull() const;
	inline const T*	GetDataConst() const;//��ü�����ָ��
	inline T*		GetData() const;//��ü�����ָ��
	bool			SetData(T *pinfo);//��ȫ�����һ��T*ָ��ֻ�ܸ�1������ֵ
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
	//����жϣ���ֹ�ⲿ�����������Ϊ�������캯������
	if (&obj != NULL && this != &obj)//TODO:�����ж��Ƿ���Ҫ��
	{
		//��ֵ���ü�����һ
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//��ȷ�������ֵ����ֹδ��ʼ����������Ϊ0�����󿽱���release����refҰָ��
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//��������
		pRef = obj.pRef;
	}
	else
	{
		//�ⲿû�ṩ���Լ�����
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
			pRef = new RefPTR<T>();// �½��� ref�����ԭ�����õİ�
	}
}

template<typename T>
inline SMARTPOINTER2<T> & SMARTPOINTER2<T>::Assign(const SMARTPOINTER2<T>& obj)
{
	if (&obj != NULL && this != &obj)// TODO:ǰ����ж��Ƿ���Ҫ��
	{
		// ��ֵ�����ü�����һ
		if (obj.pRef != NULL)
		{
			obj.pRef->addref();
			//��ȷ�������ֵ����ֹδ��ʼ����������Ϊ0�����󿽱���release����refҰָ��
			if (obj.pRef->GetCount() == 1)
				obj.pRef->addref();
		}

		//���������ͷţ���ֵ���ü�����һ��
		Release();

		//��������
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
	if (pRef == NULL)//refû����
	{
		pRef = new RefPTR<T>();//���½���
		if (pRef == NULL)
			return false;
	}

	if (newp != pRef->GetData())
	{
		if (pRef->GetCount() > 1)
		{
			//дʱ��������������һ����ֵ������Ϊ1
			/*pRef->unref();
			pRef = new RefPTR<T>(newp);
			if (pRef == NULL)
				return false;*/

			//[��ʹ�ÿ���]�޸�ȫ��ֵ�������������߸���
			pRef->ReleaseData();
			pRef->SetData(newp);
		}
		else if (pRef->GetCount() == 1)//������һ�ݿ���
		{
			//����ɵģ���ֵ�µģ�������������
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
		if (pRef->GetCount() == 0)//��������
		{
			pRef->SetCount(1);
		}
	}

	return false;
}
