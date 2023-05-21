#pragma once
/*
* Useful Macros
* prefixed with 'Z' to minimize interference with code compleation
* ZZ are internal helper macros used by other macros
* prefixed with ZZ to be sorted to the absolute bottom of auto compleation
*/

#define ZZ_CAT_(x,y) x ## y  
#define ZC_CAT(x,y) ZZ_CAT_(x,y)  
#define ZZ_STR_(x) #x  
#define ZC_STR(x) ZZ_STR_(x)  
#define ZC_WSTR(x) ZC_CAT(L,ZC_STR(x))
