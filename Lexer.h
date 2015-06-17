//
// Created by roman on 12.06.15.
//

#ifndef RGR_LEXER_H
#define RGR_LEXER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>

enum class TokenType { bool_const, dim, type, if_, then_, else_, for_, to_, do_, while_, read_, write_, relation_op, add_op,
    mul_op, un_op, identifier, int_number, float_number, as_, comma, op_separator, eof, openbr, closebr, begin, end, any  };

struct Token
{
    TokenType type;
    std::string content;
    size_t line;

    Token(TokenType _type, std::string _content, size_t _line): type(_type), content(_content), line(_line) {}

    bool operator==(const Token& b) const { return type == b.type && content == b.content && line == b.line; }
};

Token parseToken(std::string token, size_t line = 1);
std::vector<Token> lexString(std::string str);

std::ostream& operator<<(std::ostream& stream, const Token& t);

std::string prettyPrintTokType(TokenType type);

/*class LexerException: public std::runtime_error
{
public:
    explicit LexerException(TokenType _type, int _line);

    TokenType type() { return m_type; }
    int line() { return m_line; }
private:
    TokenType m_type;
    int m_line;
};*/

#endif //RGR_LEXER_H
