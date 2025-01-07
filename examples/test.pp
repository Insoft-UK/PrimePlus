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

#include <dictionary>
#include <prime>


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


#if __VERSION >= 300
    var text = {"P+ 3.x"};
#else
    var text = {"P+ 2.x"}
#endif


fn1:Function1()
begin
    auto copywriteText = COPYWRITE;
end;

auto:Function2()
begin
    Function1();
end;

Implied::AutoNameBecauseNotValidInPPL()
begin
    return 0;
end;

ValidPPLFunctionName()
begin
    Implied::AutoNameBecauseNotValidInPPL();
end;

export Parameters(auto:a, b:beta)
begin
    def L$1 DefList(n);
    
    auto alpha;
    var auto:beta;
    const z:zero;
    
    alpha += beta + zero;
    
    MacroList(1) = 0;
    DefList(1) = 0;
end;

PPL()
BEGIN
    // PPL & P+
    LOCAL a;
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
    alpha += iAmUknown;
    a += iAmUknown;
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

Switch()
begin
    var a = 0;
    
    switch a
        case 0 do
            a += 1;
        end;
        
        case 1 do
            a -= 1;
        end;
        
        default
            switch b
                case 0 do
                    b =1;
                end;
            end;
    end;
    
    case
        if a == 0 then a += 1; end;
        if a == 1 then a -= 1; end;
    end;
end;



Try()
begin
    local a, b = 0;
    try
        a += 1;
        a /= b;
    catch
        return -1;
    else
        return 0;
    end;
end;

PreCalc()
begin
    var ppl_numbers = \[320 / #2d];
#if __VERSION < 303
    var old = #[ 1 + 2 * 4 / 2 ]:2;
#endif
#if __VERSION >= 202
    var new = \ 2 [ 1 + pi * 4 / 2 & 2 % 7 ];
#endif
end;

#PPL
I_AM_PPL_CODE()
BEGIN
    LOCAL a; LOCAL b;
    var b; // I am not PPL Code!
END;
#END
