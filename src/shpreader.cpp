#include "shpreader.h"
#include "shapefil.h"
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LinearRing.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/PointLocation.h>
#include <climits>
#include <fstream>
#include <sstream>

#include <QDebug>

#ifdef HAVE_GDAL
#include "ogr_spatialref.h"
#endif

class bpp::ShpReader::ShpHandles {
public:
    ShpHandles():
        dbf(nullptr),
        shp(nullptr),
        numRecords(0),
        currentRecord(-1),
        isUtf8(true){
    }

    ~ShpHandles(){
        close();
    }

    void close() {
        currentRecord = -1;

        if(dbf) {
            DBFClose(dbf);
            dbf = nullptr;
        }

        if(shp) {
            SHPClose(shp);
            shp = nullptr;
        }
    }

    DBFHandle dbf;
    SHPHandle shp;
    int shpCount;
    int dbfCount;
    int shpType;
    int numRecords;
    int currentRecord;
    bool isUtf8;
};

bpp::ShpReader::ShpReader():
    shph(new ShpHandles()),
    geomType(gUnknown),
    lastPoint(nullptr),
    lastMultiPoint(nullptr),
    lastLineString(nullptr),
    lastMultiLineString(nullptr),
    lastMultiPolygon(nullptr)
{
    geos::geom::PrecisionModel *pm = new geos::geom::PrecisionModel( geos::geom::PrecisionModel::Type::FLOATING );
    geomFactory = geos::geom::GeometryFactory::create(pm, -1);
    delete pm;
}

bpp::ShpReader::~ShpReader() {
    geomFactory->destroy();
    geomFactory.release();

    delete shph;

    if(lastPoint)
    {
        geomFactory->destroyGeometry(lastPoint);
        lastPoint = nullptr;
    }
    if(lastMultiPoint)
    {
        geomFactory->destroyGeometry(lastMultiPoint);
        lastMultiPoint = nullptr;
    }
    if(lastLineString)
    {
        geomFactory->destroyGeometry(lastLineString);
        lastLineString = nullptr;
    }
    if(lastMultiLineString)
    {
        geomFactory->destroyGeometry(lastMultiLineString);
        lastMultiLineString = nullptr;
    }
    /*
    if(lastMultiPolygon)
    {
        geomFactory->destroyGeometry(lastMultiPolygon);
        lastMultiPolygon = nullptr;
    }
    */
}

void bpp::ShpReader::setFile(const std::string &p_shpPath)
{
    shpPath = p_shpPath;
}

std::string bpp::ShpReader::proj4FromPrj(const std::string &p_prjPath)
{
    std::string pj4Str;

    #ifdef HAVE_GDAL
    std::ifstream file(p_prjPath);
    std::string str;
    std::string file_contents;
    while (std::getline(file, str))
    {
        if(str.size() > 3 && str[0] == '\xEF' && str[1] == '\xBB' && str[2] == '\xBF'){
            //BOM MARKER
            str.erase(0,3);
        }
        file_contents += str;
        file_contents.push_back('\n');
    }

    if(!file_contents.empty()) {
        OGRSpatialReference oSRS;
        OGRErr err = oSRS.importFromWkt(file_contents.c_str());
        if(err == OGRERR_NONE){
            char *pszPj4 = nullptr;
            err = oSRS.exportToProj4(&pszPj4);
            if(err == OGRERR_NONE){
                pj4Str = pszPj4;
            }
            CPLFree(pszPj4);
        }
    }
    #endif

    return pj4Str;
}

void MySHPFixFilesize(SHPHandle hSHP) //BIGNO
{
	SAOffset nFileSize;
	hSHP->sHooks.FSeek( hSHP->fpSHP, 0, 2 );
	nFileSize = hSHP->sHooks.FTell(hSHP->fpSHP);
	if( nFileSize >= UINT_MAX )
		hSHP->nFileSize = UINT_MAX;
	else
		hSHP->nFileSize = (unsigned int)nFileSize;
}

bool bpp::ShpReader::open(bool openShp, bool openDbf, std::string &errorMessage)
{
    fieldsNameMap.clear();
    fields.clear();

    shph->close();
    if(openShp)
    {
        shph->shp = SHPOpen( shpPath.c_str(), "rb");
        if(!shph->shp)
        {
            errorMessage = "Cannot open .SHP file";
        }
        else
        {
            SHPGetInfo(shph->shp, &shph->shpCount, &shph->shpType, padMin, padMax);
            shph->numRecords = shph->shpCount;
            geomType = gUnknown;
            switch(shph->shpType)
            {
                case SHPT_POINT:
                case SHPT_POINTZ:
                case SHPT_POINTM:
                    geomType = gPoint;
                break;

                case SHPT_ARC:
                case SHPT_ARCZ:
                case SHPT_ARCM:
                    geomType = gLine;
                break;

                case SHPT_POLYGON:
                case SHPT_POLYGONZ:
                case SHPT_POLYGONM:
                    geomType = gPolygon;
                break;

                case SHPT_MULTIPOINT:
                case SHPT_MULTIPOINTZ:
                case SHPT_MULTIPOINTM:
                    geomType = gMultiPoint;
                break;

                default:
                break;
            }
        }
    }

    if(errorMessage.empty() && openDbf)
    {
        shph->dbf = DBFOpen( shpPath.c_str(), "rb");
        if(!shph->dbf)
        {
            errorMessage = "Cannot open .DBF file";
        }
        else
        {
            const char * cp(DBFGetCodePage(shph->dbf));
            if(cp)
                shph->isUtf8 = (std::string(cp).compare("UTF-8") == 0);
            else
                shph->isUtf8 = false;

            shph->dbfCount = DBFGetRecordCount(shph->dbf);
            shph->numRecords = shph->dbfCount;
        }
    }

    if(errorMessage.empty() && openShp && openDbf)
    {
        if(shph->shpCount != shph->dbfCount) {
            std::ostringstream oss;
            oss << "inconsistent SHP<->DBF. DBF has " << shph->dbfCount << " records, SHP has " << shph->shpCount;
            errorMessage = oss.str();

        }
    }

    if(errorMessage.empty() && openDbf)
    {
        int nFields = DBFGetFieldCount(shph->dbf);
        fields.reserve(size_t(nFields));

        char pszFieldName[100];
        int pnWidth, pnDecimals;
        char nativeType;

        for(int ifi=0; ifi < nFields; ifi++)
        {
            DBFFieldType type = DBFGetFieldInfo( shph->dbf, ifi, pszFieldName, &pnWidth, &pnDecimals );
            nativeType = DBFGetNativeFieldType(shph->dbf, ifi);

            eDataFieldType rType(fText);
            switch(type)
            {
                case FTString:
                break;
                case FTInteger:
                    rType = fInt;
                break;
                case FTDouble:
                    rType = fReal;
                break;
                case FTLogical:
                    rType = fBool;
                break;
                case FTInvalid:
                    rType = fInvalid;
                break;
            }

            addField(pszFieldName, rType, nativeType, pnWidth, pnDecimals);
        }
    }

    if(errorMessage.empty())
		MySHPFixFilesize(shph->shp);

    return errorMessage.empty();
}

int bpp::ShpReader::count()
{
    return shph->numRecords;
}

bool bpp::ShpReader::next()
{
    if(shph->currentRecord < shph->numRecords-1)
    {
        shph->currentRecord++;
        return true;
    }
    return false;
}

void bpp::ShpReader::begin()
{
    shph->currentRecord=-1;
}

int bpp::ShpReader::toInt(int ordinal)
{
    return DBFReadIntegerAttribute(shph->dbf, shph->currentRecord, ordinal);
}

int bpp::ShpReader::toIntDbl(int ordinal)
{
    return int(DBFReadDoubleAttribute(shph->dbf, shph->currentRecord, ordinal));
}

int64_t bpp::ShpReader::toInt64(int ordinal)
{
    return int64_t(toDouble(ordinal));
}

double bpp::ShpReader::toDouble(int ordinal)
{
    return DBFReadDoubleAttribute(shph->dbf, shph->currentRecord, ordinal);
}

const char *bpp::ShpReader::toString(int ordinal)
{
    return DBFReadStringAttribute(shph->dbf, shph->currentRecord, ordinal);

}

QString bpp::ShpReader::toQString(int ordinal)
{
    if(shph->isUtf8)
        return DBFReadStringAttribute(shph->dbf, shph->currentRecord, ordinal);
    else {
        const char* l1str = DBFReadStringAttribute(shph->dbf, shph->currentRecord, ordinal);
        return QString(QLatin1String(l1str));
    }
}

bool bpp::ShpReader::isNull(int ordinal)
{
    return DBFIsAttributeNULL(shph->dbf, shph->currentRecord, ordinal) != 0;
}

bpp::eShpGeomType bpp::ShpReader::getGeomType() const
{
    return geomType;
}

const char *bpp::ShpReader::getGeomTypeName(bpp::eShpGeomType vType)
{
    switch(vType)
    {
    case bpp::gPoint:
        return "Point";
        break;
    case bpp::gMultiPoint:
        return "Multipoint";
        break;
    case bpp::gLine:
        return "Line";
        break;
    case bpp::gPolygon :
        return "Poligon";
        break;
    case bpp::gUnknown:
        return "Unknown";
        break;
    default:
        return "Not classified";
        break;
    }
}

const char *bpp::ShpReader::getGeomTypeName() const
{
    return getGeomTypeName(geomType);
}

geos::geom::Point *bpp::ShpReader::readPoint()
{
    if(lastPoint)
    {
        geomFactory->destroyGeometry(lastPoint);
        lastPoint = nullptr;
    }

    SHPObject * shObj = SHPReadObject( shph->shp, shph->currentRecord );
    if(shObj && (shObj->nSHPType == SHPT_POINT || shObj->nSHPType == SHPT_POINTZ || shObj->nSHPType == SHPT_POINTM) && shObj->nVertices > 0)
    {
        geos::geom::Coordinate coord(shObj->padfX[0], shObj->padfY[0], shObj->padfZ[0]);
        lastPoint = geomFactory->createPoint(coord).release();



        SHPDestroyObject(shObj);
        return lastPoint;
    }
    else
    {
        SHPDestroyObject(shObj);
        return nullptr;
    }
}

geos::geom::MultiPoint *bpp::ShpReader::readMultiPoint()
{
    if(lastMultiPoint)
    {
        geomFactory->destroyGeometry(lastMultiPoint);
        lastMultiPoint = nullptr;
    }

    SHPObject * shObj = SHPReadObject( shph->shp, shph->currentRecord );
    if(shObj && (shObj->nSHPType == SHPT_POINT || shObj->nSHPType == SHPT_POINTZ || shObj->nSHPType == SHPT_POINTM ||
        shObj->nSHPType == SHPT_MULTIPOINT || shObj->nSHPType == SHPT_MULTIPOINTZ || shObj->nSHPType == SHPT_MULTIPOINTM) &&
        shObj->nVertices > 0)
    {
        std::vector<std::unique_ptr<geos::geom::Geometry>>* vec = new std::vector<std::unique_ptr<geos::geom::Geometry>>();
        std::unique_ptr<geos::geom::Geometry> geo = nullptr;

        vec->reserve(size_t(shObj->nVertices));

        geos::geom::Coordinate coord(0,0,0);
        for(int iVtx = 0; iVtx < shObj->nVertices; iVtx++)
        {
            coord.x = shObj->padfX[iVtx];
            coord.y = shObj->padfY[iVtx];
            coord.z = shObj->padfZ[iVtx];

            geo = geomFactory->createPoint(coord);
            vec->push_back(std::move(geo));
        }
        lastMultiPoint = (geomFactory->createMultiPoint(std::move(*vec))).release();

        SHPDestroyObject(shObj);
        return lastMultiPoint;
    }
    else
    {
        SHPDestroyObject(shObj);
        return nullptr;
    }
}

geos::geom::LineString *bpp::ShpReader::readLineString()
{
    if(lastLineString)
    {
        geomFactory->destroyGeometry(lastLineString);
        lastLineString = nullptr;
    }

    SHPObject * shObj = SHPReadObject( shph->shp, shph->currentRecord );
    if(shObj && (shObj->nSHPType == SHPT_ARC || shObj->nSHPType == SHPT_ARCZ || shObj->nSHPType == SHPT_ARCM) &&
        shObj->nVertices > 1 && shObj->nParts <= 1)
    {
        std::unique_ptr<geos::geom::CoordinateSequence> coords(new geos::geom::CoordinateSequence(size_t(shObj->nVertices)));

        geos::geom::Coordinate coord(0,0,0);
        for(int iVtx = 0; iVtx < shObj->nVertices; iVtx++)
        {
            coord.x = shObj->padfX[iVtx];
            coord.y = shObj->padfY[iVtx];
            coord.z = shObj->padfZ[iVtx];

            coords->setAt(coord, size_t(iVtx));
        }
        lastLineString = geomFactory->createLineString(std::move(coords)).release();

        SHPDestroyObject(shObj);
        return lastLineString;
    }
    else
    {
        SHPDestroyObject(shObj);
        return nullptr;
    }
}

geos::geom::MultiLineString *bpp::ShpReader::readMultiLineString()
{
    if(lastMultiLineString)
    {
        geomFactory->destroyGeometry(lastMultiLineString);
        lastMultiLineString = nullptr;
    }

    SHPObject * shObj = SHPReadObject( shph->shp, shph->currentRecord );
    if(shObj && (shObj->nSHPType == SHPT_ARC || shObj->nSHPType == SHPT_ARCZ || shObj->nSHPType == SHPT_ARCM) &&
        shObj->nVertices > 1)
    {
        geos::geom::Coordinate coord(0,0,0);

        int nParts(shObj->nParts);
        if(shObj->nParts == 0)
            nParts = 1;

        std::vector<std::unique_ptr<geos::geom::Geometry>>* lines = new std::vector<std::unique_ptr<geos::geom::Geometry>>;
        lines->reserve(unsigned(nParts));

        int start,end;
        for(int iPa = 0; iPa < shObj->nParts; iPa++)
        {
            start = shObj->nParts > 1 ? shObj->panPartStart[iPa] : 0;

            if( shObj->nParts <= 1 || iPa == shObj->nParts-1 )
                end = shObj->nVertices;
            else
                end = shObj->panPartStart[ iPa + 1 ];

            int nCoords = end - start;
            std::unique_ptr<geos::geom::CoordinateSequence> coords(new geos::geom::CoordinateSequence(unsigned(nCoords)));
            for(int iVtx = start; iVtx < end; iVtx++)
            {
                coord.x = shObj->padfX[iVtx];
                coord.y = shObj->padfY[iVtx];
                coord.z = shObj->padfZ[iVtx];

                coords->setAt(coord, size_t(iVtx - start));
            }

            std::unique_ptr<geos::geom::LineString> line(geomFactory->createLineString(std::move(coords)));
            lines->push_back(std::move(line));
        }

        lastMultiLineString = geomFactory->createMultiLineString(std::move(*lines)).release();
        SHPDestroyObject(shObj);
        return lastMultiLineString;
    }
    else
    {
        SHPDestroyObject(shObj);
        return nullptr;
    }
}

geos::geom::MultiPolygon *bpp::ShpReader::readMultiPolygon()
{
    /*
    if(lastMultiPolygon)
    {
        geomFactory->destroyGeometry(lastMultiPolygon);
        lastMultiPolygon = nullptr;
    }
    */
    lastMultiPolygon.reset(nullptr);

    SHPObject * shObj = SHPReadObject( shph->shp, shph->currentRecord );
    if(shObj && (shObj->nSHPType == SHPT_POLYGON || shObj->nSHPType == SHPT_POLYGONZ || shObj->nSHPType == SHPT_POLYGONM) &&
        shObj->nVertices > 1)
    {
        geos::geom::Coordinate coord(0,0,0);

        int nParts(shObj->nParts);
        if(shObj->nParts == 0)
            nParts = 1;

        int nOuter(0);
        std::vector<std::unique_ptr<geos::geom::LinearRing>> rings;
        std::vector<bool> ringsCCW; //ccw=inner
        std::map<int, std::set< int > > ringsRelation;
        rings.reserve(nParts);
        ringsCCW.reserve(nParts);

        int start,end;
        for(int iPa = 0; iPa < nParts; iPa++)
        {
            start = nParts > 1 ? shObj->panPartStart[iPa] : 0;

            if( nParts <= 1 || iPa == nParts-1 )
                end = shObj->nVertices;
            else
                end = shObj->panPartStart[ iPa + 1 ];

            int nCoords = end - start;
            std::unique_ptr<geos::geom::CoordinateSequence> temp(new geos::geom::CoordinateSequence(size_t(nCoords), 3));
            for(int iVtx = start; iVtx < end; iVtx++)
            {
                temp->setAt(geos::geom::Coordinate(shObj->padfX[iVtx], shObj->padfY[iVtx], shObj->padfZ[iVtx]), size_t(iVtx - start));
            }

            if(nCoords > 2) {
                bool isCCW = geos::algorithm::Orientation::isCCW(temp.get());
                ringsCCW.push_back( isCCW );
                rings.push_back( geomFactory->createLinearRing(std::move(temp)));
                if(!isCCW) {
                    ringsRelation[ ringsCCW.size() - 1 ].clear();
                    nOuter++;
                }
            }
        }

        for(size_t iHole=0; iHole<ringsCCW.size(); iHole++){
            if(ringsCCW[iHole]){
                const geos::geom::CoordinateSequence* csInn = rings[iHole]->getCoordinatesRO();
                const geos::geom::Coordinate& p1 = csInn->getAt(0);
                const geos::geom::Coordinate& p2 = csInn->getAt(1);
                for(size_t iOuter=0; iOuter<ringsCCW.size(); iOuter++){
                    if(!ringsCCW[iOuter]){
                        const geos::geom::CoordinateSequence* csOut = rings[iOuter]->getCoordinatesRO();
                        bool in = geos::algorithm::PointLocation::isInRing(p1, csOut);
                        if(!in) in = geos::algorithm::PointLocation::isInRing(p2, csOut);

                        if(in){
                            ringsRelation[iOuter].insert( iHole );
                            break;
                        }
                    }
                }
            }
        }

        std::vector<std::unique_ptr<geos::geom::Geometry>>* polygons = new std::vector<std::unique_ptr<geos::geom::Geometry>>();
        polygons->reserve(nOuter); //now I handle only one polygon with holes (first ring is exteriour, others are interiours)

        for(auto it=ringsRelation.begin(); it!=ringsRelation.end(); it++){
            std::unique_ptr<geos::geom::LinearRing> externalPoly = std::move(rings[ it->first ]);
            const std::set< int >& setHoles = it->second;

            if(!setHoles.empty()) {
                std::vector<std::unique_ptr<geos::geom::LinearRing>> *holes = new std::vector<std::unique_ptr<geos::geom::LinearRing>>();
                for(auto iHole=setHoles.begin(); iHole!=setHoles.end(); iHole++){
                    holes->push_back(std::move(rings[*iHole]));
                }
                std::unique_ptr<geos::geom::Polygon> thePoly (geomFactory->createPolygon(std::move(externalPoly), std::move(*holes)));
                polygons->push_back(std::move(thePoly));
            }
            else{
                std::unique_ptr<geos::geom::Polygon> thePoly (geomFactory->createPolygon(std::move(externalPoly)));
                polygons->push_back(std::move(thePoly));
            }
        }

        lastMultiPolygon.reset( geomFactory->createMultiPolygon(std::move(*polygons)).release());
        SHPDestroyObject(shObj);
        return lastMultiPolygon.get();
    }
    else
    {
        SHPDestroyObject(shObj);
        return nullptr;
    }
}

const double bpp::ShpReader::getMinX(){
    return padMin[0];
}
const double bpp::ShpReader::getMinY(){
    return padMin[1];
}
const double bpp::ShpReader::getMaxX(){
    return padMax[0];
}
const double bpp::ShpReader::getMaxY(){
    return padMax[1];
}

int bpp::ShpReader::getFieldCount()
{
    return int(fields.size());
}

const bpp::DataField &bpp::ShpReader::getField(int ordinal) const
{
    return fields[size_t(ordinal)];
}

const bpp::DataField &bpp::ShpReader::getField(const std::string &fieldName) const
{
    std::map<std::string, int>::const_iterator it = fieldsNameMap.find(fieldName);
    if(it != fieldsNameMap.end())
        return fields[size_t(it->second)];
    return emptyField;
}

bool bpp::ShpReader::existsField(const std::string &fieldName) const
{
    std::map<std::string, int>::const_iterator it = fieldsNameMap.find(fieldName);
    if(it != fieldsNameMap.end())
        return true;
    return false;
}

std::vector<int> bpp::ShpReader::getNullFields(bool emptyStringsAsNull, const std::vector<int> ordinals)
{
    std::vector<int> nullFields;

    if(ordinals.empty())
        return nullFields;

    std::vector<int> ordinalsCopy = ordinals;

    //remove from check fields that does not exists
    ordinalsCopy.erase(
        std::remove_if(ordinalsCopy.begin(), ordinalsCopy.end(), [](const int& x){
            return x == -1;
        }
    ), ordinalsCopy.end());

    //start the check
    begin();
    while(next())
    {
        bool somethingChanged(false);
        for(auto curOrdinal: ordinalsCopy){
            if(isNull(curOrdinal)) {
                somethingChanged = true;
                nullFields.push_back(curOrdinal);
            }
            else {
                if(emptyStringsAsNull) {
                    const char * strVal = toString(curOrdinal);
                    if(strVal == nullptr || strVal[0] == '\0') {
                        somethingChanged = true;
                        nullFields.push_back(curOrdinal);
                    }
                }
            }
        }

        //remove field from fields to check
        if(somethingChanged) {
            for(auto nullOrdinal: nullFields){
                ordinalsCopy.erase(
                    std::remove_if(ordinalsCopy.begin(), ordinalsCopy.end(), [nullOrdinal](const int& x){
                        return x == nullOrdinal;
                    }
                ), ordinalsCopy.end());
            }
        }
    }

    return nullFields;
}

void bpp::ShpReader::addField(const char *pszFieldName, eDataFieldType type, char pNativeType, int pnWidth, int pnDecimals)
{
    auto makeLower = [](std::string& data) {
        std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    };

    fields.push_back(DataField());

    DataField& thef = fields[ fields.size()-1 ];
    thef.name = pszFieldName;
    makeLower(thef.name);
    thef.type = type;
    thef.nativeType = pNativeType;
    thef.width = pnWidth;
    thef.decimals = pnDecimals;
    thef.fid = int(fields.size()-1);

    fieldsNameMap[thef.name] = thef.fid;
}
