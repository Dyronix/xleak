// A program that includes the header without linking xleak.
#include "xleak/xleak.h"

//-------------------------------------------------------------------------
int main()
{
    return xleak::is_active() ? 1 : 0;
}
