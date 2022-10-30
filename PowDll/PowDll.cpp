// MathLibrary.cpp : Defines the exported functions for the DLL.
#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <utility>
#include <limits.h>
#include "PowDll.h"
#include <cmath>

char name[] = "pow";
bool isBinary = true;
int priority = 3;


double func(double x, double y) {
    return pow(x, y);
}