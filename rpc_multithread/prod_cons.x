const PROD = 0;
const CONS = 1;

program producer_consumer_prog
{
    version producer_consumer_ver
    {
        char service(int)=1;
    } = 1;
} = 0x20000002;