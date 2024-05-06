#include <vector>
#include <optional>
#include <format>
#include <string_view>

#include "lexer.h"

#include "parse_common.h"
#include "expected.h"

using namespace std::string_view_literals;

static auto is_num_char(char c) -> bool {
    return '0' <= c and c <= '9';
}

static auto is_keyword_char(char c) -> bool {
    return c == '_' or c == '-' or std::isalpha(c);
}

static auto is_numkw_char(char c) -> bool {
    return is_num_char(c) or is_keyword_char(c);
}

auto Lexer::lex() -> Expected<std::vector<Token>, ParseError> {
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

auto Lexer::parse_other() -> ExpToken {
    auto const c = get_current();
    if (is_num_char(c)) {
        return parse_number();
    }
    if (is_keyword_char(c)) {
        return parse_keyword_or_identifier();
    }
    return new_invalid_error();
}

auto Lexer::new_invalid_error(
    std::optional<SourceRange> const &o_rs,
    std::optional<std::string_view> const &o_raw) const -> ExpToken {
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

auto Lexer::parse_number() -> ExpToken {
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

auto Lexer::parse_keyword_or_identifier() -> ExpToken {
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

auto Lexer::is_at_end() const -> bool {
    return current_idx >= source.size();
}

auto Lexer::get_current() const -> char {
    if (is_at_end()) {
        return 0;
    }
    return source[current_idx];
}

auto Lexer::get_peek() const -> char {
    if (current_idx + 1 >= source.size()) {
        return 0;
    }
    return source[current_idx + 1];
}

auto Lexer::advance() -> char {
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
