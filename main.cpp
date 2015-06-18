#include <fstream>
#include "Parser.h"

using namespace std;

int main()
{
    ifstream in("input.txt");
    ofstream out("ast.txt");
    if (!in)
    {
        cout << "Couldn't open input file\n";
        return 0;
    }
    if (!out)
    {
        cout << "Couldn't open output file\n";
        return 0;
    }
    try
    {
        string s, code;
        while (getline(in, s))
            code += s + "\n";

        for (auto& tok : lexString(code))
        {
            cout << prettyPrintTokType(tok.type) << "\n";
        }

        out << parseInputWithSemantic(make_shared<ProgramNode>(), code)->dump();

        cout << "Parsed successfully, output is dumped to ast.txt file";
    }
    catch(exception& e)
    {
        cout << e.what() << endl;
    }
    return 0;
}