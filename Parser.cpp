//
// Created by roman on 15.06.15.
//

#include "Parser.h"
#include <cassert>
using namespace std;

void parsing_error(string error, size_t line)
{
    throw runtime_error("Error on line " + to_string(line) + ": " + error);
}

bool OneTokenNode::feed(SyntaxStack &st, const Token &tok)
{
    if (tok.type != acceptedToken())
        parsing_error(prettyPrintTokType(acceptedToken()) + " expected, \"" + tok.content + "\" found instead", tok.line);

    line = tok.line;

    tokenContent = tok.content;
    return true;
}

SyntaxNodePtr parseInput(SyntaxNodePtr target, std::vector<Token> tokens)
{
    tokens.push_back(Token(TokenType::eof, "end of file", tokens.back().line));
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

    if (token != tokens.end() && token->type != TokenType::eof)
        parsing_error("End of input expected, \"" + token->content + "\" found instead", token->line);

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

    void pushListToStack(SyntaxStack& st, SyntaxNodeList lst)
    {
        for (auto it = lst.rbegin(); it != lst.rend(); it++)
            st.push_front(*it);
    }

    std::string dumpType(DataType type)
    {
        return type == DataType::Integer ? "integer" : type == DataType::Float ? "float" : type == DataType::Bool ? "bool" : type == DataType::None ? "none" : "invalid";
    }

    std::map<tuple<DataType, std::string, DataType>, DataType > compatibilityMatrix {
            { make_tuple(DataType::Integer, "<", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Integer, ">", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Integer, "<=", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Integer, ">=", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Integer, "<>", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Integer, "=", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Integer, "+", DataType::Integer), DataType::Integer },
            { make_tuple(DataType::Integer, "-", DataType::Integer), DataType::Integer },
            { make_tuple(DataType::Integer, "*", DataType::Integer), DataType::Integer },
            { make_tuple(DataType::Integer, "/", DataType::Integer), DataType::Integer },
            { make_tuple(DataType::Integer, "and", DataType::Integer), DataType::Integer },
            { make_tuple(DataType::Integer, "or", DataType::Integer), DataType::Integer },

            { make_tuple(DataType::Float, "<", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, ">", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, "<=", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, ">=", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, "<>", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, "=", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, "+", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, "-", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, "*", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Float, "/", DataType::Float), DataType::Bool },

            { make_tuple(DataType::Bool, "and", DataType::Bool), DataType::Bool },
            { make_tuple(DataType::Bool, "or", DataType::Bool), DataType::Bool },
            { make_tuple(DataType::Bool, "=", DataType::Bool), DataType::Bool },
            { make_tuple(DataType::Bool, "<>", DataType::Bool), DataType::Bool },

            { make_tuple(DataType::Float, "<", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Float, ">", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Float, "<=", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Float, ">=", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Float, "<>", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Float, "=", DataType::Integer), DataType::Bool },
            { make_tuple(DataType::Float, "+", DataType::Integer), DataType::Float },
            { make_tuple(DataType::Float, "-", DataType::Integer), DataType::Float },
            { make_tuple(DataType::Float, "*", DataType::Integer), DataType::Float },
            { make_tuple(DataType::Float, "/", DataType::Integer), DataType::Float },

            { make_tuple(DataType::Integer, "<", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Integer, ">", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Integer, "<=", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Integer, ">=", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Integer, "<>", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Integer, "=", DataType::Float), DataType::Bool },
            { make_tuple(DataType::Integer, "+", DataType::Float), DataType::Float },
            { make_tuple(DataType::Integer, "-", DataType::Float), DataType::Float },
            { make_tuple(DataType::Integer, "*", DataType::Float), DataType::Float },
            { make_tuple(DataType::Integer, "/", DataType::Float), DataType::Float },
    };

    bool checkTypeCompatibility(std::string operation, DataType op1, DataType op2)
    {
        return operation == "" || compatibilityMatrix.count(make_tuple(op1, operation, op2)) > 0;
    }

    set<string> bool_operations = { "<", ">", "<>", "" };

    DataType getResultType(std::string operation, DataType t1, DataType t2)
    {
        return operation == "" ? t1 : compatibilityMatrix.find(make_tuple(t1, operation, t2))->second;
    }
}

bool TransformableNode::feed(SyntaxStack &st, const Token &tok)
{
    auto t_map = transformationMap();
    auto transformation = t_map.find(tok.type);

    if (transformation == t_map.end())
        transformation = t_map.find(TokenType::any);

    if (transformation == t_map.end())
        parsing_error("Unexpected token \"" + tok.content + "\". Expected token types: " + listTokenTypes(t_map), tok.line);

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

std::string NodeWithSubnodes::dump(int shift)
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
    line = tok.line;
    return false;
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

void OneTokenNode::semanticProcess(SemanticContext &context)
{
}

void SemanticContext::declareVariable(std::string name, DataType type, size_t line)
{
    if (variables.find(name) != variables.end())
        parsing_error("Variable "  + name + " is redeclared", line);

    variables[name] = type;
}

void IdentifierNode::semanticProcess(SemanticContext &context)
{
    type = context.getVariableType(tokenContent, line);
}

DataType SemanticContext::getVariableType(std::string name, size_t line)
{
    if (variables.find(name) == variables.end())
        parsing_error("Variable " + name + " is undeclared", line);

    return variables[name];
}

void NodeWithSubnodes::semanticProcess(SemanticContext &context)
{
    for (auto node: subNodes)
        node->semanticProcess(context);
}

SyntaxNodePtr parseInputWithSemantic(SyntaxNodePtr target, std::string code)
{
    SemanticContext context;
    parseInput(target, lexString(code))->semanticProcess(context);
    return target;
}

void DeclarationNode::semanticProcess(SemanticContext &context)
{
    IdentifierListNode* identifierListNode = dynamic_cast<IdentifierListNode*>(subNodes[1].get());
    TypeNode* typeNode = dynamic_cast<TypeNode*>(subNodes[2].get());

    assert(identifierListNode);
    assert(typeNode);

    for (auto ident : identifierListNode->gatherIdentifiers())
    {
        context.declareVariable(ident, typeNode->getType(), line);
    }
    NodeWithSubnodes::semanticProcess(context);
}

DataType TypeNode::getType()
{
    if (tokenContent == "integer")
        return DataType::Integer;
    else if (tokenContent == "float")
        return DataType::Float;
    else if (tokenContent == "bool")
        return DataType::Bool;
    else
        throw runtime_error("Unknown type " + tokenContent);
}

void IdentifierListNode::gatherIdentifiers(std::list<std::string> &identifiers)
{
    IdentifierNode* identifierNode = dynamic_cast<IdentifierNode*>(subNodes[0].get());
    IdentifierListTailNode* identifierListTailNode = dynamic_cast<IdentifierListTailNode*>(subNodes[1].get());

    assert(identifierNode);
    assert(identifierListTailNode);

    identifiers.push_back(identifierNode->getContent());
    identifierListTailNode->gatherIdentifiers(identifiers);
}

void IdentifierListTailNode::gatherIdentifiers(std::list<std::string> &identifiers)
{
    if (subNodes.size() > 1)
    {
        IdentifierListNode* identifierListNode = dynamic_cast<IdentifierListNode*>(subNodes[1].get());
        assert(identifierListNode);
        identifierListNode->gatherIdentifiers(identifiers);
    }
}

std::list<std::string> IdentifierListNode::gatherIdentifiers()
{
    std::list<std::string> result;
    gatherIdentifiers(result);
    return result;
}

void NumberNode::semanticProcess(SemanticContext &context)
{
    IntNumberNode* intNumberNode = dynamic_cast<IntNumberNode*>(subNodes[0].get());
    FloatNumberNode* floatNumberNode = dynamic_cast<FloatNumberNode*>(subNodes[0].get());

    assert(intNumberNode || floatNumberNode);

    if (intNumberNode)
        type = intNumberNode->getType();
    else
        type = floatNumberNode->getType();

    NodeWithSubnodes::semanticProcess(context);
}

void FactorNode::semanticProcess(SemanticContext &context)
{
    /*
     * { TokenType::identifier, SyntaxNodeList { make_shared<IdentifierNode>() } },
            { TokenType::int_number, SyntaxNodeList { make_shared<NumberNode>() } },
            { TokenType::float_number, SyntaxNodeList { make_shared<NumberNode>() } },
            { TokenType::bool_const, SyntaxNodeList { make_shared<BoolConstNode>() } },
            { TokenType::un_op, SyntaxNodeList { make_shared<UnaryOperationNode>(), make_shared<FactorNode>() } },
            { TokenType::openbr, SyntaxNodeList { make_shared<OpenBraceNode>(), make_shared<ExpressionNode>(), make_shared<CloseBraceNode>() } }
     */
    NodeWithSubnodes::semanticProcess(context);

    WithType* subNode = dynamic_cast<IdentifierNode*>(subNodes[0].get());
    if (!subNode) subNode = dynamic_cast<NumberNode*>(subNodes[0].get());
    if (!subNode) subNode = dynamic_cast<BoolConstNode*>(subNodes[0].get());
    if (!subNode) subNode = subNodes.size() > 1 ? dynamic_cast<OperandNode*>(subNodes[1].get()) : 0;
    if (!subNode) subNode = subNodes.size() > 1 ? dynamic_cast<ExpressionNode*>(subNodes[1].get()) : 0;

    assert(subNode);

    UnaryOperationNode* unaryOp = dynamic_cast<UnaryOperationNode*>(subNodes[0].get());
    if (unaryOp && subNode->getType() == DataType::Float)
        throw "\"not\" operation can't be applied to float";

    type = subNode->getType();
}

void ExpressionNode::semanticProcess(SemanticContext &context)
{
    NodeWithSubnodes::semanticProcess(context);

    OperandNode* operandNode = dynamic_cast<OperandNode*>(subNodes[0].get());
    ExpressionTailNode* expressionTailNode = dynamic_cast<ExpressionTailNode*>(subNodes[1].get());

    assert(operandNode);
    assert(expressionTailNode);

    if (!checkTypeCompatibility(expressionTailNode->getOperation(), operandNode->getType(), expressionTailNode->getType()))
        parsing_error("Types " + dumpType(operandNode->getType()) + " and " + dumpType(expressionTailNode->getType())
                            + " are not compatible for " + expressionTailNode->getOperation() + " operation", line);

    if (expressionTailNode->getOperation() == "")
        type = operandNode->getType();
    else
        type = DataType::Bool;
}

std::string TailNode::getOperation()
{
    OneTokenNode* oneTokenNode = subNodes.size() > 0 ? dynamic_cast<OneTokenNode*>(subNodes[0].get()) : 0;
    return oneTokenNode ? oneTokenNode->getContent() : "";
}

void AssignmentNode::semanticProcess(SemanticContext &context)
{
    NodeWithSubnodes::semanticProcess(context);

    IdentifierNode* identifierNode = dynamic_cast<IdentifierNode*>(subNodes[0].get());
    ExpressionNode* expressionNode = dynamic_cast<ExpressionNode*>(subNodes[2].get());

    assert(identifierNode);
    assert(expressionNode);

    DataType type1 = identifierNode->getType(), type2 = expressionNode->getType();

    if (type1 != type2)
        if (!(type1 == DataType::Float && type2 == DataType::Integer))
            parsing_error("Can't assign value of type " + dumpType(type2) + " to a variable of type " + dumpType(type1), line);
}

void OperandNode::semanticProcess(SemanticContext &context)
{
    NodeWithSubnodes::semanticProcess(context);

    AddendNode* addendNode = dynamic_cast<AddendNode*>(subNodes[0].get());
    OperandTailNode* operandTailNode = dynamic_cast<OperandTailNode*>(subNodes[1].get());

    assert(addendNode);
    assert(operandTailNode);

    std::string operation = operandTailNode->getOperation();
    DataType t1 = addendNode->getType();
    DataType t2 = operandTailNode->getType();

    if (!checkTypeCompatibility(operation, t1, t2))
        parsing_error("Types " + dumpType(t1) + " and " + dumpType(t2)
                            + " are not compatible for " + operation + " operation", line);

    type = getResultType(operation, t1, t2);
}

void AddendNode::semanticProcess(SemanticContext &context)
{
    NodeWithSubnodes::semanticProcess(context);

    FactorNode* factorNode = dynamic_cast<FactorNode*>(subNodes[0].get());
    AddendTailNode* addendTailNode = dynamic_cast<AddendTailNode*>(subNodes[1].get());

    assert(factorNode);
    assert(addendTailNode);

    std::string operation = addendTailNode->getOperation();
    DataType t1 = factorNode->getType();
    DataType t2 = addendTailNode->getType();

    if (!checkTypeCompatibility(operation, t1, t2))
        parsing_error("Types " + dumpType(t1) + " and " + dumpType(t2)
                            + " are not compatible for " + operation + " operation", line);

    type = getResultType(operation, t1, t2);
}

void ExpressionTailNode::semanticProcess(SemanticContext &context)
{
    NodeWithSubnodes::semanticProcess(context);

    if (subNodes.size() == 0)
        type = DataType::None;
    else
        type = dynamic_cast<ExpressionNode*>(subNodes[1].get())->getType();
}

void OperandTailNode::semanticProcess(SemanticContext &context)
{
    NodeWithSubnodes::semanticProcess(context);

    if (subNodes.size() == 0)
        type = DataType::None;
    else
        type = dynamic_cast<OperandNode*>(subNodes[1].get())->getType();
}

void AddendTailNode::semanticProcess(SemanticContext &context)
{
    NodeWithSubnodes::semanticProcess(context);

    if (subNodes.size() == 0)
        type = DataType::None;
    else
        type = dynamic_cast<AddendNode*>(subNodes[1].get())->getType();
}

std::string SyntaxNode::dumpInternal()
{
    WithType* thisWithType = dynamic_cast<WithType*>(this);
    if (thisWithType)
        return className() + "(type = " + dumpType(thisWithType->getType()) + ")" + "\n";
    else
        return className() + "\n";
}

std::string OneTokenNode::dumpInternal()
{
    WithType* thisWithType = dynamic_cast<WithType*>(this);
    if (thisWithType)
        return className() + " { " + tokenContent + " } " + "(type = " + dumpType(thisWithType->getType()) + ")" + "\n";
    else
        return className() + className() + " { " + tokenContent + " }\n";
}
