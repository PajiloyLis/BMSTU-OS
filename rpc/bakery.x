const ADD = 0;
const SUB = 1;
const MUL = 2;
const DIV = 3;

struct BAKERY
{
    int num;
    int op;
    float arg1;
    float arg2;
    float res;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        int GET_NUMBER() = 1;
        struct BAKERY SERVICE(struct BAKERY) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001;