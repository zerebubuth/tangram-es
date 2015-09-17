/*
 * Reference used for implementation: http://www.maptiler.org/google-maps-coordinates-tile-bounds-projection/
 */

#include "mapProjection.h"

#include <cmath>

namespace Tangram {

MercatorProjection::MercatorProjection() :
    MapProjection(ProjectionType::mercator) {
}

glm::dvec2 MercatorProjection::LonLatToMeters(const glm::dvec2 _lonLat) const {
    glm::dvec2 meters;
    meters.x = _lonLat.x / 180.0;
    meters.y = log(tan(PI * 0.25 + _lonLat.y * (PI / 360.0))) / PI;
    return meters;
}

glm::dvec2 MercatorProjection::MetersToLonLat(const glm::dvec2 _meters) const {
    glm::dvec2 lonLat;
    lonLat.x = _meters.x * 180.0;
    lonLat.y = (2.0 * atan(exp((_meters.y * PI))) - PI * 0.5) * (180.0 / PI);
    return lonLat;
}

glm::dvec2 MercatorProjection::PixelsToMeters(const glm::dvec2 _pix, const int _zoom) const {
    glm::dvec2 meters;
    double res = CIRCUMFERENCE / (1 << _zoom);
    meters.x = _pix.x * res - HALF_CIRCUMFERENCE;
    meters.y = _pix.y * res - HALF_CIRCUMFERENCE;
    return meters;
}

glm::dvec2 MercatorProjection::MetersToPixel(const glm::dvec2 _meters, const int _zoom) const {
    glm::dvec2 pix;
    double invRes = (1 << _zoom) / CIRCUMFERENCE;
    pix.x = (_meters.x + HALF_CIRCUMFERENCE) * invRes;
    pix.y = (_meters.y + HALF_CIRCUMFERENCE) * invRes;
    return pix;
}

glm::ivec2 MercatorProjection::PixelsToTileXY(const glm::dvec2 _pix) const {
    //returns the tile covering a region of a pixel
    glm::ivec2 tileXY;
    tileXY.x = int(ceil(_pix.x) - 1);
    tileXY.y = int(ceil(_pix.y) - 1);
    return tileXY;
}

glm::ivec2 MercatorProjection::MetersToTileXY(const glm::dvec2 _meters, const int _zoom) const {
    return PixelsToTileXY(MetersToPixel(_meters, _zoom));
}

glm::dvec2 MercatorProjection::PixelsToRaster(const glm::dvec2 _pix, const int _zoom) const {
    double mapSize = 1 << _zoom;
    return glm::dvec2(_pix.x, (mapSize - _pix.y));
}

BoundingBox MercatorProjection::TileBounds(const TileID _tileCoord) const {
    return {
        PixelsToMeters({
                _tileCoord.x,
                _tileCoord.y },
            _tileCoord.z),
        PixelsToMeters({
                (_tileCoord.x + 1),
                (_tileCoord.y + 1) },
            _tileCoord.z)
    };
}

BoundingBox MercatorProjection::TileLonLatBounds(const TileID _tileCoord) const {
    BoundingBox tileBounds(TileBounds(_tileCoord));
    return {
        MetersToLonLat(tileBounds.min),
        MetersToLonLat(tileBounds.max)
    };
}

glm::dvec2 MercatorProjection::TileCenter(const TileID _tileCoord) const {
    return PixelsToMeters(glm::dvec2((_tileCoord.x + 0.5),
                                     (_tileCoord.y + 0.5)),
                          _tileCoord.z);
}

}
