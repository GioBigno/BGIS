#ifndef SHPFORMAT_H
#define SHPFORMAT_H

#include <string>

namespace bpp
{

enum eDataFieldType {
    fInvalid,
    fInt,
    fReal,
    fText,
    fBool
};

enum eShpGeomType {
    gUnknown,
    gPoint,
    gMultiPoint,
    gLine,
    gPolygon
};

class DataField {
public:
    DataField();

    std::string name;
    int width;
    int decimals;
    eDataFieldType type;
    char nativeType;
    int fid;    //field ordinal in fields vector

    static const char* typeName(eDataFieldType vType);
    const char* typeName() const;
};

}
#endif // SHPFORMAT_H
