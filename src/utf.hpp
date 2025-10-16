// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft.
//
// Created: 2025-05-27
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

#ifndef utf_hpp
#define utf_hpp

#include <sstream>
#include <fstream>
#include <cstdlib>

namespace utf {
    enum BOM {
        BOMle,
        BOMbe,
        BOMnone
    };
    
    [[deprecated("Use utf8 instead")]]
    std::string to_utf8(const std::wstring& wstr);
    
    [[deprecated("Use utf16 instead")]]
    std::wstring to_utf16(const std::string& str);
    
    [[deprecated]]
    std::wstring read_as_utf16(std::ifstream& is);
    
    [[deprecated("Use read instead")]]
    std::wstring read_utf16(std::ifstream& is);
    
    [[deprecated("Use load instead")]]
    std::wstring load_utf16(const std::string& filepath);
    
    [[deprecated("Use write instead")]]
    size_t write_utf8(std::ofstream& os, const std::string& str);
    
    [[deprecated("Use save instead")]]
    bool save_as_utf8(const std::string& filepath, const std::string& str);
    
    [[deprecated("Use write instead")]]
    size_t write_as_utf16(std::ofstream& os, const std::string& str);
    
    [[deprecated("Use write instead")]]
    size_t write_utf16(std::ofstream& os, const std::wstring& wstr);
    
    [[deprecated("Use save instead")]]
    bool save_as_utf16(const std::string& filepath, const std::string& str);
    
    std::string utf8(const std::wstring& wstr);
    std::wstring utf16(const std::string& str);
    std::wstring read(std::ifstream& is, BOM bom = BOMle);
    std::wstring load(const std::string& filepath, BOM bom = BOMle);
    size_t write(std::ofstream& os, const std::string& str);
    size_t write(std::ofstream& os, const std::wstring& wstr, BOM bom = BOMle);
    bool save(const std::string& filepath, const std::string& str);
    bool save(const std::string& filepath, const std::wstring& wstr, BOM bom = BOMle);
};

#endif /* utf_hpp */
