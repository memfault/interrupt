%module bindings

%include "stdint.i"

%inline %{
extern int32_t multiply(int32_t x, int32_t y);
%}
