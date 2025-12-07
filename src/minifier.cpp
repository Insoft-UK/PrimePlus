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

#include "minifier.hpp"

// MARK: - ● String cleanup helpers

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
 */
static std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

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
static std::string replaceWords(const std::string& input, const std::vector<std::string>& words, const std::string& replacement) {
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
            result += replacement;
            continue;
        }
        
        result += word;
    }
    
    return result;
}

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

static std::string cleanWhitespace(const std::string& input) {
    std::string output;
    char current = '\0';
    
    auto iswordc = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    };
    
    for (size_t i = 0; i < input.length(); i++) {
        if (std::isspace(static_cast<unsigned char>(current))) {
            if(iswordc(input[i]) && !output.empty() && iswordc(output.back())) {
                output += ' ';
            }
        }
        current = input[i];

        // by Jozef Dekoninck: && current != '\n' to exclude the deletion of the LF character.
        if (std::isspace(static_cast<unsigned char>(current)) && current != '\n') {
            continue;
        }

        output += current;
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

// MARK: - ● BEGIN/END block parsing

static std::string base10ToBase32(unsigned int num) {
    if (num == 0) {
        return "0";  // Edge case: if the number is 0, return "0"
    }

    std::string result;
    const char digits[] = "0123456789ABCDEFGHIJKLMNabcdefgh";  // Base-32 digits
    
    // Keep dividing the number by 32 and store the remainders
    while (num > 0) {
        int remainder = num % 32;  // Get the current base-32 digit
        result += digits[remainder];  // Add the corresponding character
        num /= 32;  // Reduce the number
    }

    // The digits are accumulated in reverse order, so reverse the result string
    std::reverse(result.begin(), result.end());

    return result;
}

static std::pair<size_t, size_t> findBeginEndBlock(const std::string& code, size_t beginPos = 0) {
    std::regex tokenRegex(R"(\b(?:BEGIN|FOR|IF|WHILE|REPEAT|CASE)\b|END;)");
    std::smatch match;
    
    std::regex pattern(R"(\bBEGIN\b)");
    // Create iterators for all matches
    auto begin = std::sregex_iterator(code.begin(), code.end(), pattern);
    auto end = std::sregex_iterator();

    // Loop through all matches
    for (auto it = begin; it != end; ++it) {
        std::smatch match = *it;
        if (match.position() >= beginPos) {
            beginPos = match.position() + match.length();
            break;
        }
        if (std::next(it) == end) return {std::string::npos, std::string::npos};
    }
    
    
    size_t searchPos = beginPos;
    int depth = 1;
    
    while (std::regex_search(code.begin() + searchPos, code.end(), match, tokenRegex))
    {
        std::string token = match.str();
        size_t pos = match.position() + searchPos;
        
        if (token == "BEGIN" || token == "FOR" || token == "IF" ||
            token == "WHILE" || token == "REPEAT" || token == "CASE")
        {
            depth++;
        }
        else if (token == "END;")
        {
            depth--;
            if (depth == 0)
            {
                size_t endPos = pos; // start of matching END;
                return {beginPos, endPos};
            }
        }
        
        searchPos = pos + token.length();
    }
    
    return {std::string::npos, std::string::npos};
}

// Extracts variable names from all LOCAL declarations
static std::vector<std::string> extractLocalVariables(const std::string& code) {
    std::vector<std::string> variables;

    // Match full LOCAL lines: e.g. LOCAL a, b:=10;
    std::regex localRegex(R"(\bLOCAL\b([^;:]*);)", std::regex_constants::icase);
    std::smatch match;
    std::string::const_iterator searchStart(code.cbegin());

    while (std::regex_search(searchStart, code.cend(), match, localRegex))
    {
        std::string locals = match[1]; // everything after LOCAL up to ;
        
        // Split by commas
        std::regex varRegex(R"([A-Za-z_]\w*)");
        std::smatch varMatch;
        std::string::const_iterator varSearch(locals.cbegin());

        while (std::regex_search(varSearch, locals.cend(), varMatch, varRegex))
        {
            std::string varName = varMatch.str();
            variables.push_back(varName);
            varSearch = varMatch.suffix().first;
        }

        searchStart = match.suffix().first;
    }

    return variables;
}

static std::string shortenVariableNames(const std::string& code) {
    auto str = code;
    auto pos = findBeginEndBlock(str);
    
    if (pos.first == std::string::npos && pos.second == std::string::npos) return str;
   
    do {
        auto s = str.substr(pos.first, pos.second - pos.first);
        
        int count = 0;
        auto vars = extractLocalVariables(s);
        for (const auto& v : vars) {
            if (v.length() < 3) continue;
            s = replaceWords(s, {v}, "v" + std::to_string(++count));
        }
        str = str.replace(pos.first, pos.second - pos.first, s);
        pos = findBeginEndBlock(str, pos.first);
           
    } while (pos.first != std::string::npos && pos.second != std::string::npos);
    
    return str;
}

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


// MARK: - 📣 Public API functions

std::string minifier::minify(const std::string& code) {
    std::string str = code;
    
    auto python = extractPythonBlocks(str);
    str = blankOutPythonBlocks(str);
    
    auto strings = preserveStrings(str);
    str = blankOutStrings(str);
    
    str = shortenVariableNames(str);
    str = replaceWords(str, {"FROM"}, ":=");
    
    str = cleanWhitespace(str);
    str = fixUnaryMinus(str);
    
    str = restoreStrings(str, strings);
    str = separatePythonMarkers(str);
    str = restorePythonBlocks(str, python);
    
    str = regex_replace(str, std::regex(R"(^#pragma mode\(([a-z]+\([^()]+\))+\))"), "$0\n");
    str = regex_replace(str, std::regex(R"(\n{2,})"), "\n");
    str = regex_replace(str, std::regex(R"(;\s*)"), ";");
    
    std::regex re(R"((?:\b(EXPORT|LOCAL) )?([a-zA-Z]\w*)\([a-zA-Z,]*\)\s*(?=BEGIN\b))");
    std::sregex_iterator begin(str.begin(), str.end(), re);
    std::sregex_iterator end;
    std::string result = str;
    int fn = 0;
    
    for (auto it = begin; it != end; ++it) {
        std::smatch match = *it;

        if (match[1].str() == "LOCAL") {
            result = replaceWords(result, {match[2].str()}, "fn" + base10ToBase32(fn++));
        }
    }
    
    return result;
}
