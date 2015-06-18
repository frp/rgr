//
// Created by roman on 12.06.15.
//

#include "catch.hpp"
#include "Lexer.h"

using namespace std;


TEST_CASE( "token types test", "[parseToken]" ) {
    SECTION( "operators" ) {
        REQUIRE (parseToken("<").type == TokenType::relation_op);
        REQUIRE (parseToken(">").type == TokenType::relation_op);
        REQUIRE (parseToken("<>").type == TokenType::relation_op);
        REQUIRE (parseToken("=").type == TokenType::relation_op);
        REQUIRE (parseToken(">=").type == TokenType::relation_op);
        REQUIRE (parseToken("<=").type == TokenType::relation_op);

        REQUIRE (parseToken("+").type == TokenType::add_op);
        REQUIRE (parseToken("-").type == TokenType::add_op);
        REQUIRE (parseToken("or").type == TokenType::add_op);

        REQUIRE (parseToken("*").type == TokenType::mul_op);
        REQUIRE (parseToken("/").type == TokenType::mul_op);
        REQUIRE (parseToken("and").type == TokenType::mul_op);
    }

    SECTION( "identifiers" ) {
        REQUIRE (parseToken("aaa").type == TokenType::identifier);
        REQUIRE (parseToken("a009").type == TokenType::identifier);
        REQUIRE (parseToken("aA009").type == TokenType::identifier);
        REQUIRE_THROWS (parseToken("009aa"));
    }

    SECTION( "keywords" ) {
        REQUIRE (parseToken("dim").type == TokenType::dim);
        REQUIRE (parseToken("if").type == TokenType::if_);
        REQUIRE (parseToken("else").type == TokenType::else_);
        REQUIRE (parseToken("while").type == TokenType::while_);
        REQUIRE (parseToken("for").type == TokenType::for_);
        REQUIRE (parseToken("do").type == TokenType::do_);
        REQUIRE (parseToken("to").type == TokenType::to_);
        REQUIRE (parseToken("read").type == TokenType::read_);
        REQUIRE (parseToken("write").type == TokenType::write_);
    }

    SECTION( "int numbers" ) {
        REQUIRE (parseToken("423423").type == TokenType::int_number);
        REQUIRE (parseToken("423423d").type == TokenType::int_number);
        REQUIRE (parseToken("423423D").type == TokenType::int_number);
        REQUIRE (parseToken("423423o").type == TokenType::int_number);
        REQUIRE (parseToken("423423O").type == TokenType::int_number);
        REQUIRE (parseToken("0111011b").type == TokenType::int_number);
        REQUIRE (parseToken("0111011B").type == TokenType::int_number);
        REQUIRE_THROWS (parseToken("423423b"));
        REQUIRE_THROWS (parseToken("423428O"));
    }

    SECTION( "float numbers" ) {
        REQUIRE (parseToken("1.3").type == TokenType::float_number);
        REQUIRE (parseToken(".3").type == TokenType::float_number);
        REQUIRE (parseToken("1e3").type == TokenType::float_number);
        REQUIRE (parseToken("1.3e3").type == TokenType::float_number);
        REQUIRE (parseToken("1.3e+3").type == TokenType::float_number);
        REQUIRE (parseToken("1.3e-3").type == TokenType::float_number);
        REQUIRE (parseToken(".3e-3").type == TokenType::float_number);
        REQUIRE_THROWS (parseToken(".3.3"));
    }

    SECTION( "braces" ) {
        REQUIRE (parseToken("(").type == TokenType::openbr);
        REQUIRE (parseToken(")").type == TokenType::closebr);
    }
}

TEST_CASE ( "lexing whole string test", "[lexString]" ) {
    vector<Token> expected_tok_seq {
            { TokenType::dim, "dim", 1 },
            { TokenType::identifier, "a", 1 },
            { TokenType::type, "integer", 1 },
            { TokenType::op_separator, "\n", 1 },
            { TokenType::identifier, "a", 2 },
            { TokenType::as_, "as", 2 },
            { TokenType::int_number, "1", 2 },
            { TokenType::add_op, "+", 2 },
            { TokenType::int_number, "2", 2 },
            { TokenType::add_op, "+", 2 },
            { TokenType::int_number, "3", 2 }
    };

    SECTION ("simple tests")
    {
        REQUIRE ( lexString("dim a integer\n"
                                    "a as 1 + 2 + 3") == expected_tok_seq);

        REQUIRE ( lexString("true") == (vector<Token> { { TokenType::bool_const, "true", 1 } }) );

        REQUIRE ( lexString("begin") == (vector<Token> { { TokenType::begin, "begin", 1 } } ));
        REQUIRE ( lexString("end") == (vector<Token> { { TokenType::end, "end", 1 } } ));
    }

    SECTION ("no spaces between operators")
    {
        vector<Token> expected {
                { TokenType::int_number, "1", 1 },
                { TokenType::add_op, "+", 1 },
                { TokenType::int_number, "2", 1 },
                { TokenType::add_op, "+", 1 },
                { TokenType::int_number, "3", 1 }
        };
        REQUIRE ( lexString("1+2+3") == expected);
    }

    SECTION ("floating point numbers")
    {
        string t1 = "1.23", t2 = "1.23e10", t3 = "1.23e+10", t4 = "1.23e-10", t5 = ".23e10";
        REQUIRE ((lexString(t1) == vector<Token> { { TokenType::float_number, t1, 1 } }));
        REQUIRE ((lexString(t2) == vector<Token> { { TokenType::float_number, t2, 1 } }));
        REQUIRE ((lexString(t3) == vector<Token> { { TokenType::float_number, t3, 1 } }));
        REQUIRE ((lexString(t4) == vector<Token> { { TokenType::float_number, t4, 1 } }));
        REQUIRE ((lexString(t5) == vector<Token> { { TokenType::float_number, t5, 1 } }));
    }

    SECTION ("dim with three variables")
    {
        string str = "dim a,b,c integer";
        vector<Token> expected = {
                { TokenType::dim, "dim", 1 },
                { TokenType::identifier, "a", 1 },
                { TokenType::comma, ",", 1 },
                { TokenType::identifier, "b", 1 },
                { TokenType::comma, ",", 1 },
                { TokenType::identifier, "c", 1 },
                { TokenType::type, "integer", 1 }
        };

        REQUIRE (lexString(str) == expected);
    }

    SECTION ("two dims separated with colon")
    {
        string str = "dim a integer : dim b,c integer";
        vector<Token> expected = {
                { TokenType::dim, "dim", 1 },
                { TokenType::identifier, "a", 1 },
                { TokenType::type, "integer", 1 },
                { TokenType::op_separator, ":", 1},
                { TokenType::dim, "dim", 1 },
                { TokenType::identifier, "b", 1 },
                { TokenType::comma, ",", 1 },
                { TokenType::identifier, "c", 1 },
                { TokenType::type, "integer", 1 }
        };

        REQUIRE (lexString(str) == expected);
    }

    SECTION ("braces") {
        vector<Token> expected {
                { TokenType::int_number, "3", 1 },
                { TokenType::mul_op, "*", 1 },
                { TokenType::openbr, "(", 1 },
                { TokenType::int_number, "2", 1 },
                { TokenType::add_op, "+", 1 },
                { TokenType::int_number, "3", 1 },
                { TokenType::closebr, ")", 1 },
        };
        REQUIRE (lexString("3*(2+3)") == expected);
    }

    SECTION ("comments") {
        vector<Token> expected {
                { TokenType::int_number, "3", 1 }
        };

        vector<Token> expected2 {
                { TokenType::int_number, "3", 2 }
        };

        REQUIRE (lexString("{gfdgjkfdh ghfd jghfd jghfdj ghdfj ghdfj ghdfj gjdfkjg df}3") == expected);
        REQUIRE (lexString("{gfdgjkfdh\nghfd jghfd jghfdj ghdfj ghdfj ghdfj gjdfkjg df}3") == expected2);

        REQUIRE_THROWS (lexString("{gfdgjkfdh ghfd jghfd jghfdj ghdfj ghdfj ghdfj gjdfkjg df"));

        //REQUIRE(lexString("{if a > b then c as b - a else c as a - b}\n3") == expected2);

    }
}