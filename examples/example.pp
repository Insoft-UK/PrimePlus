#pragma mode( separator(.,,) integer(h64) )
#include <pplang>

auto:displayCopyright()
begin
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
end;

EXPORT START()
BEGIN
  displayCopyright();
  WAIT;
  var a:alpha = 0;
  alpha += 10;
  RETURN a;
END;
