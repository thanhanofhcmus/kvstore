#include <iostream>
#include <string>

#include "parse_common.h"
#include "lexer.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

int main() {
    auto const source = "true some_thing 123 and 4 false 23 or (\ntrue or((())) true false )"s;

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