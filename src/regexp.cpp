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
    
        if (regularExpressionExists(regexp.pattern)) return true;
        
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

void Regexp::resolveAllRegularExpression(std::string &str) {
    std::smatch match;
    std::regex re;
    
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
            str = regex_replace(str, re, it->replacement);
           
            std::string key = "__SCOPE__";
            std::string value = std::to_string(Singleton::shared()->scopeDepth);

            size_t pos;
            while ((pos = str.find(key)) != std::string::npos) {
                str.replace(pos, key.length(), value);
            }
            
            key = "__++COUNT__";
            while ((pos = str.find(key)) != std::string::npos) {
                Singleton::shared()->increaseCount();
                str.replace(pos, key.length(), std::to_string(Singleton::shared()->count));
            }
            
            key = "__COUNT__";
            while ((pos = str.find(key)) != std::string::npos) {
                str.replace(pos, key.length(), std::to_string(Singleton::shared()->count));
            }
            
            key = "__COUNT++__";
            while ((pos = str.find(key)) != std::string::npos) {
                str.replace(pos, key.length(), std::to_string(Singleton::shared()->count));
                Singleton::shared()->increaseCount();
            }
            
            // TODO: resolve \t
            
            Calc::evaluateMathExpression(str);
            resolveAllRegularExpression(str);
        }
    }
}

bool Regexp::regularExpressionExists(const std::string &pattern) {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->pattern == pattern) {
            std::cout
            << MessageType::Warning
            << "regular expresion already defined. previous definition at " << basename(it->pathname) << ":" << it->line << "\n";
            return true;
        }
    }
    
    return false;
}



