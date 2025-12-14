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

#include "reformat.hpp"



#define INDENT_WIDTH 2

// MARK: - ● Python block logic

static std::list<std::string> extractPythonBlocks(const std::string& str) {
    std::list<std::string> blocks;
    const std::string startTag = "#PYTHON";
    const std::string endTag = "#END";
    
    size_t pos = 0;

    while (true) {
        size_t start = str.find(startTag, pos);
        if (start == std::string::npos)
            break;

        start += startTag.length();  // move past the #PYTHON tag

        size_t end = str.find(endTag, start);
        if (end == std::string::npos)
            break;  // no matching #END, so stop

        blocks.push_back(str.substr(start, end - start));
        pos = end + endTag.length();  // move past this #END
    }

    return blocks;
}


static std::string blankOutPythonBlocks(const std::string& str) {
    std::string result;
    const std::string startTag = "#PYTHON";
    const std::string endTag = "#END";
    
    size_t pos = 0;

    while (pos < str.length()) {
        size_t start = str.find(startTag, pos);

        if (start == std::string::npos) {
            result.append(str, pos, str.length() - pos);
            break;
        }

        // Append everything before #PYTHON
        result.append(str, pos, start - pos);

        size_t end = str.find(endTag, start + startTag.length());
        if (end == std::string::npos) {
            // No matching #END — treat rest as normal text
            result.append(str, start, str.length() - start);
            break;
        }

        // Keep the #PYTHON and #END markers, but blank out in between
        result += startTag;
        result.append(end - (start + startTag.length()), ' ');
        result += endTag;

        pos = end + endTag.length();
    }

    return result;
}

static std::string restorePythonBlocks(const std::string& str, std::list<std::string>& blocks) {
    if (blocks.empty()) return str;

    const std::string startTag = "#PYTHON";
    const std::string endTag = "#END";

    std::string result;
    size_t pos = 0;

    while (pos < str.size()) {
        size_t start = str.find(startTag, pos);
        if (start == std::string::npos) {
            result.append(str, pos, str.size() - pos);  // append rest
            break;
        }

        // Append text before #PYTHON
        result.append(str, pos, start - pos);

        size_t end = str.find(endTag, start + startTag.length());
        if (end == std::string::npos || blocks.empty()) {
            // No matching #END or no block left — append rest
            result.append(str, start, str.size() - start);
            break;
        }

        // Append #PYTHON
        result.append(str, start, startTag.length());

        // Append original block content
        result.append(blocks.front());
        blocks.pop_front();

        // Append #END
        result.append(str, end, endTag.length());

        pos = end + endTag.length();
    }

    return result;
}

static std::string separatePythonMarkers(const std::string& input) {
    std::istringstream iss(input);
    std::ostringstream oss;
    std::string line;

    const std::string markers[] = {"#PYTHON", "#END"};

    while (std::getline(iss, line)) {
        size_t pos = 0;

        while (pos < line.size()) {
            bool foundMarker = false;
            for (const std::string& marker : markers) {
                size_t markerPos = line.find(marker, pos);
                if (markerPos != std::string::npos) {
                    // Add any content before the marker (if any) as a separate line
                    if (markerPos > pos) {
                        oss << line.substr(pos, markerPos - pos) << '\n';
                    }
                    // Add the marker as its own line
                    oss << marker << '\n';
                    pos = markerPos + marker.length();
                    foundMarker = true;
                    break;
                }
            }

            if (!foundMarker) {
                // No more markers on this line, output the rest
                oss << line.substr(pos) << '\n';
                break;
            }
        }
    }

    return oss.str();
}

static bool isUnaryTarget(char c) {
    return std::isdigit((unsigned char)c) ||
           std::isalpha((unsigned char)c) ||
           c == '.';
}

static std::string fixUnaryMinus(const std::string& s) {
    std::string out;
    out.reserve(s.size());

    size_t i = 0;
    while (i < s.size()) {

        if ((s[i] == '-' || s[i] == '+' || s[i] == '*' || s[i] == '/') && i + 1 < s.size() && s[i + 1] == '-') {
            size_t j = i + 2;

            // Skip spaces after the second '-'
            while (j < s.size() && std::isspace((unsigned char)s[j]))
                j++;

            if (j < s.size() && isUnaryTarget(s[j])) {
                out.push_back(s[i]);
                out.push_back(' ');
                out.push_back('-');
                // Continue from character after second '-'
                i = i + 2;
                continue;
            }
        }

        if (s[i] == '-') {
            size_t j = i + 1;

            // Skip spaces after '-'
            while (j < s.size() && std::isspace((unsigned char)s[j]))
                j++;

            // If minus followed by unary target => remove spaces
            if (j < s.size() && isUnaryTarget(s[j])) {
                out.push_back('-');
                i = j;
                continue;
            }
        }

        // Normal character
        out.push_back(s[i]);
        i++;
    }
    return out;
}

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
static std::string toUpper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

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
static std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

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
static std::string capitalizeWords(const std::string& input, const std::unordered_set<std::string>& words) {
    // Create lowercase word set
    std::unordered_set<std::string> wordSet;
    for (const auto& w : words) {
        wordSet.insert(toLower(w));
    }
    
    std::string result;
    size_t i = 0;
    
    while (i < input.size()) {
        if (!isalpha(static_cast<unsigned char>(input[i])) && input[i] != '_') {
            result += input[i];
            ++i;
            continue;
        }
        size_t start = i;
        
        while (i < input.size() && (isalpha(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
            ++i;
        }
        
        std::string word = input.substr(start, i - start);
        std::string lowercase = toLower(word);
        
        if (wordSet.count(lowercase)) {
            result += toUpper(lowercase);
            continue;
        }
        
        result += word;
    }
    
    return result;
}

static std::string cleanWhitespace(const std::string& input) {
    std::string output;
    bool lastWasWordChar = false;
    bool pendingSpace = false;

    auto isWordChar = [](char c) {
        /*
         by Jozef Dekoninck || c == '('
         Fixes an issue when parentheses after UNTIL, compression removes
         the space after UNTIL what gives an error in compression.
         */
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '(' || c == ')';
    };

    for (char ch : input) {
        if (ch == '\n') {
            if (pendingSpace) {
                pendingSpace = false; // discard pending space before newline
            }
            output += '\n';
            lastWasWordChar = false;
            continue;
        }
        
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (lastWasWordChar) {
                pendingSpace = true;
            }
            continue;
        }
        
        if (pendingSpace && lastWasWordChar && isWordChar(ch)) {
            output += ' ';
        }
        output += ch;
        lastWasWordChar = isWordChar(ch);
        pendingSpace = false;
    }

    return output;
}

static std::string replaceOperators(const std::string& input) {
    std::string output;
    output.reserve(input.size());  // Reserve space to reduce reallocations

    for (std::size_t i = 0; i < input.size(); ++i) {
        if (i + 1 < input.size()) {
            // Lookahead for 2-character operators
            if (input[i] == '>' && input[i + 1] == '=') {
                output += "≥";
                ++i;
                continue;
            }
            if (input[i] == '<' && input[i + 1] == '=') {
                output += "≤";
                ++i;
                continue;
            }
            if (input[i] == '<' && input[i + 1] == '>') {
                output += "≠";
                ++i;
                continue;
            }
        }

        // Default: copy character
        output += input[i];
    }

    return output;
}

/**
 * @brief Extracts and preserves all double-quoted substrings from the input string.
 *
 * Handles escaped quotes (e.g., \" inside quoted text) and does not use regex.
 *
 * @param str The input string.
 * @return std::list<std::string> A list of quoted substrings, including the quote characters.
 */
static std::list<std::string> preserveStrings(const std::string& str) {
    std::list<std::string> strings;
    bool inQuotes = false;
    std::string current;
    
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];

        if (!inQuotes) {
            if (c == '"') {
                inQuotes = true;
                current.clear();
                current += c;  // start quote
            }
        } else {
            current += c;

            if (c == '"' && (i == 0 || str[i - 1] != '\\')) {
                // End of quoted string (unescaped quote)
                inQuotes = false;
                strings.push_back(current);
            }
        }
    }

    return strings;
}

/**
 * @brief Restores quoted strings into a string that had them blanked out.
 *
 * @param str The string with blanked-out quoted substrings (e.g., `""`).
 * @param strings A list of original quoted substrings, in the order they appeared.
 * @return std::string A new string with the original quoted substrings restored.
 */
static std::string restoreStrings(const std::string& str, std::list<std::string>& strings) {
    static const std::regex re(R"("[^"]*")");

    if (strings.empty()) return str;

    std::string result;
    std::size_t lastPos = 0;

    auto stringIt = strings.begin();
    for (auto it = std::sregex_iterator(str.begin(), str.end(), re);
         it != std::sregex_iterator() && stringIt != strings.end(); ++it, ++stringIt)
    {
        const std::smatch& match = *it;

        // Append the part before the match
        result.append(str, lastPos, match.position() - lastPos);

        // Append the preserved quoted string
        result.append(*stringIt);

        // Update the last position
        lastPos = match.position() + match.length();
    }

    // Append the remaining part of the string after the last match
    result.append(str, lastPos, std::string::npos);

    return result;
}

/**
 * @brief Replaces all double-quoted substrings in the input string with "".
 *
 * Handles escaped quotes (e.g., \" inside strings) and does not use regex.
 *
 * @param str The input string to process.
 * @return std::string A new string with quoted substrings replaced by "".
 */
static std::string blankOutStrings(const std::string& str) {
    std::string result;
    bool inQuotes = false;
    size_t start = 0;

    for (size_t i = 0; i < str.length(); ++i) {
        // Start of quoted string
        if (!inQuotes && str[i] == '"') {
            inQuotes = true;
            result.append(str, start, i - start);  // Append text before quote
            start = i; // mark quote start
        }
        // Inside quoted string
        else if (inQuotes && str[i] == '"' && (i == 0 || str[i - 1] != '\\')) {
            // End of quoted string
            inQuotes = false;
            result += "\"\"";  // Replace quoted string with empty quotes
            start = i + 1;     // Next copy chunk starts after closing quote
        }
    }

    // Append remaining text after last quoted section
    if (start < str.size()) {
        result.append(str, start, str.size() - start);
    }

    return result;
}

static bool isWordChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

static bool isWordBoundary(const std::string& s, size_t pos)
{
    if (pos == 0 || pos >= s.size())
        return true;

    return isWordChar(s[pos - 1]) != isWordChar(s[pos]);
}

static bool isOnlyWhitespaceUntilNewline(const std::string& s, size_t pos)
{
    for (size_t i = pos; i < s.size(); ++i) {
        if (s[i] == '\n')
            return true;
        if (!std::isspace(static_cast<unsigned char>(s[i])))
            return false;
    }
    return true;
}



static std::string insertNewlineAfterSemicolon(const std::string& input)
{
    std::string output;
    const size_t n = input.size();

    for (size_t i = 0; i < n; ++i) {
        output += input[i];

        if (input[i] == ';') {
            size_t next = i + 1;

            if (next < n &&
                input[next] != '\n' &&
                !isOnlyWhitespaceUntilNewline(input, next))
            {
                output += '\n';
            }
        }
    }

    return output;
}

static std::string insertSpaceAfterComma(const std::string& input)
{
    std::string output;
    const size_t n = input.size();

    for (size_t i = 0; i < n; ++i) {
        output += input[i];

        if (input[i] == ',' && i + 1 < n && input[i + 1] != '\n') {
            output += ' ';
        }
    }

    return output;
}

static std::string insertNewlineAfterWords(
    const std::string& input,
    const std::unordered_set<std::string>& words)
{
    std::string output;
    const size_t n = input.size();

    for (size_t i = 0; i < n; ) {
        bool matched = false;

        for (const auto& word : words) {
            const size_t len = word.length();
            const size_t end = i + len;

            if (end <= n &&
                input.compare(i, len, word) == 0 &&
                isWordBoundary(input, i) &&
                isWordBoundary(input, end))
            {
                output += word;

                // Insert newline ONLY if meaningful text follows on same line
                if (end < n &&
                    input[end] != '\n' &&
                    !isOnlyWhitespaceUntilNewline(input, end))
                {
                    output += '\n';
                }

                i = end;
                matched = true;
                break;
            }
        }

        if (!matched) {
            output += input[i++];
        }
    }

    return output;
}

static std::string insertNewlineBeforeWords(
    const std::string& input,
    const std::unordered_set<std::string>& words)
{
    std::string output;
    const size_t n = input.size();

    for (size_t i = 0; i < n; ) {
        bool matched = false;

        for (const auto& word : words) {
            const size_t len = word.length();
            const size_t end = i + len;

            if (end <= n &&
                input.compare(i, len, word) == 0 &&
                isWordBoundary(input, i) &&
                isWordBoundary(input, end))
            {
                // Insert newline if previous char exists and is NOT whitespace
                if (i > 0 &&
                    input[i - 1] != '\n' &&
                    !std::isspace(static_cast<unsigned char>(input[i - 1])))
                {
                    output += '\n';
                }

                output += word;
                i = end;
                matched = true;
                break;
            }
        }

        if (!matched) {
            output += input[i++];
        }
    }

    return output;
}

// MARK: - Utills


static std::string reformatLine(const std::string& str) {
    static int depth = 0;
    std::string result = str;
    
    std::regex tokenRegex(R"(\b(?:BEGIN|FOR|IF|WHILE|REPEAT|CASE|UNTIL|ELSE)\b|END;)");
    std::smatch match;
    if (std::regex_search(result, match, tokenRegex)) {
        std::string token = match.str();
        
        if (token == "BEGIN" || token == "FOR" || token == "IF" ||
            token == "WHILE" || token == "REPEAT" || token == "CASE")
        {
            result.insert(0, std::string(depth * INDENT_WIDTH, ' '));
            depth++;
        }
        
        if (token == "END;" || token == "UNTIL")
        {
            depth--;
            result.insert(0, std::string(depth * INDENT_WIDTH, ' '));
        }
        
        if (token == "ELSE")
        {
            result.insert(0, std::string((depth - 1) * INDENT_WIDTH, ' '));
        }
    } else {
        result.insert(0, std::string(depth * INDENT_WIDTH, ' '));
    }
    
    return result + '\n';
}

static std::string reformatAllLines(std::istringstream& iss)
{
    std::string str;
    std::string result;
    
    while(getline(iss, str)) {
        result.append(reformatLine(str));
    }
    
    return result;
}

std::string reformat::prgm(const std::string& s)
{
    std::string str = s;
    std::regex re;

    // Keywords
    str = capitalizeWords(str, {
        "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
        "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
        "while", "repeat", "until", "break", "continue", "export", "const", "local", "key",
        "eval", "freeze", "view"
    });
    

    str = insertNewlineAfterWords(str, {"THEN", "DO", "REPEAT", "ELSE", "DEFAULT", "CASE"});
    str = insertNewlineBeforeWords(str, {"EXPORT", "LOCAL", "CONST", "IF", "CASE", "REPEAT", "FOR", "WHILE", "DEFAULT", "UNTIL", "ELSE", "IFERR", "END"});
    str = insertNewlineAfterSemicolon(str);
    
    auto python = extractPythonBlocks(str) ;
    str = blankOutPythonBlocks(str);
    auto strings = preserveStrings(str);
    str = blankOutStrings(str);
    str = cleanWhitespace(str);
    str = insertSpaceAfterComma(str);
    str = fixUnaryMinus(str);
    str = replaceOperators(str);
    
    std::istringstream iss;
    iss.str(str);
    str = reformatAllLines(iss);
    str = restoreStrings(str, strings);
    str = separatePythonMarkers(str);
    str = restorePythonBlocks(str, python);
    
    
    return str;
}







