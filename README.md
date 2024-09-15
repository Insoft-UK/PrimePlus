![Social Preview](https://raw.githubusercontent.com/Insoft-UK/PrimePlus/main/assets/social-preview.png)
P+ is a proprietary programming language designed to mitigate the challenges associated with code maintenance and comprehensibility in the HP Programming Language (PPL). P+ serves as an intermediary language that addresses these issues by introducing support for pre-processing and facilitating code organization. The final output is a PPL program file with a minimal footprint, optimized to conserve the limited storage space available on the HP Prime.

**Variable & Constant**

Using longer, more meaningful variable and function names can enhance code readability and ease maintenance. However, when targeting the HP Prime's PPL (Programming Language) with UTF16-LE text-based files, employing lengthy names can lead to larger .hpprgm files, which poses a downside. This is where substitution proves beneficial in P+.

> [!NOTE]
Please look at using **PPL Minifier** to handle long names in some situations.
>
```
local a:indexA;
const b:indexB:=1;
indexA += indexB; // Using subtitution.
a := a+b; // PPL results in a smaller .hpprgm file.
```
> [!WARNING]
Deprecated: The use of **:=** in declaring variables will be dropped in **v2.0** to simplifie the language for the use of **PPL Minifier** to be able to reduce long named variables being declared.



**for...next**
```
local I:index
for index:=0; index<10; index++ do
    // statement/s
next;
```
> [!NOTE]
While **end** can be used in place of **next**, by using **next** helps in idetifing it's the end of a for loop.

**do...loop**
```
do
    // statement/s
loop;
```


**switch**
```
local m:menuSelection;
switch menuSelection
    case 0 do
        // statement/s
    end;
end;
```
> [!NOTE]
Advised to use PPL style **case** instead of P+ style.


**if** _condition_ **then** _statement/s_ **else** _statement/s_ **endif**
```
local e:hasError = true;
if hasError==true then
    // statement/s
else
    // statement/s
endif;
```
> [!TIP]
Also in P+ the use of <=, <> and  >= for ≤, ≠ and ≥ is also optional as the pre-processor will automatically covert any `<=`, `<>` or `>=` to `≤`, `≠` and `≥` PPL style.

> [!NOTE]
While **end** can be used in place of **endif**, by using **endif** helps in idetifing it's the end of a if statement.

**guard** _condition_ **else** _statement/s_ **end**
```
guard key != KeyCode.Esc else
    return;
end;
```


**while...wend**
```
local r:isRunning = true
while isRunning == true do
    // statement/s
    isRunning = false;
wend;
```


**try** _condition_ **catch** _statement/s_ [**else** _statement/s_] **end**
```
try
    // statement/s
catch
    // statement/s
else
    // statement/s
end;
```


**(** _condition_ **?** _true_ **:** _false_**)**
```
local a:myValue = 0;
a = (X>Y ? 1 : 0);
```


**def eval:... name(...);**
```
local auto:alpha;
/// It is necessary to evaluate, as we are referencing a defined alias 'alpha' in the definition.
def eval:alpha := a setAlpha(a);
setAlpha(50.0);
```


**#pragma**
```
// Turn off C style bitwise operators ! ^ can now be used as math operations.
#pragma ( bitwise 0 )
```


**Pre-Calc**
```
#define SCREEN_WIDTH 320
local a = #[SCREEN_WIDTH / #[8 - 6]];
// PPL LOCAL a:=160;
local b = #[SCREEN_WIDTH / 4];
b = #[SCREEN_WIDTH / 2]:2; // Pre-Calc #[]:scale
// PPL b:=160.00;
```


> [!IMPORTANT]
In P+ **=** is treated as := were in PPL **=** is treated as ==

>[!NOTE]
The P+ proprietary programming language is susceptible to change, while also maintaining full compatibility with previous versions.
