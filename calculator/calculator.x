/*
 * filename: calculator.x
     * function: Define constants, non-standard data types and the calling process in remote calls
 */
 
const ADD = 0;
const SUB = 1;
const MUL = 2;
const DIV = 3;

struct CALCULATOR
{
    int op;
    float arg1;
    float arg2;
    float result;
    int number;
    int return_code;
};

program CALCULATOR_PROG
{
    version CALCULATOR_VER
    {
        int GET_NUM() = 1;
        struct CALCULATOR CALCULATOR_PROC(struct CALCULATOR) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001; /* RPC program number */