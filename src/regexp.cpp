// The MIT License (MIT)
//
// Copyright (c) 2023 Insoft. All rights reserved.
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

using namespace pp;

bool Regexp::parse(std::string &str) {
    std::regex re;
    std::smatch match;
    
    re = R"(^ *regex +`([^`]*)` +(.*)$)";
    if (regex_search(str, match, re)) {
        TRegexp regexp = {
            .regularExpression = match[1].str(),
            .replacement = match[2].str(),
            .scopeLevel = Singleton::shared()->scopeDepth.size(),
            .line = Singleton::shared()->currentLineNumber(),
            .pathname = Singleton::shared()->currentPathname()
        };
        
        if (regularExpressionExists(regexp.regularExpression)) return true;
        
        _regexps.push_back(regexp);
        str = std::string("");
        
        if (verbose) std::cout
            << MessageType::Verbose
            << "regex"
            << " '" << ANSI::Green << regexp.regularExpression << ANSI::Default << "' for '" << ANSI::Green << regexp.replacement << ANSI::Default << "' defined\n";
        return true;
    }
    
    return false;
}

void Regexp::removeAllOutOfScopeRegexps() {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->scopeLevel < Singleton::shared()->scopeDepth.size()) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "regex"
                << " '" << ANSI::Green << it->regularExpression << ANSI::Default << "' removed❗\n";
            
            _regexps.erase(it);
            removeAllOutOfScopeRegexps();
            break;
        }
    }
}

void Regexp::resolveAllRegularExpression(std::string &str) {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        str = regex_replace(str, std::regex(it->regularExpression), it->replacement);
    }
}

bool Regexp::regularExpressionExists(const std::string &regularExpression) {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->regularExpression == regularExpression) {
            std::cout
            << MessageType::Warning
            << "redefinition of: " << ANSI::Bold << regularExpression << ANSI::Default << ", ";
            if (basename(Singleton::shared()->currentPathname()) == basename(it->pathname)) {
                std::cout << "previous definition on line " << it->line << "\n";
            }
            else {
                std::cout << "previous definition in '" << ANSI::Green << basename(it->pathname) << ANSI::Default << "' on line " << it->line << "\n";
            }
            return true;
        }
    }
    
    return false;
}



