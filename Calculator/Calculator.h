#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <list>
#include "Operation.h"

using std::vector;
using std::string;
using std::list;

class Calculator {
public:
    vector<Operation*> operations;
    string expression;
    list<string> polish_notation;
    list<HMODULE> dll_files;

public:
    Calculator();
    ~Calculator();
    void SetExpression(string const& str);
    void RemoveGaps();
    void MakePolishNotation();
    void SolvePolishNotation();
    void ImportDll();
};
