#ifndef QUERY_LANG_PARSE_COMMON_H
#define QUERY_LANG_PARSE_COMMON_H

#include <ostream>

enum class TokenType {
    And,
    Or,
    True,
    False,

    LParen,
    RParen,

    Number,
    Identifier,
};

constexpr auto token_type_repr(TokenType t) -> const char *;

struct SourcePosition {
    ssize_t line;
    ssize_t column;
};

std::ostream &operator<<(std::ostream &o, SourcePosition const &p);

struct SourceRange {
    SourcePosition start;
    SourcePosition end;
};

std::ostream &operator<<(std::ostream &o, SourceRange const &p);

struct Token {
    TokenType type;
    std::string_view literal;
    SourceRange source_range;
};

std::ostream &operator<<(std::ostream &o, Token const &tk);

enum class ParseErrorType {
    Eof,
    UnknownToken,
};

struct ParseError {
    ParseErrorType type;
    SourceRange source_range;
    std::string cause;
};

#endif //QUERY_LANG_PARSE_COMMON_H
