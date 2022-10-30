// MathLibrary.cpp : Defines the exported functions for the DLL.
#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <utility>
#include <limits.h>
#include "FactorialDll.h"
#include <cmath>
#include <exception>

char name[] = "!";
bool isBinary = false;
int priority = 3;


double func(double x) {
    if (x - int(x) != 0)
        throw std::exception("factorial is only for integers\n");
    else {
        int y = 1;
        for(int i = 1; i <= int(x); ++i) {
            y = y * i;
        }
        return y;
    }  
}