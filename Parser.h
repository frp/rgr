//
// Created by roman on 15.06.15.
//

#ifndef RGR_PARSER_H
#define RGR_PARSER_H

#include <memory>
#include <list>
#include <map>
#include <set>
#include "Lexer.h"

class SyntaxNode;

typedef std::shared_ptr<SyntaxNode> SyntaxNodePtr;
typedef std::list<SyntaxNodePtr> SyntaxStack;

enum class DataType { None, Int, Float, Bool, Invalid };

class SemanticContext
{
private:
    std::map<std::string, DataType> variables;
public:
    void declareVariable(std::string name, DataType type, size_t line);
    DataType getVariableType(std::string name, size_t line);
};

class WithType
{
protected:
    DataType type;
public:
    WithType() { type = DataType::Invalid; }
    DataType getType() { return type; }
};

class SyntaxNode
{
protected:
    virtual std::string className() { return "SyntaxNode"; }
    virtual std::string dumpInternal();
public:
    virtual ~SyntaxNode() {}
    virtual bool feed(SyntaxStack& st, const Token& tok) = 0;

    virtual std::string dump(int shift = 0);
    virtual void semanticProcess(SemanticContext &context) = 0;
};

class OneTokenNode : public SyntaxNode
{
protected:
    virtual TokenType acceptedToken() = 0;
    size_t line;
    std::string tokenContent;

    virtual std::string className() { return "OneTokenNode"; }
    virtual std::string dumpInternal();
public:
    virtual bool feed(SyntaxStack& st, const Token& tok);
    virtual void semanticProcess(SemanticContext &context);
    std::string getContent() { return tokenContent; }
};

class IntNumberNode : public OneTokenNode, public WithType
{
protected:
    TokenType acceptedToken() { return TokenType::int_number; };
    virtual std::string className() { return "IntNumberNode"; }
public:
    IntNumberNode() { type = DataType::Int; }
    IntNumberNode(std::string number) { tokenContent = number; type = DataType::Int; }
};

class FloatNumberNode : public OneTokenNode, public WithType
{
protected:
    TokenType acceptedToken() { return TokenType::float_number; }
    virtual std::string className() { return "FloatNumberNode"; }
public:
    FloatNumberNode() { type = DataType::Float; }
    FloatNumberNode(std::string number) { tokenContent = number; type = DataType::Float; }
};

class IdentifierNode : public OneTokenNode, public WithType
{
protected:
    TokenType acceptedToken() { return TokenType::identifier; }

    virtual std::string className() { return "IdentifierNode"; }
public:
    IdentifierNode() {}
    IdentifierNode(std::string ident) { tokenContent = ident; }
    virtual void semanticProcess(SemanticContext &context);
};

typedef std::vector<SyntaxNodePtr> SyntaxNodeList;
typedef std::map<TokenType, SyntaxNodeList> TransformationMap;

class NodeWithSubnodes : public SyntaxNode
{
protected:
    SyntaxNodeList subNodes;
public:
    virtual std::string dump(int shift = 0);
    virtual void semanticProcess(SemanticContext &context);
};

class TransformableNode : public NodeWithSubnodes
{
protected:
    virtual TransformationMap transformationMap() = 0;
    virtual std::string className() { return "TransformableNode"; }
public:
    virtual bool feed(SyntaxStack& st, const Token& tok);
};

class NumberNode : public TransformableNode, public WithType
{
protected:
    virtual std::string className() { return "NumberNode"; }
    virtual TransformationMap transformationMap();
public:
    NumberNode() {}
    NumberNode(SyntaxNodePtr innerNode) { subNodes.push_back(innerNode); }

    virtual void semanticProcess(SemanticContext &context);
};

class BoolConstNode : public OneTokenNode, public WithType
{
protected:
    TokenType acceptedToken() { return TokenType::bool_const; }
    virtual std::string className() { return "BoolConstNode"; }
public:
    BoolConstNode() { type = DataType::Bool; }
    BoolConstNode(std::string content) { tokenContent = content; type = DataType::Bool; }
};

class FactorNode : public TransformableNode, public WithType
{
protected:
    virtual TransformationMap transformationMap();
    virtual std::string className() { return "FactorNode"; }
public:
    FactorNode() {}
    FactorNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class UnaryOperationNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::un_op; }
    virtual std::string className() { return "UnaryOperationNode"; }
public:
    UnaryOperationNode() {}
    UnaryOperationNode(std::string content) { tokenContent = content; }
};

class ExpandableNode : public NodeWithSubnodes
{
protected:
    virtual SyntaxNodeList expand() = 0;
    virtual std::string className() { return "ExpandableNode"; }
    size_t line;
public:
    virtual bool feed(SyntaxStack& st, const Token& tok);
};

class AddendNode : public ExpandableNode, public WithType
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "AddendNode"; }
public:
    AddendNode() {}
    AddendNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class TailNode : public NodeWithSubnodes
{
protected:
    virtual std::set<TokenType> acceptedTokens() = 0;
    virtual SyntaxNodeList expand() = 0;
    virtual std::string className() { return "TailNode"; }
public:
    virtual bool feed(SyntaxStack& st, const Token& tok);

    std::string getOperation();
};

class AddendTailNode : public TailNode, public WithType
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "AddendTailNode"; }
public:
    AddendTailNode() {}
    AddendTailNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class MulOperationNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::mul_op; }
    virtual std::string className() { return "MulOperationNode"; }
public:
    MulOperationNode() {}
    MulOperationNode(std::string content) { tokenContent = content; }
};

class AddOperationNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::add_op; }
    virtual std::string className() { return "AddOperationNode"; }
public:
    AddOperationNode() {}
    AddOperationNode(std::string content) { tokenContent = content; }
};

class OperandNode : public ExpandableNode, public WithType
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "OperandNode"; }
public:
    OperandNode() {}
    OperandNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class OperandTailNode : public TailNode, public WithType
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "OperandTailNode"; }
public:
    OperandTailNode() {}
    OperandTailNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class RelationOperationNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::relation_op; }
    virtual std::string className() { return "RelationOperationNode"; }
public:
    RelationOperationNode() {}
    RelationOperationNode(std::string content) { tokenContent = content; }
};

class OpenBraceNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::openbr; }
    virtual std::string className() { return "OpenBraceNode"; }
public:
    OpenBraceNode() {}
    OpenBraceNode(std::string content) { tokenContent = content; }
};

class CloseBraceNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::closebr; }
    virtual std::string className() { return "CloseBraceNode"; }
public:
    CloseBraceNode() {}
    CloseBraceNode(std::string content) { tokenContent = content; }
};

class ExpressionNode : public ExpandableNode, public WithType
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ExpressionNode"; }
public:
    ExpressionNode() {}
    ExpressionNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class ExpressionTailNode : public TailNode, public WithType
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ExpressionTailNode"; }
public:
    ExpressionTailNode() {}
    ExpressionTailNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class DeclarationNode: public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "DeclarationNode"; }
public:
    DeclarationNode() {}
    DeclarationNode(SyntaxNodeList nodes) { subNodes = nodes; }
    virtual void semanticProcess(SemanticContext &context);
};

class TypeNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::type; }
    virtual std::string className() { return "TypeNode"; }
public:
    TypeNode() {}
    TypeNode(std::string content) { tokenContent = content; }
    DataType getType();
};

class IdentifierListNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "IdentifierListNode"; }
public:
    IdentifierListNode() {}
    IdentifierListNode(SyntaxNodeList nodes) { subNodes = nodes; }
    void gatherIdentifiers(std::list<std::string>& identifiers);
    std::list<std::string> gatherIdentifiers();
};

class IdentifierListTailNode : public TailNode
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "IdentifierListTailNode"; }
public:
    IdentifierListTailNode() {}
    IdentifierListTailNode(SyntaxNodeList nodes) { subNodes = nodes; }

    void gatherIdentifiers(std::list<std::string>& identifiers);
};

class CommaNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::comma; }
    virtual std::string className() { return "CommaNode"; }
public:
    CommaNode() {}
    CommaNode(std::string content) { tokenContent = content; }
};

class AssignmentNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "AssignmentNode"; }
public:
    AssignmentNode() {}
    AssignmentNode(SyntaxNodeList nodes) { subNodes = nodes; }

    virtual void semanticProcess(SemanticContext &context);
};

class AsNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::as_; }
    virtual std::string className() { return "AsNode"; }
public:
    AsNode() {}
    AsNode(std::string content) { tokenContent = content; }
};

class ConditionNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ConditionNode"; }
public:
    ConditionNode() {}
    ConditionNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class ReadingNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ReadingNode"; }
public:
    ReadingNode() {}
    ReadingNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class ReadNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::read_; }
    virtual std::string className() { return "ReadNode"; }
public:
    ReadNode() {}
    ReadNode(std::string content) { tokenContent = content; }
};

class WritingNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "WritingNode"; }
public:
    WritingNode() {}
    WritingNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class WriteNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::write_; }
    virtual std::string className() { return "WriteNode"; }
public:
    WriteNode() {}
    WriteNode(std::string content) { tokenContent = content; }
};

class ExpressionListNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ExpressionListNode"; }
public:
    ExpressionListNode() {}
    ExpressionListNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class ExpressionListTailNode : public TailNode
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ExpressionListTailNode"; }
public:
    ExpressionListTailNode() {}
    ExpressionListTailNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class IfNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::if_; }
    virtual std::string className() { return "IfNode"; }
public:
    IfNode() {}
    IfNode(std::string content) { tokenContent = content; }
};

class ThenNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::then_; }
    virtual std::string className() { return "ThenNode"; }
public:
    ThenNode() {}
    ThenNode(std::string content) { tokenContent = content; }
};

class ElseNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::else_; }
    virtual std::string className() { return "ElseNode"; }
public:
    ElseNode() {}
    ElseNode(std::string content) { tokenContent = content; }
};


class OperatorNode : public TransformableNode
{
protected:
    virtual std::string className() { return "OperatorNode"; }
    virtual TransformationMap transformationMap();
public:
    OperatorNode() {}
    OperatorNode(SyntaxNodePtr innerNode) { subNodes.push_back(innerNode); }
};

class ConditionTailNode : public TailNode
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ConditionTailNode"; }
public:
    ConditionTailNode() {}
    ConditionTailNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class ForLoopNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ForLoopNode"; }
public:
    ForLoopNode() {}
    ForLoopNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class ForNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::for_; }
    virtual std::string className() { return "ForNode"; }
public:
    ForNode() {}
    ForNode(std::string content) { tokenContent = content; }
};

class ToNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::to_; }
    virtual std::string className() { return "ToNode"; }
public:
    ToNode() {}
    ToNode(std::string content) { tokenContent = content; }
};

class DoNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::do_; }
    virtual std::string className() { return "DoNode"; }
public:
    DoNode() {}
    DoNode(std::string content) { tokenContent = content; }
};

class WhileLoopNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "WhileLoopNode"; }
public:
    WhileLoopNode() {}
    WhileLoopNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class WhileNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::while_; }
    virtual std::string className() { return "WhileNode"; }
public:
    WhileNode() {}
    WhileNode(std::string content) { tokenContent = content; }
};

class NestedOperatorNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "NestedOperatorNode"; }
public:
    NestedOperatorNode() {}
    NestedOperatorNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class BeginNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::begin; }
    virtual std::string className() { return "BeginNode"; }
public:
    BeginNode() {}
    BeginNode(std::string content) { tokenContent = content; }
};

class ProgramTNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::program; }
    virtual std::string className() { return "ProgramTNode"; }
public:
    ProgramTNode() {}
    ProgramTNode(std::string content) { tokenContent = content; }
};

class VarNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::var_; }
    virtual std::string className() { return "VarNode"; }
public:
    VarNode() {}
    VarNode(std::string content) { tokenContent = content; }
};

class EndNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::end; }
    virtual std::string className() { return "EndNode"; }
public:
    EndNode() {}
    EndNode(std::string content) { tokenContent = content; }
};

class OperatorSepNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::op_separator; }
    virtual std::string className() { return "OperatorSepNode"; }
public:
    OperatorSepNode() {}
    OperatorSepNode(std::string content) { tokenContent = content; }
};

class EndWithDotNode : public OneTokenNode
{
protected:
    TokenType acceptedToken() { return TokenType::endwithdot; }
    virtual std::string className() { return "EndWithDotNode"; }
public:
    EndWithDotNode() {}
    EndWithDotNode(std::string content) { tokenContent = content; }
};

class OperatorListNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "OperatorListNode"; }
public:
    OperatorListNode() {}
    OperatorListNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class OperatorListTailNode : public TailNode
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "OperatorListTailNode"; }
public:
    OperatorListTailNode() {}
    OperatorListTailNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class ProgramNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "ProgramNode"; }
public:
    ProgramNode() {}
    ProgramNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class DeclarationListNode : public ExpandableNode
{
protected:
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "DeclarationListNode"; }
public:
    DeclarationListNode() {}
    DeclarationListNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

class DeclarationListTailNode : public TailNode
{
protected:
    virtual std::set<TokenType> acceptedTokens();
    virtual SyntaxNodeList expand();
    virtual std::string className() { return "DeclarationListTailNode"; }
public:
    DeclarationListTailNode() {}
    DeclarationListTailNode(SyntaxNodeList nodes) { subNodes = nodes; }
};

SyntaxNodePtr parseInput(SyntaxNodePtr target, std::vector<Token> tokens);
SyntaxNodePtr parseInputWithSemantic(SyntaxNodePtr target, std::string code);

#endif //RGR_PARSER_H
