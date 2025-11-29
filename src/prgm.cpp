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

#include "prgm.hpp"

static inline std::vector<uint8_t> readBytes(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open file");

    std::vector<uint8_t> buf((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());
    buf.push_back(0);
    buf.push_back(0);
    return buf;
}

static inline void writeBytes(const std::filesystem::path& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot write file");
    f.write((const char*)data.data(), data.size());
}

static inline std::wstring utf16le(const std::string& s) {
    std::wstring out;
    out.reserve(s.size());
    uint32_t code = 0;
    int bytes = 0;

    for (unsigned char c : s) {
        if (c <= 0x7F) {                       // 1-byte
            out.push_back(c);
        }
        else if (c >> 5 == 0x6) {              // 2-byte
            code = c & 0x1F;
            bytes = 1;
        }
        else if (c >> 4 == 0xE) {              // 3-byte
            code = c & 0x0F;
            bytes = 2;
        }
        else if (c >> 3 == 0x1E) {             // 4-byte
            code = c & 0x07;
            bytes = 3;
        }
        else if (bytes) {
            code = (code << 6) | (c & 0x3F);
            if (--bytes == 0) {
                if (code <= 0xFFFF) out.push_back(code);
                else {
                    code -= 0x10000;
                    out.push_back((code >> 10) + 0xD800);
                    out.push_back((code & 0x3FF) + 0xDC00);
                }
            }
        }
    }
    return out;
}

void prgm::buildHPPrgm(const std::filesystem::path& path, const std::string& name, const std::vector<uint8_t>& prgm)
{
    std::vector<uint8_t> header = {
        0x0C, 0, 0, 0, 0x7F, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    
    auto prgmSize = prgm.size();
    header.push_back(prgmSize & 0xFF);
    header.push_back((prgmSize >> 8) & 0xFF);
    header.push_back((prgmSize >> 16) & 0xFF);
    header.push_back(prgmSize >> 24);
    
    std::vector<uint8_t> out;
    out.reserve((uint32_t)(header.size() + prgm.size()));
    out.insert(out.end(), header.begin(), header.end());
    out.insert(out.end(), prgm.begin(), prgm.end());

    writeBytes(path, out);
}

std::vector<uint8_t> prgm::loadPrgm(const std::filesystem::path& prgmPath) {
    auto raw = readBytes(prgmPath);
    if (raw.size() < 2)
        throw std::runtime_error("Invalid .prgm file");
    
    if (raw[0] == 0xFF && raw[1] == 0xFE) {
        raw.erase(raw.begin(), raw.begin() + 2);
    }
    
    return raw;
}








