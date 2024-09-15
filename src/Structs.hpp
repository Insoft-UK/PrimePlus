/*
 The MIT License (MIT)
 
 Copyright (c) 2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */


#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include "Singleton.hpp"
#include <iostream>
#include <vector>
#include <list>

namespace pp {

class Structs {
public:
    bool parsing = false;
    bool verbose = false;

    
    bool parse(std::string &str);
    
private:
    struct _Structure {
        std::string identifier;
        std::vector<std::string> members;
    };
    std::vector<_Structure> _structures;
    _Structure _structure;
    
    void defineStruct(const _Structure &structure, const std::string &real, const std::string &identifier);
    void createStructureVariableMembers(_Structure *structure, const std::string &structureVariableName);
    void createStructureVariable(std::string &str);
};

}

#endif /* STRUCTS_HPP */
