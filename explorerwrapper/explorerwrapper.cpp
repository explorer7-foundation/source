// explorerwrapper.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include "explorerwrapper.h"


// Пример экспортированной переменной
EXPLORERWRAPPER_API int nexplorerwrapper=0;

// Пример экспортированной функции.
EXPLORERWRAPPER_API int fnexplorerwrapper(void)
{
	return 42;
}

// Конструктор для экспортированного класса.
// см. определение класса в explorerwrapper.h
Cexplorerwrapper::Cexplorerwrapper()
{
	return;
}
