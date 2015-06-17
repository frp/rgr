#include <iostream>
#include "Parser.h"
using namespace std;

int main()
{
    vector<Token> toks = lexString("write(a,b,c)");
    for (auto& tok: toks)
        cout << prettyPrintTokType(tok.type) << endl;

    cout << parseInput(make_shared<WritingNode>(), toks)->dump() << endl;
    return 0;
}