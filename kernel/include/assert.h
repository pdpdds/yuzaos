#pragma once

extern void __SKY_ASSERT(const char* expr_str, bool expr, const char* file, int line, const char* msg);

#define SKY_ASSERT(Expr, Msg) \
    __SKY_ASSERT(#Expr, Expr, __FILE__, __LINE__, Msg)


#define ASSERT(a) SKY_ASSERT(a, "error")
#define ASSERT_WITH_MESSAGE(a, Msg) SKY_ASSERT(a, Msg)
#define assert(a) SKY_ASSERT(a, "error")