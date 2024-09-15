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


#pragma ( messages )

#include <prime>
#include <hp>

global auto:gg;


#define bb aa


myFunction()
begin
local auto:aa;
#define cc aa
aa++;
bb++;
cc++;
gg++;
    local longname;
    longname = <calc>(π / 2 :2);
    
    
    do
        if NOT(A) then
            return;
        endif;
    loop;
    try
    catch
    else
    end;
end;

Example::DoLoop(auto:value);
gg++;

Example::Switch()
begin
    gg++;
    struct Event auto:event;
    event = hp::waitEvent;
    
    if [hp isKeyPressedForEvent:event] == true then
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
    endif;
end

Example::DoLoop(auto:value)
begin
    do
        if value <= 0 then
            return;
        endif;
        
        value -= 1;
    loop;
end;

Example::ForNext()
begin
    local n:number, auto:count = 0;
    
    for number = 0; number < 100; number += 1 do
        count += 1;
    next
    return count;
end;

export Example:MyLongNameExample(auto:value)
begin
    local piTest = #[2*pi]:0;
    hp::storeScreen;
    
    struct MyStruct
        one[1];
        two[2];
    end;
    
    struct MyStruct auto:myStruct;
    
    Example::DoLoop(Example::Switch);
  
    myStruct = {value, [Example ForNext]};
    return myStruct.one + myStruct.two;
    
    if value <= π do
        KILL;
    endif;
    
    R→B(value,4)=>value;
    
    String(#[5.5/7]);
    local t = IP(0.06);
    local t = <calc>(<calc>(1+1)^-4:2);
    var t = <int>(#[#[1+1]^-4]:2);
    
end;



