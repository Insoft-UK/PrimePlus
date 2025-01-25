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
#include <pplang>
#include <prime>
#include <string>

#include <pplang>


@disregard
 When defining a macro with parameters, issues can arise if an argument
 name is adjacent to a letter. To resolve this, you can use numbered
 placeholders like `1$` for the first argument, `2$` for the second, `3$`
 for the third, and so on. This ensures the macro arguments remain uniquely
 identifiable and avoid issues, note that $0 is the identifier.
@end

#define MacroList(L)      L$1
#define COPYWRITE "Copyright (c) 2023-2025 Insoft. All rights reserved."

#PYTHON
#END



NAME:Very_Long_Name()
{
    A := 0x0;
    
    do {
        A += 1;
    } while A < 10;
    
    A *= B;

    if A == 100 && (C <= 10 || C == 100) {
        A /= C;
    }
    
    while A > 0 {
        A -= 1;
    }

    auto copywriteText;
    copywriteText = COPYWRITE;
    auto mt;
    mt = MouseType.LongClick;
    mt = integer::Base.Decimal;
}

auto:Function_Name_Uknown()
begin
    Very_Long_Name();
end;

Implied::AutoNameBecauseNotValidInPPL()
begin
end;

ValidPPLFunctionName()
begin
    // Very_Long_Name()
    Very_Long_Name();
    NAME();
        
    // Function_Name_Uknown()
    Function_Name_Uknown();
    
    // Implied::AutoNameBecauseNotValidInPPL()
    Implied::AutoNameBecauseNotValidInPPL();
end;

export Parameters(auto:a, b:beta)
begin
    auto alpha;
    alpha = beta + a;
end;

PPL()
BEGIN
    // PPL & P+
    LOCAL a;
    a = Int8(a);
    var b:beta;
    
    
    IF a==beta THEN
        a ▶ beta;
        if a==1 then
            a = a + 1;
        endif;
    END;
END;

// Namespace
using namespace std;
Namespace()
begin
    def RECT std::cartesian::rect;
    std::cartesian::rect(0);
    cartesian::rect(0);
end;
remove namespace std;

Var()
begin
    var a, b; var c, d;
    auto iAmVerryLong;
end;

Parameter(a:alpha, auto:iAmUknown)
begin
    alpha = alpha + iAmUknown;
    a = alpha + iAmUknown;
end;


Branch()
begin
    var a, b;
    
    if a == b then
        a = a + 1; else 10 := a;
    endif;
    
    if a <= b and b > 10 then a = a + 1; endif;
    if a != b and b > 10 then
    a = a + 1; endif;
    if a <> b and b > 10 then a = a + 1; endif;
    if a >= b or b < 20 then a = a + 1; endif;
    if a == b or b < 20 then a = a + 1; endif;
end;

Loop()
begin
    var a, b = 10;
    
    for a from 0 ... 9 step 1 do
        b - 1 ▶ b;
    end;
    
    while a != b do
        a = b;
    wend;
    
    repeat
        a = a + 1;
    until a == b;
    
end;

Auto()
begin
    auto iWasAVeryLongName, b;
    auto iWas::Invalid = 0; // invalid because it's asigning a value.
    var a:thisIsMyLongName, auto:letYouDecideMyShortName;
    
    iWasAVeryLongName = thisIsMyLongName + b;
    a = thisIsMyLongName + b;
    iWasAVeryLongName *= letYouDecideMyShortName;
end;


#define SEVEN 7
PreCalc()
begin
    var ppl_numbers = \[320 / #2d];


#if __VERSION >= 304
    var new = \` 1 + pi * 4 / 2 & 2 % SEVEN `;
    LOCAL new = \` 10.0 + SEVEN `;
#endif
end;

#PPL
I_AM_PPL_CODE()
BEGIN
    LOCAL a; LOCAL b;
    var b; // I am not PPL Code!
END;
#END
