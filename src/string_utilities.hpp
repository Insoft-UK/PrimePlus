// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <regex>
#include <string>
#include <ranges>
#include <unordered_set>


bool is_all_whitespace(const std::string& s);

/**
 * @brief Cleans up whitespace in a string while preserving word separation.
 *
 * This function removes all unnecessary whitespace characters (spaces, tabs, newlines, etc.)
 * from the input string. It ensures that only a single space is inserted between consecutive
 * word characters (letters, digits, or underscores) when needed to maintain logical separation.
 *
 * Non-word characters (such as punctuation) are not separated by spaces, and leading/trailing
 * whitespace is removed.
 *
 * @param input The input string to be cleaned.
 * @return A new string with cleaned and normalized whitespace.
 *
 * @note This is useful for normalizing input for parsers, code formatters, or text display
 *       where compact and readable word separation is desired.
 */

std::string cleanWhitespace(const std::string& input);

/**
 * @brief Replaces common two-character operators with their symbolic Unicode equivalents.
 *
 * This function scans the input string and replaces specific two-character operator
 * sequences with their corresponding Unicode symbols:
 *
 * - `>=` becomes `≥`
 * - `<=` becomes `≤`
 * - `=>` becomes `▶`
 * - `<>` becomes `≠`
 *
 * All other characters are copied as-is.
 *
 * @param input The input string potentially containing ASCII operator sequences.
 * @return A new string with supported operators replaced by Unicode symbols.
 *
 * @note Useful for rendering more readable mathematical or logical expressions in UI output,
 *       documents, or educational tools.
 */
std::string replaceOperators(const std::string& input);

/**
 * @brief Expands single `=` characters into `==` for comparison, preserving special cases.
 *
 * This function processes an input string and replaces single `=` characters with `==`,
 * which is commonly used for comparison in many programming languages. It handles the
 * following exceptions:
 *
 * - `:=` (colon-equals) is left unchanged, as it may represent assignment in some languages.
 * - `==` is preserved and not duplicated.
 *
 * This is useful for transforming assignment-style expressions into comparison-style
 * expressions for parsing or analysis.
 *
 * @param input The input string containing `=` characters.
 * @return A new string with applicable `=` characters expanded to `==`.
 *
 * @note This function assumes a language syntax where `==` is the comparison operator and
 *       `:=` has a distinct meaning. It does not perform full syntax validation.
 */
std::string expandAssignmentEquals(const std::string& input);
/**
 * @brief Converts single assignment `=` operators to `:=`, preserving comparison and existing assignment syntax.
 *
 * This function scans the input string and replaces standalone `=` characters with `:=`,
 * often used in languages like Pascal or certain calculators to represent assignment.
 * It carefully handles the following cases:
 *
 * - `==` (equality comparison) is preserved and not modified.
 * - `:=` (already valid assignment syntax) is left unchanged.
 * - Any other standalone `=` is converted to `:=`.
 *
 * @param input The input string potentially containing assignment or comparison operators.
 * @return A new string with appropriate `=` characters converted to `:=`.
 *
 * @note Useful for translating code or expressions from languages that use `=` for assignment
 *       into those that use `:=`, while avoiding accidental changes to comparison or already-valid syntax.
 */
std::string convertAssignToColonEqual(const std::string& input);

/**
 * @brief Normalizes spacing around operators in a string.
 *
 * This function ensures that operators from a predefined list are properly spaced
 * within the input string. It scans the string, identifies any known operators,
 * and inserts spaces before and after them if needed to ensure consistent formatting.
 *
 * After operator normalization, the function also removes any extra or redundant
 * whitespace, ensuring clean and readable output.
 *
 * @param input The input string that may contain operators with inconsistent spacing.
 * @param operators The list of `operators`.
 * @return A new string with operators consistently spaced and extraneous whitespace removed.
 */

std::string normalizeOperators(const std::string& input, const std::vector<std::string> operators = { ":=", "==", "▶", "≥", "≤", "≠" });


bool isUnaryTarget(char c);

std::string fixUnaryMinus(const std::string& s);

/**
 * @brief Splits a string into a vector of substrings using commas as delimiters.
 *
 * This function tokenizes the input string based on commas (`,`) and returns
 * a vector containing each separated segment as an individual string.
 * Empty tokens between consecutive commas are preserved.
 *
 * @param input The input string to be split.
 * @return A vector of substrings split at each comma.
 *
 * @note Leading or trailing whitespace within tokens is not trimmed.
 *       Use additional processing if whitespace cleanup is needed.
 *
 * Example usage:
 * splitCommas("a,b,c") returns {"a", "b", "c"}
 * splitCommas("one,,three") returns {"one", "", "three"}
 */
std::vector<std::string> splitCommas(const std::string& input);

/**
 * @brief Splits a string into substrings at commas, supporting escaped commas.
 *
 * This function tokenizes the input string by commas (`,`) while allowing
 * commas to be escaped using a backslash (`\\`). Escaped commas are treated
 * as literal characters and are included in the resulting token rather than
 * splitting the string at that point.
 *
 * - A sequence like `\\,` becomes a literal comma in the result.
 * - Other escape sequences (e.g. `\\n`) are preserved with the backslash.
 *
 * @param input The input string to be split.
 * @return A vector of strings split on unescaped commas.
 *
 * @note This function does not handle more advanced escaping rules like double escaping (`\\\\,`),
 *       and does not trim whitespace around tokens.
 *
 * Example usage:
 * splitEscapedCommas("one,two,three") returns {"one", "two", "three"}
 * splitEscapedCommas("one\\,with\\,commas,two") returns {"one,with,commas", "two"}
 * splitEscapedCommas("escaped\\\\slash,test") returns {"escaped\\slash", "test"}
 */
std::vector<std::string> splitEscapedCommas(const std::string& input);

/**
 * @brief Processes escape sequences in a string and replaces them with corresponding characters.
 *
 * This function scans the input string for backslash-prefixed escape sequences and replaces
 * them with their corresponding characters or strings. Supported escape sequences include:
 *
 * - `\\n` → newline character (`\n`)
 * - `\\s` → space (`' '`)
 * - `\\t` → tab-like spacing (4 spaces)
 * - `\\i` → indentation (number of spaces defined by `INDENT_WIDTH`)
 *
 * Unrecognized escape sequences are preserved as-is (i.e., the backslash and the following character).
 *
 * @param input The input string potentially containing escape sequences.
 * @return A new string with escape sequences expanded into their corresponding characters.
 *
 */
std::string processEscapes(const std::string& input, int indentWidth = 4);


/**
 * @brief Converts all characters in a string to lowercase.
 *
 * This function takes an input string and returns a new string
 * with every character converted to its lowercase equivalent,
 * using the standard C locale rules.
 *
 * @param s The input string to convert.
 * @return A new string with all characters in lowercase.
 *
 * @note The conversion uses `std::tolower` with `unsigned char` casting to
 *       avoid undefined behavior for negative `char` values.
 *
 * Example:
 * toLower("Hello World!") returns "hello world!"
 */
std::string toLower(const std::string& s);

/**
 * @brief Converts all characters in a string to uppercase.
 *
 * This function takes an input string and returns a new string
 * with every character converted to its uppercase equivalent,
 * using the standard C locale rules.
 *
 * @param s The input string to convert.
 * @return A new string with all characters in uppercase.
 *
 * @note The conversion uses `std::toupper` with `unsigned char` casting to
 *       avoid undefined behavior for negative `char` values.
 *
 * Example:
 * toUpper("Hello World!") returns "HELLO WORLD!"
 */
std::string toUpper(const std::string& s);

/**
 * @brief Replaces specified words in a string with a given replacement string.
 *
 * This function scans the input string and replaces all occurrences of words
 * found in the provided list (case-insensitive) with the specified replacement string.
 * Words are defined as sequences of alphabetic characters and underscores (`_`).
 * Non-word characters are preserved as-is.
 *
 * @param input The input string to process.
 * @param words A vector of words to be replaced (case-insensitive).
 * @param replacement The string to replace each matched word with.
 * @return A new string with the specified words replaced.
 *
 * @note Matching is case-insensitive. The function treats underscores as part of words.
 *
 * Example"
 *   replaceWords("Hello world_123", {"world_123"}, "Earth") returns "Hello Earth"
 */
std::string replaceWords(const std::string& input, const std::vector<std::string>& words, const std::string& replacement);

/**
 * @brief Capitalizes specified words in a string by converting them to uppercase.
 *
 * This function scans the input string and converts to uppercase all occurrences
 * of words found in the provided set (case-insensitive). Words are defined as
 * sequences of alphabetic characters and underscores (`_`). Non-word characters
 * are preserved as-is.
 *
 * @param input The input string to process.
 * @param words An unordered set of words to capitalize (case-insensitive).
 * @return A new string with the specified words converted to uppercase.
 *
 * @note Matching is case-insensitive. The function treats underscores as part of words.
 *
 * Example usage:
 * capitalizeWords("hello world_test", {"world_test"}) returns "hello WORLD_TEST"
 */
std::string capitalizeWords(const std::string& input, const std::unordered_set<std::string>& words);

/**
 * @brief Inserts a single space after specified characters in a string.
 *
 * This function scans the input string and ensures that after every character
 * found in the provided set, there is exactly one space inserted. If spaces
 * already exist immediately after such characters, they are collapsed to a
 * single space. If there is no space, one is added.
 *
 * @param input The input string to process.
 * @param chars An unordered set of characters after which to insert a space.
 * @return A new string with spaces inserted as specified.
 *
 * @note Existing whitespace sequences following the specified characters are
 *       replaced with a single space.
 *
 * Example usage:
 * insertSpaceAfterChars("a,b;c", {',', ';'}) returns "a, b; c"
 */
std::string insertSpaceAfterChars(const std::string& input, const std::unordered_set<char>& chars);

/**
 * @brief Inserts a space before a word that immediately follows a closing parenthesis without a space.
 *
 * This function scans the input string and inserts a space before any alphabetic character
 * that directly follows a closing parenthesis `')'` when there is no space in between.
 * This helps separate words that start immediately after a closing parenthesis.
 *
 * @param input The input string to process.
 * @return A new string with spaces inserted before words following a closing parenthesis.
 *
 * Example usage:
 * insertSpaceAfterClosingParen("foo(bar)baz") returns "foo(bar) baz"
 * insertSpaceAfterClosingParen("foo(bar) baz") returns "foo(bar) baz"
 */
std::string insertSpaceAfterClosingParen(const std::string& input);

/**
 * @brief Extracts a single-line PPL comment from a given string.
 *
 * This function searches for the occurrence of a PPL  comment
 * marker (`//`) in the input string and returns the comment, excluding the `//`.
 * If no comment is found, an empty string is returned.
 *
 * @param str The input string potentially containing a PPL comment.
 * @return The extracted comment starting with `//`, or an empty string if none is found.
 */
std::string extractComment(const std::string &str);

/**
 * @brief Removes a PPL single-line comment from a line of source code.
 *
 * This function scans a string representing a line of PPL (HP Prime Programming Language)
 * source code and removes any single-line comment starting with `//`.
 * If no comment is found, the original string is returned unchanged.
 *
 * @param str A line of PPL source code.
 * @return The input line without the `//` comment, if present.
 *
 */
std::string removeComment(const std::string& str);

/**
 * @brief Removes a triple-slash (`///`) comment from a line of source code.
 *
 * This function searches for the first occurrence of a `///` comment in the input
 * string and removes the comment portion. If a newline follows the comment, only the
 * comment part up to the newline is removed; otherwise, the remainder of the string
 * is removed. This is useful for stripping documentation-style comments (e.g., in PPL or C++).
 *
 * @param str A string potentially containing a `///` comment.
 * @return A copy of the input string with the `///` comment removed.
 *
 * @note This function does not remove standard `//` comments or handle multiline block comments.
 */
std::string removeTripleSlashComment(const std::string& str);

/**
 * @brief Appends a comment to a line of code, ensuring proper spacing.
 *
 * Adds a PPL single-line comment (`// comment`) to the end of the given string,
 * making sure the comment starts with exactly one space after the last non-whitespace
 * character in the code.
 *
 * @param line The base line of code (without comment).
 * @param comment The comment text (without the leading slashes).
 * @return A new string with the comment appended properly.
 */
std::string applyComment(const std::string& line, const std::string& comment);

/**
 * @brief Extracts and preserves all double-quoted substrings from the input string.
 *
 * Handles escaped quotes (e.g., \" inside quoted text) and does not use regex.
 *
 * @param str The input string.
 * @return std::list<std::string> A list of quoted substrings, including the quote characters.
 */
std::list<std::string> preserveStrings(const std::string& str);

/**
 * @brief Replaces all double-quoted substrings in the input string with "".
 *
 * Handles escaped quotes (e.g., \" inside strings) and does not use regex.
 *
 * @param str The input string to process.
 * @return std::string A new string with quoted substrings replaced by "".
 */
std::string blankOutStrings(const std::string& str);

/**
 * @brief Restores quoted strings into a string that had them blanked out.
 *
 * @param str The string with blanked-out quoted substrings (e.g., `""`).
 * @param strings A list of original quoted substrings, in the order they appeared.
 * @return std::string A new string with the original quoted substrings restored.
 */
std::string restoreStrings(const std::string& str, std::list<std::string>& strings);


/**
 * @brief Extracts and preserves all backtick substrings from the input string.
 *
 * Handles escaped quotes (e.g., \` inside backtick text) and does not use regex.
 *
 * @param str The input string.
 * @return std::list<std::string> A list of backtick substrings, including the backtick characters.
 */
std::list<std::string> preserveBacktick(const std::string& str);

/**
 * @brief Replaces all backtick substrings in the input string with ``.
 *
 * Handles escaped quotes (e.g., \` inside strings) and does not use regex.
 *
 * @param str The input string to process.
 * @return std::string A new string with backtick substrings replaced by ``.
 */
std::string blankOutBacktick(const std::string& str);

/**
 * @brief Restores backtick strings into a string that had them blanked out.
 *
 * @param str The string with blanked-out backtick substrings (e.g., ``).
 * @param strings A list of original backtick substrings, in the order they appeared.
 * @return std::string A new string with the original backtick substrings restored.
 */
std::string restoreBacktick(const std::string& str, std::list<std::string>& strings);

