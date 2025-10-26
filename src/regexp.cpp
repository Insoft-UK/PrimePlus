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

#include "regexp.hpp"
#include "common.hpp"
#include "singleton.hpp"
#include "calc.hpp"

using pplplus::Regexp;

bool Regexp::parse(const std::string &str) {
    std::regex re;
    std::smatch match;
    
    re = R"(^ *\bregex +([@<>=≠≤≥~])?`([^`]*)`(i)? *(.*)$)";
    if (regex_search(str, match, re)) {
        TRegexp regexp = {
            .pattern = match[2].str(),
            .insensitive = match[3].matched,
            .replacement = match[4].str(),
            .scopeLevel = static_cast<size_t>(Singleton::shared()->scopeDepth),
            .line = Singleton::shared()->currentLineNumber(),
            .pathname = Singleton::shared()->currentSourceFilePath()
        };
        
        if (match[1].matched) {
            if (match[1].str() == "@") {
                regexp.scopeLevel = 0;
            } else {
                
                if (match[1].str() == "~") {
                    regexp.scopeLevel = 1;
                    regexp.compare = "=";
                } else {
                    regexp.compare = match[1].str();
                }
            }
        }
        
        if (regularExpressionExists(regexp.pattern, regexp.compare)) return true;
        
        _regexps.push_back(regexp);
        if (verbose) std::cout
            << MessageType::Verbose
            << "defined " << (regexp.scopeLevel ? "local " : "") << "regular expresion "
            << "`" << ANSI::Green << regexp.pattern << ANSI::Default << "`\n";
        return true;
    }
    
    return false;
}

void Regexp::removeAllOutOfScopeRegexps() {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->scopeLevel > Singleton::shared()->scopeDepth) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "removed " << (it->scopeLevel ? "local " : "") <<"regular expresion `" << ANSI::Green << it->pattern << ANSI::Default << "`\n";
            
            _regexps.erase(it);
            removeAllOutOfScopeRegexps();
            break;
        }
    }
}

/*
 * Custom parser macros:
 *
 *  __COUNTER__  – Substituted with the current running counter value.
 *                The counter increments each time this token is processed.
 *
 *  __LINE__   – Replaced with the current line number being parsed.
 *                Useful for debugging or generated code tracking.
 *
 *  __RESET__  – Resets the internal counter to zero and removes itself
 *                from the output (produces a blank value).
 *
 * These macros are handled by the parser during preprocessing and are not
 * part of standard C++ preprocessor behavior.
 */
static std::string resolve(const std::string &str) {
    std::regex re;
    std::smatch match;
    std::string::const_iterator it;
    std::string output = str;
    
    re = R"(__SCOPE__|__LINE__|__COUNTER__|__RESET__)";
    it = output.cbegin();
    while (std::regex_search(it, output.cend(), match, re)) {

        if (match.str() == "__SCOPE__") {
            output.replace(match.position(), match.length(), std::to_string(pplplus::Singleton::shared()->scopeDepth));
            it = output.cbegin();
            continue;
        }
        
        if (match.str() == "__COUNTER__") {
            output.replace(match.position(), match.length(), std::to_string(pplplus::Singleton::shared()->count));
            pplplus::Singleton::shared()->advanceCount();
            it = output.cbegin();
            continue;
        }
        
        if (match.str() == "__LINE__") {
            output.replace(match.position(), match.length(), std::to_string(pplplus::Singleton::shared()->currentLineNumber()));
            it = output.cbegin();
            continue;
        }
        
        
        if (match.str() == "__RESET__") {
            pplplus::Singleton::shared()->resetCount();
        }
        
        // Erase only the matched portion and update the iterator correctly
        it = output.erase(it + match.position(), it + match.position() + match.length());
        it = output.cbegin();
    }
    
    return output;
}

void Regexp::resolveAllRegularExpression(std::string &str, const size_t index) {
    std::smatch match;
    std::regex re;
    
    // index is used to prevent the function from entering a recursive loop.
    
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (!it->compare.empty()) {
            auto currentScopeLevel = Singleton::shared()->scopeDepth;
            if (it->compare == "<" && currentScopeLevel >= it->scopeLevel) continue;
            if (it->compare == ">" && currentScopeLevel <= it->scopeLevel) continue;
            if (it->compare == "=" && currentScopeLevel != it->scopeLevel) continue;
            if (it->compare == "≠" && currentScopeLevel == it->scopeLevel) continue;
            if (it->compare == "≤" && currentScopeLevel > it->scopeLevel) continue;
            if (it->compare == "≥" && currentScopeLevel < it->scopeLevel) continue;
        }
        if (it->insensitive)
            re = std::regex(it->pattern, std::regex_constants::icase);
        else
            re = std::regex(it->pattern);
        
        if (std::regex_search(str, match, re)) {
            size_t i = std::distance(_regexps.begin(), it);
            
            // If the function encounters the same index again, it means recursion is repeating.
            // Reset prev_index and exit to stop an infinite recursive loop.
            if (index == i) {
                return;
            }
            str = regex_replace(str, re, it->replacement);
            str = resolve(str);
            Calc::evaluateMathExpression(str);
            
            resolveAllRegularExpression(str, i);
        }
    }
}

bool Regexp::regularExpressionExists(const std::string &pattern, const std::string &compare) {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->pattern == pattern && it->compare == compare) {
            std::cout
            << MessageType::Warning
            << "regular expresion already defined. previous definition at " << basename(it->pathname) << ":" << it->line << "\n";
            return true;
        }
    }
    
    return false;
}



