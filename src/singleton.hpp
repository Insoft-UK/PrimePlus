// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft. All rights reserved.
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


#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <iostream>
#include <vector>
#include "aliases.hpp"
#include "auto.hpp"
#include "comments.hpp"
#include "common.hpp"
#include "regexp.hpp"

using namespace pp;

class Singleton {

public:
    Aliases aliases;
    Auto autoname;
    Comments comments;
    Regexp regexp;
    
    typedef struct {
        std::string endCode;
    } TScopeDepth;
    
    const std::vector<TScopeDepth>& scopeDepth;
    
    static Singleton *shared();
    
    void incrementLineNumber(void);
    long currentLineNumber(void);
    std::string currentPath(void);
    std::string getProjectPath(void);
    void pushPath(const std::string &path);
    void popPath(void);
    
    void increaseScopeDepth(const std::string &endCode = "") {
        TScopeDepth scopeDepth = {
            .endCode = endCode
        };
        _scopeDepth.push_back(scopeDepth);
    }
    
    void decreaseScopeDepth() {
        if (_scopeDepth.size() == 0) {
            //std::cout << MessageType::Error << "unexpected '" << "..." << "'\n";
            return;
        }
        _scopeDepth.pop_back();
    }
    
private:
    std::vector<std::string> _paths;
    std::vector<long> _lines;
    static Singleton *_shared;
    
    std::vector<TScopeDepth> _scopeDepth;
    
    Singleton() : scopeDepth(_scopeDepth) {
        _currentline = 1;
        
    }
    Singleton(const Singleton &);
    Singleton &operator=(const Singleton &);
    
protected:
    long _currentline;
};


#endif /* SINGLETON_HPP */
