#include "expected.h"

#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <iomanip>

using namespace std::string_literals;
using namespace std::string_view_literals;

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

constexpr auto token_type_repr(TokenType t) {
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

struct SourcePosition {
    ssize_t line;
    ssize_t column;
};

std::ostream &operator<<(std::ostream &o, SourcePosition const &p) {
    constexpr int w = 2;
    o << '[' << std::setw(w) << p.line << ':' << std::setw(w) << p.column << ']';
    return o;
}

struct SourceRange {
    SourcePosition start;
    SourcePosition end;
};

std::ostream &operator<<(std::ostream &o, SourceRange const &p) {
    o << '[' << p.start << '-' << p.end << ']';
    return o;
}

struct Token {
    TokenType type;
    std::string_view literal;
    SourceRange source_range;
};

std::ostream &operator<<(std::ostream &o, Token const &tk) {
    o << '{' << std::setw(10) << token_type_repr(tk.type)
        << " | " << tk.source_range << " | " << std::quoted(tk.literal) << '}';
    return o;
}

enum class ParseErrorType {
    Eof,
    UnknownToken,
};

struct ParseError {
    ParseErrorType type;
    SourceRange source_range;
    std::string cause;
};

class Lexer {
public:
    using ExpToken = Expected<Token, ParseError>;

    explicit Lexer(std::string const &source_)
        : source(source_),
          current_idx(0),
          current_position({.line = 1, .column = 1}) {
    }

    auto lex() -> Expected<std::vector<Token>, ParseError> {
        using result_t = Expected<std::vector<Token>, ParseError>;
        auto tokens = std::vector<Token>{};

        while (not is_at_end()) {
            auto const c = get_current();
            auto tk = Token{
                .literal = std::string_view{source}.substr(current_idx, 1),
                .source_range = SourceRange{current_position, current_position},
            };

            switch (c) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                advance();
                break;
            case '(':
                tk.type = TokenType::LParen;
                tokens.push_back(tk);
                advance();
                break;
            case ')':
                tk.type = TokenType::RParen;
                tokens.push_back(tk);
                advance();
                break;
            default: {
                auto const tk_exp = parse_other();
                if (tk_exp.has_error()) {
                    return result_t{UnexpectTag, tk_exp.error()};
                }
                tokens.push_back(tk_exp.value());
            }
            break;
            }
        }

        return result_t{ExpectTag, tokens};
    }

private:
    static auto is_num_char(char c) -> bool {
        return '0' <= c and c <= '9';
    }

    static auto is_numkw_char(char c) -> bool {
        return is_num_char(c) or is_keyword_char(c);
    }

    static auto is_keyword_char(char c) -> bool {
        return c == '_' or c == '-' or std::isalpha(c);
    }

    auto parse_other() -> ExpToken {
        auto const c = get_current();
        if (is_num_char(c)) {
            return parse_number();
        }
        if (is_keyword_char(c)) {
            return parse_keyword_or_identifier();
        }
        return new_invalid_error();
    }

    [[nodiscard]] auto new_invalid_error(
        std::optional<SourceRange> const &o_rs = std::nullopt,
        std::optional<std::string_view> const &o_raw = std::nullopt) const -> ExpToken {
        const auto source_range = o_rs.value_or(SourceRange{current_position, current_position});
        const auto [line, column] = source_range.start;
        const auto raw = o_raw.value_or(std::string_view{source}.substr(current_idx, 1));
        return ExpToken{
            ParseError{
                .type = ParseErrorType::UnknownToken,
                .source_range = source_range,
                .cause = std::format(
                    "unknown literal `{}` [{}:{}]",
                    raw, line, column),
            }
        };
    }

    auto parse_number() -> ExpToken {
        auto const start = current_position;
        auto const start_idx = current_idx;
        while (not is_at_end() and is_num_char(get_current())) {
            advance();
        }
        auto const raw = std::string_view{source}.substr(start_idx, current_idx - start_idx);
        return ExpToken{
            Token{
                .type = TokenType::Number,
                .literal = raw,
                .source_range = {start, current_position}
            }
        };
    }

    auto parse_keyword_or_identifier() -> ExpToken {
        auto const start = current_position;
        auto const start_idx = current_idx;
        while (not is_at_end() and is_numkw_char(get_current())) {
            advance();
        }
        auto const source_range = SourceRange{start, current_position};

        // check for key worlds
        auto const raw = std::string_view{source}.substr(start_idx, current_idx - start_idx);
        auto tk = std::optional<TokenType>();
        if (raw == "and"sv) {
            tk = std::make_optional(TokenType::And);
        } else if (raw == "or"sv) {
            tk = std::make_optional(TokenType::Or);
        } else if (raw == "true"sv) {
            tk = std::make_optional(TokenType::True);
        } else if (raw == "false"sv) {
            tk = std::make_optional(TokenType::False);
        }
        if (!tk.has_value()) {
            tk = std::make_optional(TokenType::Identifier);
        }

        return ExpToken{
            Token{
                .type = tk.value(),
                .literal = raw,
                .source_range = source_range,
            }
        };
    }

    [[nodiscard]] auto is_at_end() const -> bool {
        return current_idx >= source.size();
    }

    [[nodiscard]] auto get_current() const -> char {
        if (is_at_end()) {
            return 0;
        }
        return source[current_idx];
    }

    [[nodiscard]] auto get_peek() const -> char {
        if (current_idx + 1 >= source.size()) {
            return 0;
        }
        return source[current_idx + 1];
    }

    auto advance() -> char {
        if (is_at_end()) {
            return 0;
        }

        const auto old_idx = current_idx;
        current_idx += 1;
        current_position.column += 1;

        if (last_value == '\n') {
            current_position.line += 1;
            current_position.column = 1;
        }

        last_value = source[old_idx];
        return last_value;
    }

    std::string const &source;
    size_t current_idx;
    SourcePosition current_position;
    char last_value = 0;
};

int main() {
    auto source = "true some_thing 123 and 4 false 23 or (\ntrue or((())) true false )"s;

    auto lexer = Lexer(source);
    auto const tokens = lexer.lex();

    if (tokens.has_error()) {
        std::cout << tokens.error().cause;
        return 0;
    }

    for (auto const &tk : tokens.value()) {
        std::cout << tk << '\n';
    }
}