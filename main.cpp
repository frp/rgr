#include <iostream>
#include "Parser.h"
using namespace std;

int main()
{
    //REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "dim a,b,c integer : a : b : c"));
    try
    {
        cout << parseInputWithSemantic(make_shared<ProgramNode>(), "dim a,b float : a as 3*(2+4)")->dump(0) << endl;
    }
    catch(exception& e)
    {
        cout << e.what() << endl;
    }
    return 0;
}