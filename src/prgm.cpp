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

// MARK: - Helper Functions

static inline void writeBytes(const std::filesystem::path& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot write file");
    f.write((const char*)data.data(), data.size());
}

static inline std::vector<uint8_t> utf16le(const std::string& s)
{
    std::vector<uint8_t> out;
    out.reserve(s.size() * 2); // worst case UTF-16 length

    uint32_t code = 0;
    int bytesNeeded = 0;

    for (unsigned char c : s) {

        if (bytesNeeded == 0) {
            // Start of a new UTF-8 sequence
            if (c <= 0x7F) {
                // 1-byte ASCII
                uint16_t v = c;
                out.push_back(v & 0xFF);
                out.push_back((v >> 8) & 0xFF);
            }
            else if ((c & 0xE0) == 0xC0) {
                code = c & 0x1F;
                bytesNeeded = 1;
            }
            else if ((c & 0xF0) == 0xE0) {
                code = c & 0x0F;
                bytesNeeded = 2;
            }
            else if ((c & 0xF8) == 0xF0) {
                code = c & 0x07;
                bytesNeeded = 3;
            }
            else {
                // invalid UTF-8 start byte
                continue;
            }
        }
        else {
            // continuation byte
            if ((c & 0xC0) != 0x80) {
                // invalid UTF-8 continuation
                bytesNeeded = 0;
                continue;
            }

            code = (code << 6) | (c & 0x3F);

            if (--bytesNeeded == 0) {
                if (code <= 0xFFFF) {
                    // Direct UTF-16
                    uint16_t v = static_cast<uint16_t>(code);
                    out.push_back(v & 0xFF);
                    out.push_back((v >> 8) & 0xFF);
                }
                else {
                    // Surrogate pair
                    code -= 0x10000;
                    uint16_t high = 0xD800 + (code >> 10);
                    uint16_t low  = 0xDC00 + (code & 0x3FF);

                    out.push_back(high & 0xFF);
                    out.push_back((high >> 8) & 0xFF);

                    out.push_back(low & 0xFF);
                    out.push_back((low >> 8) & 0xFF);
                }
            }
        }
    }

    out.push_back(0);
    out.push_back(0);
    return out;
}

// MARK: - 📣 Public API functions

void prgm::buildHPPrgm(const std::filesystem::path& path, const std::string& name, const std::string& prgm)
{
    std::vector<uint8_t> header = {
        0x0C, 0, 0, 0, 0x7F, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    
    auto sourceCode = utf16le(prgm);
    auto prgmSize = sourceCode.size();
    header.push_back(prgmSize & 0xFF);
    header.push_back((prgmSize >> 8) & 0xFF);
    header.push_back((prgmSize >> 16) & 0xFF);
    header.push_back(prgmSize >> 24);
    
    std::vector<uint8_t> out;
    out.reserve((uint32_t)(header.size() + sourceCode.size()));
    out.insert(out.end(), header.begin(), header.end());
    out.insert(out.end(), sourceCode.begin(), sourceCode.end());

    writeBytes(path, out);
}









