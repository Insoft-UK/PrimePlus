@disregard
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
@end
#pragma mode( separator(.,,) integer(h64) )

#include <cartesian>
#include <cspace>
#include <dictionary>
#include <hp>
#include <integer>
#include <io>
#include <matrix>
#include <pixel>
#include <prime>
#include <string>
#include <clang>
#include <pplang>

@disregard
 When defining a macro with parameters, issues can arise if an argument
 name is adjacent to a letter. To resolve this, you can use numbered
 placeholders like `1$` for the first argument, `2$` for the second, `3$`
 for the third, and so on. This ensures the macro arguments remain uniquely
 identifiable and avoid issues, note that $0 is the identifier.
@end

#define MacroList(i)      L$1
#define COPYWRITE "Copyright (c) 2023-2025 Insoft. All rights reserved."

#PYTHON
#END

// Namespace
namespace std::cartesian::rect:=RECT;

Example:AVeryLongName(p:first, q:second)
begin
    // Local Variables with aliases
    local a:alpha, b:beta;
    
    // Macro
    alpha := MacroList(1);
    
    // b
    beta := first + second;
    
    if a <= b and b > 10 then a = a + 1; end;
    if a != b and b > 10 then a = a + 1; end;
    if a <> b and b > 10 then a = a + 1; end;
    if a >= b or b < 20 then a = a + 1; end;
    if a == b or b < 20 then a = a + 1; end;
    
    // Pre-Calculate
    #define VALUE 5
    local pre_calculated := \2`10.0 + VALUE + #Ah`;
    local hex := \`#A:2h`;
    local bin := \`#1111:3b`;
    local oct := \`#1111:3o`;
    local dec := \`#15:3d`;
    
    // LOCAL auto variable name
    auto iAmVerryLong;

#PPL
  LOCAL a; LOCAL b;
#END

    using std::cartesian;
    std::cartesian::rect();
    rect();
end;

auto:myFunction()
begin
    AVeryLongName(2,5);
end;

myFunction();

void clang()
{
    for (local A:=0; A <= 10; A := A + 1) {
        B := B + A;
    }
    
    while (A > 0) {
        A := 0;
    }
}


