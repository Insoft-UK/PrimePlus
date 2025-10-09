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


#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "aliases.hpp"
#include <fstream>
#include <deque>
#include <filesystem>
#include <string>

namespace pplplus {
    class Preprocessor {
    public:
        std::string filename;
        std::deque<std::filesystem::path> systemIncludePath;
        
        bool verbose = false;
        
        bool disregard = false;
        bool operators = true;
        bool logicalOperators = true;
        
        bool isIncludeLine(const std::string& str);
        bool isQuotedInclude(const std::string& str);
        bool isAngleInclude(const std::string& str);
        std::filesystem::path extractIncludePath(const std::string& str);
        std::string parse(const std::string& str);
        
        Preprocessor() {
            systemIncludePath.push_front(std::filesystem::path("/Applications/HP/PrimeSDK/include"));
        }
        
    };
    
}

#endif /* PREPROCESSOR_HPP */

