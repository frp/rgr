//
// Created by roman on 15.06.15.
//

#include "catch.hpp"
#include "Parser.h"

using namespace std;

TEST_CASE( "main test" )
{
    SECTION ("single token parsing tests")
    {
        REQUIRE(parseInput(make_shared<IntNumberNode>(), lexString("1"))->dump() == ((IntNumberNode {
                "1"}).dump()));
        REQUIRE(parseInput(make_shared<FloatNumberNode>(), lexString("1.23"))->dump() == ((FloatNumberNode {
                "1.23"}).dump()));
        REQUIRE(parseInput(make_shared<IdentifierNode>(), lexString("abc"))->dump() == ((IdentifierNode {
                "abc"}).dump()));
        REQUIRE(parseInput(make_shared<BoolConstNode>(), lexString("true"))->dump() == ((BoolConstNode {
                "true"}).dump()));

        REQUIRE_THROWS(parseInput(make_shared<IntNumberNode>(), lexString("1.23")));
        REQUIRE_THROWS(parseInput(make_shared<IdentifierNode>(), lexString("1argfds")));
    }

    SECTION ("simple nested nodes parsing tests")
    {
        REQUIRE(parseInput(make_shared<NumberNode>(), lexString("1"))->dump() ==
                (NumberNode {make_shared<IntNumberNode>("1")}).dump());

        REQUIRE(parseInput(make_shared<FactorNode>(), lexString("1"))->dump() ==
                (FactorNode(SyntaxNodeList {make_shared<NumberNode>(make_shared<IntNumberNode>("1"))})).dump());
        REQUIRE(parseInput(make_shared<FactorNode>(), lexString("1.23"))->dump() ==
                (FactorNode(SyntaxNodeList {make_shared<NumberNode>(make_shared<FloatNumberNode>("1.23"))})).dump());
        REQUIRE(parseInput(make_shared<FactorNode>(), lexString("ax"))->dump() ==
                (FactorNode(SyntaxNodeList {make_shared<IdentifierNode>("ax")})).dump());
        REQUIRE(parseInput(make_shared<FactorNode>(), lexString("true"))->dump() ==
                (FactorNode(SyntaxNodeList {make_shared<BoolConstNode>("true")})).dump());

        FactorNode nodeToTestFactorWithUnop(SyntaxNodeList
        {
            make_shared<UnaryOperationNode>("not"),
                    make_shared<FactorNode>(SyntaxNodeList {make_shared<BoolConstNode>("true")})
        }
        );
        REQUIRE(parseInput(make_shared<FactorNode>(), lexString("not true"))->dump() ==
                nodeToTestFactorWithUnop.dump());

        AddendNode nodeToTestAddend(SyntaxNodeList
        {
            make_shared<FactorNode>(SyntaxNodeList {make_shared<NumberNode>(make_shared<IntNumberNode>("1"))}),
                    make_shared<AddendTailNode>(SyntaxNodeList {make_shared<MulOperationNode>("*"),
                                                                make_shared<AddendNode>(SyntaxNodeList {
                                                                        make_shared<FactorNode>(SyntaxNodeList {
                                                                                make_shared<NumberNode>(
                                                                                        make_shared<IntNumberNode>(
                                                                                                "3"))}),
                                                                        make_shared<AddendTailNode>()})})
        });

        REQUIRE(parseInput(make_shared<AddendNode>(), lexString("1*3"))->dump() == nodeToTestAddend.dump());
    }

    SECTION ("simple tests")
    {
        REQUIRE_NOTHROW(parseInput(make_shared<ReadingNode>(), lexString("read(a,b,c)")));
        REQUIRE_THROWS(parseInput(make_shared<ReadingNode>(), lexString("read(a*b+c,b,c)")));
        REQUIRE_NOTHROW(parseInput(make_shared<WritingNode>(), lexString("write(a,b,c)")));
        REQUIRE_NOTHROW(parseInput(make_shared<WritingNode>(), lexString("write(a+b*c,b,c)")));

        REQUIRE_NOTHROW(parseInput(make_shared<ConditionNode>(), lexString("if a or b then b as a + 3")));
        REQUIRE_NOTHROW(
                parseInput(make_shared<ConditionNode>(), lexString("if a or b then b as a + 3 else c as a + 4")));

        REQUIRE_NOTHROW(parseInput(make_shared<ForLoopNode>(), lexString("for a as 3 to 5 do c as d + e")));

        REQUIRE_NOTHROW(parseInput(make_shared<WhileLoopNode>(), lexString("while a + b < 5 do c as c + 1")));
        REQUIRE_NOTHROW(parseInput(make_shared<NestedOperatorNode>(), lexString("begin a as 5 end")));
        REQUIRE_NOTHROW(
                parseInput(make_shared<NestedOperatorNode>(), lexString("begin while a + b < 5 do c as c + 1 end")));


        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("read(a,b,c)")));
        REQUIRE_THROWS(parseInput(make_shared<OperatorNode>(), lexString("read(a*b+c,b,c)")));
        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("write(a,b,c)")));
        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("write(a+b*c,b,c)")));

        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("if a or b then b as a + 3")));
        REQUIRE_NOTHROW(
                parseInput(make_shared<OperatorNode>(), lexString("if a or b then b as a + 3 else c as a + 4")));

        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("for a as 3 to 5 do c as d + e")));

        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("while a + b < 5 do c as c + 1")));
        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("begin a as 5 end")));
        REQUIRE_NOTHROW(parseInput(make_shared<OperatorNode>(), lexString("begin while a + b < 5 do c as c + 1 end")));

        REQUIRE_NOTHROW(parseInput(make_shared<ProgramNode>(),
                                   lexString("a: integer ; b: bool ; if a > b then a as a + b else b as a + b")));

        // check case when there's more than one operator separator
        REQUIRE_NOTHROW(parseInput(make_shared<ProgramNode>(),
                                   lexString("a: integer ; b: bool ; if a > b then a as a + b else b as a + b")));
    }

    SECTION ("identifier existence checks") {
        REQUIRE_THROWS(parseInputWithSemantic(make_shared<ProgramNode>(), "a"));
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a,b,c: integer"));
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a,b,c: integer ; read(a,b,c)"));
        REQUIRE_THROWS(parseInputWithSemantic(make_shared<ProgramNode>(), "a,b: integer ; read(a,b,c)"));
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a,b: integer ; read(a,b)"));
    }

    SECTION ("type checks on assignment") {
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a: integer ; a as 5"));
        REQUIRE_THROWS(parseInputWithSemantic(make_shared<ProgramNode>(), "a: integer ; a as 5.1"));
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a,b: integer ; a as 2+3"));
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a,b: bool ; a as a or b"));
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a,b: float ; a as 2+3.0*(2+4)"));
        REQUIRE_NOTHROW(parseInputWithSemantic(make_shared<ProgramNode>(), "a: bool ; b: integer ; a as b < 3 "));
        REQUIRE_THROWS(parseInputWithSemantic(make_shared<ProgramNode>(), "a: bool ; b: integer ; a as b - 3 "));
    }
}
