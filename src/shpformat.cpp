#include "shpformat.h"

bpp::DataField::DataField():
    width(0),decimals(0),type(fInvalid),nativeType(0),fid(-1)
{

}

const char *bpp::DataField::typeName(bpp::eDataFieldType vType)
{
    switch(vType)
    {
    case bpp::fBool:
        return "Bool";
        break;
    case bpp::fInt:
        return "Int";
        break;
    case bpp::fReal:
        return "Real";
        break;
    case bpp::fText :
        return "Text";
        break;
    case bpp::fInvalid:
        return "Invalid";
        break;
    default:
        return "Not classified";
        break;
    }
}

const char *bpp::DataField::typeName() const
{
    return typeName(type);
}
