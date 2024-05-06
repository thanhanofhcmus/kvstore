#include "parse_common.h"

#include <iomanip>

constexpr auto token_type_repr(TokenType t) -> const char * {
    switch (t) {
    case TokenType::And:
        return "and";
    case TokenType::Or:
        return "or";
    case TokenType::True:
        return "true";
    case TokenType::False:
        return "false";
    case TokenType::LParen:
        return "lparen";
    case TokenType::RParen:
        return "rparen";
    case TokenType::Number:
        return "number";
    case TokenType::Identifier:
        return "identifier";
    default:
        return "unknown";
    }
}

std::ostream &operator<<(std::ostream &o, SourcePosition const &p) {
    constexpr int w = 2;
    o << '[' << std::setw(w) << p.line << ':' << std::setw(w) << p.column << ']';
    return o;
}

std::ostream &operator<<(std::ostream &o, SourceRange const &p) {
    o << '[' << p.start << '-' << p.end << ']';
    return o;
}

std::ostream &operator<<(std::ostream &o, Token const &tk) {
    o << '{' << std::setw(10) << token_type_repr(tk.type)
        << " | " << tk.source_range << " | " << std::quoted(tk.literal) << '}';
    return o;
}

