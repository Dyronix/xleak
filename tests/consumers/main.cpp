// A small program outside xleak, as another project would use it.
#include "xleak/xleak.h"

//-------------------------------------------------------------------------
int main()
{
    return xleak::is_active() ? 0 : 1;
}
