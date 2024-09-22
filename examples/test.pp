/*
 The MIT License (MIT)
 
 Copyright (c) 2023-2024 Insoft. All rights reserved.
 
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


#pragma ( messages )



Operators()
begin
    var a, b;

    a >> b;
    a << b;
    a & b;
    a | b;
    a = b;
    
    a += b;
    a -= b;
    a /= b;
    a %= b;
    a &= b;
    a |= b;
    a ^= b;
    
    a <<= b;
    a >>= b;
end;

Branch()
begin
    var a, b;

    if a <= b && b > 10 then a += 1; endif;
    if a != b && b > 10 then a += 1; endif;
    if a <> b && b > 10 then a += 1; endif;
    if a >= b || b < 20 then a += 1; endif;
    if a == b || b < 20 then a += 1; endif;
end;

Loop()
begin
    var a, b = 10;
    
    for a = 0; a <= 10; a += 1 do
        b += 1;
    next;
    
    for a := 0 step 1 do
        b - 1 ▶ b;
    end;
    
    while a != b do
        a = b;
    wend;
    
    repeat
        a += 1;
    until a == b;
    
    do
        break;
    loop;
end;

Auto()
begin
    auto iWasAVeryLongName, b;
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

Struct()
begin
    struct TMyStruct
        first[1];
        second[2];
    end;
    
    struct TMyStruct auto:myStruct;
    
    myStruct.first = 1;
    myStruct.second = 2;
end;

Try()
begin
    var a, b = 0;
    try
        a += 1;
        a /= b;
    catch
        return -1;
    else
        return 0;
    end;
end;




