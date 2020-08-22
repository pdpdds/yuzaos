#pragma once

extern void __SKY_ASSERT(const char* expr_str, bool expr, const char* file, int line, const char* msg);

static char assert_message[] = "ASSERT!!";

#define SKY_ASSERT(Expr, Msg) \
    __SKY_ASSERT(#Expr, Expr, __FILE__, __LINE__, Msg)

#define SKY_ASSERT2(Expr) \
    __SKY_ASSERT(#Expr, Expr, __FILE__, __LINE__, assert_message)
