#include "haya/error.h"

const char *_HY_ERR_STRINGS[] = {
#define X(name) #name,
    _HY_ERR_LIST
#undef X
};

const char *hyErrToStr(HyErr err)
{
    if (HY_ERR_NONE <= err && err < _HY_ERR_END)
        return _HY_ERR_STRINGS[err];
    return "HY_ERR_UNKNOWN";
}