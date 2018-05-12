// RefCount.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iomanip>
#include "ref.h"


using std::cout;
using std::cerr;

typedef SMARTPOINTER2<int> INTPTR;

void info(INTPTR **obj, char strShow[] = "", int nObjs = 1)
{
	std::cout << strShow << endl;

	for (int i = 0; i < nObjs; i++)
	{
		if (obj[i] != NULL)
		{
			std::cout << obj[i]->GetRefCount()<<',';
			cout << obj[i]->GetDataConst();
		}
		else
			std::cout << '*';
		std::cout << "\t";
	}

	std::cout << endl;
}

void testSP(INTPTR iptr)
{
	;
}

int main()
{
	/*INTPTR intptr2[5];
	INTPTR **p2 = new INTPTR*[5];
	if (p2 == NULL)
	{
		cerr << "INTPTR ** failed" << endl;
		return 0;
	}
	for (int i = 0; i < 5; i++)
	{
		p2[i] = &intptr2[i];
	}

	int *tt = new int;
	*tt = 4;
	info(p2, "init", 5);

	intptr2[1].SetData(tt);
	info(p2, "2setdata", 5);

	tt = NULL;
	intptr2[0] = intptr2[1];
	info(p2, "2=1  tt=NULL", 5);

	intptr2[0].Release();
	info(p2, "1release", 5);

	intptr2[0].SetData(tt);
	info(p2, "1setdata", 5);

	intptr2[0].Release();
	info(p2, "1release", 5);

	intptr2[1].Release();
	info(p2, "2release", 5);*/


	int *ss = new int;
	*ss = 1234;
	INTPTR *intptr[5] = { 0 };
	intptr[0] = new INTPTR;
	info(intptr, "0初始化", 5);
	intptr[1] = new INTPTR(*intptr[0]);
	info(intptr, "1用0初始化 ", 5);
	SAFE_RELEASE(intptr[0]);
	info(intptr, "0释放", 5);
	SAFE_RELEASE(intptr[1]);
	info(intptr, "1释放", 5);
	intptr[2] = new INTPTR(*intptr[3]);
	intptr[3] = new INTPTR;
	intptr[4] = new INTPTR;
	//intptr[2]->Assign(intptr[3]);
	info(intptr, "2用3初始化，3、4初始化", 5);
	*intptr[3] = *intptr[4];
	info(intptr, "4赋值给3", 5);
	intptr[3]->SetData(ss);
	info(intptr, "修改3数据指针", 5);
	for (int i = 0; i < 5; i++)
	{
		SAFE_RELEASE(intptr[i]);
		char strShow[64] = { 0 };
		sprintf_s(strShow, "%dRelease()", i);
		info(intptr, strShow, 5);
	}

	system("pause");
    return 0;
}

