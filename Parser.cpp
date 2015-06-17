//
// Created by roman on 15.06.15.
//

#include "Parser.h"
#include <cassert>
using namespace std;

bool OneTokenNode::feed(SyntaxStack &st, const Token &tok)
{
    if (tok.type != acceptedToken())
        throw runtime_error(prettyPrintTokType(acceptedToken()) + " expected, \"" + tok.content + "\" foud instead");

    tokenContent = tok.content;
    return true;
}

SyntaxNodePtr parseInput(SyntaxNodePtr target, std::vector<Token> tokens)
{
    tokens.push_back(Token(TokenType::eof, "", tokens.back().line));
    SyntaxStack stack;
    stack.push_front(target);

    auto token = tokens.begin();

    while (!stack.empty())
    {
        assert (token != tokens.end());

        SyntaxNodePtr node = stack.front();
        stack.pop_front();

        if (node->feed(stack, *token))
            token++;
    }

    while (token != tokens.end() && token->type == TokenType::eof)
        token++;

    if (token != tokens.end())
        throw runtime_error("End of input expected, \"" + token->content + "\" found instead");

    return target;
}

namespace
{
    template<class T>
    string listTokenTypes(map<TokenType, T> t_map)
    {
        string result;

        bool first = true;
        for (auto item : t_map)
        {
            if (first)
                first = false;
            else
                result += ", ";

            result += prettyPrintTokType(item.first);
        }

        return result;
    }

    void pushListToStack(SyntaxStack& st, list<SyntaxNodePtr> lst)
    {
        for (auto it = lst.rbegin(); it != lst.rend(); it++)
            st.push_front(*it);
    }
}

bool TransformableNode::feed(SyntaxStack &st, const Token &tok)
{
    auto t_map = transformationMap();
    auto transformation = t_map.find(tok.type);

    if (transformation == t_map.end())
        transformation = t_map.find(TokenType::any);

    if (transformation == t_map.end())
        throw runtime_error("Unexpected token \"" + tok.content + "\". Expected token types: " + listTokenTypes(t_map));

    pushListToStack(st, transformation->second);
    subNodes = transformation->second;

    return false;
}

TransformationMap NumberNode::transformationMap()
{
    return TransformationMap {
            { TokenType::int_number, SyntaxNodeList { make_shared<IntNumberNode>() } },
            { TokenType::float_number, SyntaxNodeList { make_shared<FloatNumberNode>() } }
    };
}

std::string SyntaxNode::dump(int shift)
{
    string result;
    for (int i = 0; i < shift; ++i)
        result += '\t';

    return result + dumpInternal();
}

std::string TransformableNode::dump(int shift)
{
    string result;
    for (int i = 0; i < shift; ++i)
        result += '\t';

    result += dumpInternal();

    for (auto node : subNodes)
        result += node->dump(shift + 1);

    return result;
}

TransformationMap FactorNode::transformationMap()
{
    return TransformationMap{
            { TokenType::identifier, SyntaxNodeList { make_shared<IdentifierNode>() } },
            { TokenType::int_number, SyntaxNodeList { make_shared<NumberNode>() } },
            { TokenType::float_number, SyntaxNodeList { make_shared<NumberNode>() } },
            { TokenType::bool_const, SyntaxNodeList { make_shared<BoolConstNode>() } },
            { TokenType::un_op, SyntaxNodeList { make_shared<UnaryOperationNode>(), make_shared<FactorNode>() } },
            { TokenType::openbr, SyntaxNodeList { make_shared<OpenBraceNode>(), make_shared<ExpressionNode>(), make_shared<CloseBraceNode>() } }
    };
}

SyntaxNodeList AddendNode::expand()
{
    return SyntaxNodeList { make_shared<FactorNode>(), make_shared<AddendTailNode>() };
}

std::set<TokenType> AddendTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::mul_op } ;
}

SyntaxNodeList AddendTailNode::expand()
{
    return SyntaxNodeList { make_shared<MulOperationNode>(), make_shared<AddendNode>() };
}

bool TailNode::feed(SyntaxStack &st, const Token &tok)
{
    auto tokens = acceptedTokens();

    if (tokens.find(tok.type) == tokens.end())
        return false;

    subNodes = expand();

    pushListToStack(st, subNodes);
    return false;
}

bool ExpandableNode::feed(SyntaxStack &st, const Token &tok)
{
    subNodes = expand();
    pushListToStack(st, subNodes);
    return false;
}

std::string ExpandableNode::dump(int shift)
{
    string result;
    for (int i = 0; i < shift; ++i)
        result += '\t';

    result += dumpInternal();

    for (auto node : subNodes)
        result += node->dump(shift + 1);

    return result;
}

std::string TailNode::dump(int shift)
{
    string result;
    for (int i = 0; i < shift; ++i)
        result += '\t';

    result += dumpInternal();

    for (auto node : subNodes)
        result += node->dump(shift + 1);

    return result;
}

SyntaxNodeList OperandNode::expand()
{
    return SyntaxNodeList { make_shared<AddendNode>(), make_shared<OperandTailNode>() };
}

std::set<TokenType> OperandTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::add_op };
}

SyntaxNodeList OperandTailNode::expand()
{
    return SyntaxNodeList { make_shared<AddOperationNode>(), make_shared<OperandNode>() } ;
}

SyntaxNodeList ExpressionNode::expand()
{
    return SyntaxNodeList { make_shared<OperandNode>(), make_shared<ExpressionTailNode>() };
}

std::set<TokenType> ExpressionTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::relation_op };
}

SyntaxNodeList ExpressionTailNode::expand()
{
    return SyntaxNodeList { make_shared<RelationOperationNode>(), make_shared<ExpressionNode>() };
}

SyntaxNodeList DeclarationNode::expand()
{
    return SyntaxNodeList { make_shared<DimNode>(), make_shared<IdentifierListNode>(), make_shared<TypeNode>() };
}

SyntaxNodeList IdentifierListNode::expand()
{
    return SyntaxNodeList { make_shared<IdentifierNode>(), make_shared<IdentifierListTailNode>() };
}

std::set<TokenType> IdentifierListTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::comma };
}

SyntaxNodeList IdentifierListTailNode::expand()
{
    return SyntaxNodeList { make_shared<CommaNode>(), make_shared<IdentifierListNode>() };
}

SyntaxNodeList AssignmentNode::expand()
{
    return SyntaxNodeList { make_shared<IdentifierNode>(), make_shared<AsNode>(), make_shared<ExpressionNode>() };
}

SyntaxNodeList ConditionNode::expand()
{
    return SyntaxNodeList { make_shared<IfNode>(), make_shared<ExpressionNode>(), make_shared<ThenNode>(), make_shared<OperatorNode>(), make_shared<ConditionTailNode>() };
}

SyntaxNodeList ReadingNode::expand()
{
    return SyntaxNodeList { make_shared<ReadNode>(), make_shared<OpenBraceNode>(), make_shared<IdentifierListNode>(), make_shared<CloseBraceNode>() };
}

SyntaxNodeList WritingNode::expand()
{
    return SyntaxNodeList { make_shared<WriteNode>(), make_shared<OpenBraceNode>(), make_shared<ExpressionListNode>(), make_shared<CloseBraceNode>() };
}

SyntaxNodeList ExpressionListNode::expand()
{
    return SyntaxNodeList { make_shared<ExpressionNode>(), make_shared<ExpressionListTailNode>() };
}

std::set<TokenType> ExpressionListTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::comma };
}

SyntaxNodeList ExpressionListTailNode::expand()
{
    return SyntaxNodeList { make_shared<CommaNode>(), make_shared<ExpressionListNode>() };
}

TransformationMap OperatorNode::transformationMap()
{
    return TransformationMap {
            { TokenType::identifier, SyntaxNodeList { make_shared<AssignmentNode>() } },
            { TokenType::begin, SyntaxNodeList { make_shared<NestedOperatorNode>() } },
            { TokenType::if_, SyntaxNodeList { make_shared<ConditionNode>() } },
            { TokenType::for_, SyntaxNodeList { make_shared<ForLoopNode>() } },
            { TokenType::while_, SyntaxNodeList { make_shared<WhileLoopNode>() } },
            { TokenType::read_, SyntaxNodeList { make_shared<ReadingNode>() } },
            { TokenType::write_, SyntaxNodeList { make_shared<WritingNode>() } },
            { TokenType::op_separator, SyntaxNodeList { make_shared<OperatorSepNode>(), make_shared<OperatorNode>() } },
    };
}

std::set<TokenType> ConditionTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::else_ };
}

SyntaxNodeList ConditionTailNode::expand()
{
    return SyntaxNodeList { make_shared<ElseNode>(), make_shared<OperatorNode>() };
}

SyntaxNodeList ForLoopNode::expand()
{
    return SyntaxNodeList { make_shared<ForNode>(), make_shared<AssignmentNode>(), make_shared<ToNode>(), make_shared<ExpressionNode>(), make_shared<DoNode>(), make_shared<OperatorNode>() };
}

SyntaxNodeList WhileLoopNode::expand()
{
    return SyntaxNodeList { make_shared<WhileNode>(), make_shared<ExpressionNode>(), make_shared<DoNode>(), make_shared<OperatorNode>() };
}

SyntaxNodeList NestedOperatorNode::expand()
{
    return SyntaxNodeList { make_shared<BeginNode>(), make_shared<OperatorListNode>(), make_shared<EndNode>() };
}

SyntaxNodeList OperatorListNode::expand()
{
    return SyntaxNodeList { make_shared<OperatorNode>(), make_shared<OperatorListTailNode>() };
}

std::set<TokenType> OperatorListTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::op_separator };
}

SyntaxNodeList OperatorListTailNode::expand()
{
    return SyntaxNodeList { make_shared<OperatorSepNode>(), make_shared<OperatorListNode>() };
}

SyntaxNodeList ProgramNode::expand()
{
    return SyntaxNodeList { make_shared<ProgramItemNode>(), make_shared<ProgramTailNode>() };
}

TransformationMap ProgramItemNode::transformationMap()
{
    return TransformationMap {
            { TokenType::dim, SyntaxNodeList { make_shared<DeclarationNode>() } },
            { TokenType::any, SyntaxNodeList { make_shared<OperatorNode>() } },
    };
}

std::set<TokenType> ProgramTailNode::acceptedTokens()
{
    return std::set<TokenType> { TokenType::op_separator };
}

SyntaxNodeList ProgramTailNode::expand()
{
    return SyntaxNodeList { make_shared<OperatorSepNode>(), make_shared<ProgramNode>() };
}
