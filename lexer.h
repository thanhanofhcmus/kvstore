#ifndef QUERY_LANG_LEXER_H
#define QUERY_LANG_LEXER_H

#include <vector>
#include <optional>
#include <string_view>


#include "parse_common.h"
#include "expected.h"

class Lexer {
public:
    using ExpToken = Expected<Token, ParseError>;

    explicit Lexer(std::string const &source_)
        : source(source_),
          current_idx(0),
          current_position({.line = 1, .column = 1}) {
    }

    auto lex() -> Expected<std::vector<Token>, ParseError>;

private:
    auto parse_other() -> ExpToken;

    [[nodiscard]] auto new_invalid_error(
        std::optional<SourceRange> const &o_rs = std::nullopt,
        std::optional<std::string_view> const &o_raw = std::nullopt) const -> ExpToken;

    auto parse_number() -> ExpToken;

    auto parse_keyword_or_identifier() -> ExpToken;

    [[nodiscard]] auto is_at_end() const -> bool;

    [[nodiscard]] auto get_current() const -> char;

    [[nodiscard]] auto get_peek() const -> char;

    auto advance() -> char;

    std::string const &source;
    size_t current_idx;
    SourcePosition current_position;
    char last_value = 0;
};


#endif //QUERY_LANG_LEXER_H
