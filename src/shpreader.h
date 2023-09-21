#ifndef SHPREADER_H
#define SHPREADER_H

#include "shpformat.h"
#include <map>
//OKKIO #define _MATH_DEFINES_DEFINED
//#include "geos.h"
//#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <QString>

namespace bpp {

class ShpReader
{
public:
    ShpReader();
    ~ShpReader();

    void setFile(const std::string& p_shpPath);

    static std::string proj4FromPrj(const std::string& p_prjPath);

    bool open(bool openShp, bool openDbf, std::string& errorMessage);
    int count();
    bool next();
    void begin();

    int toInt(int ordinal);
    int toIntDbl(int ordinal); //reads double as integer
    int64_t toInt64(int ordinal);
    double toDouble(int ordinal);
    const char *toString(int ordinal);
    QString toQString(int ordinal);
    bool isNull(int ordinal);

    eShpGeomType getGeomType() const;
    static const char* getGeomTypeName(eShpGeomType vType);
    const char* getGeomTypeName() const;

    geos::geom::Point *readPoint();
    geos::geom::MultiPoint *readMultiPoint();
    geos::geom::LineString *readLineString();
    geos::geom::MultiLineString *readMultiLineString();
    geos::geom::MultiPolygon *readMultiPolygon();

    const double getMinX();
    const double getMinY();
    const double getMaxX();
    const double getMaxY();

    int getFieldCount();
    const DataField& getField(int ordinal) const;
    const DataField& getField(const std::string& fieldName) const;
    bool existsField(const std::string& fieldName) const;

    std::vector<int> getNullFields(bool emptyStringsAsNull, const std::vector<int> ordinals);
private:
    std::string shpPath;

    geos::geom::GeometryFactory::Ptr geomFactory;
    DataField emptyField;

    std::map<std::string, int> fieldsNameMap;
    std::vector<DataField> fields;
    void addField(const char* pszFieldName, eDataFieldType type, char pNativeType, int pnWidth, int pnDecimals);

    class ShpHandles;
    ShpHandles* shph;

    double padMin[4];
    double padMax[4];

    eShpGeomType geomType;
    geos::geom::Point *lastPoint;
    geos::geom::MultiPoint *lastMultiPoint;
    geos::geom::LineString *lastLineString;
    geos::geom::MultiLineString *lastMultiLineString;
    std::unique_ptr<geos::geom::MultiPolygon> lastMultiPolygon;
};

}
#endif // SHPREADER_H
