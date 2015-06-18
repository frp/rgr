//
// Created by roman on 12.06.15.
//

#include "Lexer.h"
#include <regex>
#include <sstream>
#include <map>
using namespace std;

//enum class TokenType { relation_op, add_op, mul_op, un_op, bool_const, identifier, int_number, float_number, dim, type, if_, then_, else_, for_, to_, do_, while_, read_, write_ };

namespace
{
    map<TokenType, regex> regexps = {
            { TokenType::relation_op, regex("<>|<=|>=|=|<|>") },
            { TokenType::add_op, regex("or|\\+|-") },
            { TokenType::mul_op, regex("and|\\*|/") },
            { TokenType::un_op, regex("not") },
            { TokenType::bool_const, regex("true|false") },
            { TokenType::identifier, regex("[A-Za-z][A-Za-z0-9]*") },
            { TokenType::int_number, regex("[0-1]+[bB]|[0-7]+[oO]|[0-9A-Fa-f]+[hH]|[0-9]+[dD]?") },
            { TokenType::float_number, regex("[0-9]+[eE][\\+\\-]?[0-9]+|[0-9]*\\.[0-9]+([eE][\\+\\-]?[0-9]+)?")},
            { TokenType::dim, regex("dim")},
            { TokenType::type, regex("integer|float|bool")},
            { TokenType::if_, regex("if")},
            { TokenType::then_, regex("then")},
            { TokenType::else_, regex("else")},
            { TokenType::for_, regex("for")},
            { TokenType::to_, regex("to")},
            { TokenType::do_, regex("do")},
            { TokenType::while_, regex("while")},
            { TokenType::read_, regex("read")},
            { TokenType::write_, regex("write")},
            { TokenType::as_, regex("as")},
            { TokenType::comma, regex(",") },
            { TokenType::op_separator, regex(":|\n") },
            { TokenType::openbr, regex("\\(") },
            { TokenType::closebr, regex("\\)") },
            { TokenType::begin, regex("begin") },
            { TokenType::end, regex("end") },
    };

    map<TokenType, string> dumpClasses = {
            { TokenType::relation_op, "relation operator" },
            { TokenType::add_op, "addition operator" },
            { TokenType::mul_op, "multiplication operator" },
            { TokenType::un_op, "not" },
            { TokenType::bool_const, "bool const" },
            { TokenType::identifier, "identifier" },
            { TokenType::int_number, "integral number" },
            { TokenType::float_number, "float number" },
            { TokenType::dim, "dim" },
            { TokenType::type, "type" },
            { TokenType::if_, "if"},
            { TokenType::then_, "then"},
            { TokenType::else_, "else"},
            { TokenType::for_, "for"},
            { TokenType::to_, "to"},
            { TokenType::do_, "do"},
            { TokenType::while_, "while"},
            { TokenType::read_, "read"},
            { TokenType::write_, "write"},
            { TokenType::as_, "as"},
            { TokenType::comma, "comma"},
            { TokenType::op_separator, "operation separator"},
            { TokenType::openbr, "(" },
            { TokenType::closebr, ")" },
            { TokenType::begin, "begin" },
            { TokenType::end, "end" },
    };

    vector<TokenType> priorityList = {
            TokenType::begin,
            TokenType::end,
            TokenType::openbr,
            TokenType::closebr,
            TokenType::op_separator,
            TokenType::bool_const,
            TokenType::if_,
            TokenType::then_,
            TokenType::else_,
            TokenType::for_,
            TokenType::to_,
            TokenType::do_,
            TokenType::dim,
            TokenType::while_,
            TokenType::read_,
            TokenType::write_,
            TokenType::as_,
            TokenType::un_op,
            TokenType::type,
            TokenType::comma,
            TokenType::relation_op,
            TokenType::add_op,
            TokenType::mul_op,
            TokenType::int_number,
            TokenType::float_number,
            TokenType::identifier
    };

    string extractToken(size_t& ptr, const string& input, size_t& line)
    {
        string result;
        while (ptr < input.size() && isspace(input[ptr]))
        {
            ptr++;

            if (input[ptr-1] == '\n')
                return "\n";
        }

        if (ptr == input.size())
            return "";

        if (input[ptr] == '{')
        {
            while (ptr < input.size() && input[ptr] != '}')
            {
                if (input[ptr] == '\n')
                    line++;

                ptr++;
            }

            if (ptr == input.size())
                throw runtime_error("Comment is not finished");

            ptr++;

            if (ptr == input.size())
                return "";
        }

        if (input[ptr] == '(')
        {
            ptr++;
            return "(";
        }

        if (input[ptr] == ')')
        {
            ptr++;
            return ")";
        }

        bool startWithAlnum = isalnum(input[ptr]);
        bool startWithDigit = isdigit(input[ptr]) || input[ptr] == '.';

        while (ptr < input.size()
               && input[ptr] != '(' && input[ptr] != ')' && (
                startWithDigit && (isalnum(input[ptr]) || input[ptr] == '.' || (ptr > 0 && tolower(input[ptr-1]) == 'e' && (input[ptr] == '+' || input[ptr] == '-')))
                || startWithAlnum && isalnum(input[ptr])
                || !startWithAlnum && !isspace(input[ptr]) && !isalnum(input[ptr])))
        {
            result += input[ptr];
            ptr++;
        }

        return result;
    }
};

Token parseToken(string token, size_t line)
{
    for (auto type: priorityList)
    {
        if (regex_match(token, regexps[type]))
            return Token(type, token, line);
    }
    throw std::runtime_error(token + " is not a valid token");
}

std::vector<Token> lexString(std::string str)
{
    size_t ptr = 0, line = 1;
    string nextToken;
    vector<Token> result;
    while ((nextToken = extractToken(ptr, str, line)) != "")
    {
        result.push_back(parseToken(nextToken, line));

        if (nextToken == "\n")
            line++;
    }
    return result;
}

std::ostream& operator<<(std::ostream& stream, const Token& t)
{
    return stream << " { " << dumpClasses[t.type] << ", " << t.content << ", " << t.line << " } ";
}

std::string prettyPrintTokType(TokenType type)
{
    return dumpClasses[type];
}
