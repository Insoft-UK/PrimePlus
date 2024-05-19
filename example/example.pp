/*
Copyright © 2024 Insoft. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma ( minify 0, tabsize 1 )

#include <prime>
#include <hp>

Example::Switch()
begin
    struct Event auto:event;
    event = hp::waitEvent;
    
    if [hp isKeyPressedForEvent:event] == true do
        switch event.key
            case KeyCode.Esc do
                return 0;
            end
            
            case KeyCode.1 do
                return 10;
            end
            
            case KeyCode.2 do
                return 100;
            end
            
            case KeyCode.3 do
                return 1000;
            end
        end
    endif
end

Example::DoLoop(auto:value)
begin
    do
        if value <= 0 then return;
        value -= 1;
    loop
end

Example::ForNext()
begin
    var n:number, auto:count = 0;
    
    for number = 0; number < 100; number += 1 do
        count += 1;
    next
    return count;
end

export Example:MyLongNameExample(auto:value)
begin
    var piTest = 2*pi;
    hp::storeScreen;
    
    struct MyStruct
        one[1];
        two[2];
    end
    
    struct MyStruct auto:myStruct;
    
    [Example DoLoopWithValueOf[Example Switch]];
  
    myStruct = {value, [Example ForNext]};
    return myStruct.one + myStruct.two;
    
    if value <= π do
        KILL;
    endif
    
    R→B(value,4)=>value;
    
    var t = #[320/(7%3/-2*2)--6];
end
