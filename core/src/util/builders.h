#pragma once

#include "data/tileData.h"

#include <functional>
#include <vector>

namespace Tangram {

enum class CapTypes {
    butt = 0, // No points added to end of line
    square = 2, // Two points added to make a square extension
    round = 6 // Six points added in a fan to make a round cap
};

CapTypes CapTypeFromString(const std::string& str);

enum class JoinTypes {
    miter = 0, // No points added at line join
    bevel = 1, // One point added to flatten the corner of a join
    round = 5 // Five points added in a fan to make a round outer join
};

JoinTypes JoinTypeFromString(const std::string& str);


/* PolygonBuilder context,
 * see Builders::buildPolygon() and Builders::buildPolygonExtrusion()
 */
struct PolygonBuilder {
    struct Context {
        // indices for drawing the polyline as
        // triangles are added to this  vector
        std::vector<uint16_t> indices;
        size_t numVertices = 0;
        bool useTexCoords;

        Context(bool _useTexCoords = true)
            :  useTexCoords(_useTexCoords){}
    };

    /* Callback function for PolygonBuilder:
     *
     * @coord  tesselated output coordinate
     * @normal triangle plane normal
     * @uv     texture coordinate of the output coordinate
     */
    virtual void addVertex(const glm::vec3& coord,
                           const glm::vec3& normal,
                           const glm::vec2& uv) = 0;

    virtual void sizeHint(size_t reserve) = 0;

    /* Build a tesselated polygon
     * @_polygon input coordinates describing the polygon
     * @_ctx output vectors, see <PolygonBuilder>
     */
    void tesselate(const Polygon& _polygon, float _height, Context& _ctx);

    /* Build extruded 'walls' from a polygon
     * @_polygon input coordinates describing the polygon
     * @_minHeight the extrusion will extend from this z coordinate to the z of
     * the polygon points
     * @_ctx output vectors, see <PolygonBuilder>
     */
    void extrude(const Polygon& _polygon, float _minHeight,
                 float _maxHeight, Context& _ctx);
};


// using PolyLineVertexFn = std::function<void(const glm::vec3& coord,
//                                             const glm::vec2& enormal,
//                                             const glm::vec2& uv)>;

/* PolyLineBuilder context,
 * see Builders::buildPolyLine()
 */
struct PolyLineBuilder {
    struct Context {
        // indices for drawing the polyline as
        // triangles are added to this  vector
        std::vector<uint16_t> indices;
        size_t numVertices = 0;
        CapTypes cap;
        JoinTypes join;

    Context(CapTypes _cap = CapTypes::butt,
            JoinTypes _join = JoinTypes::bevel)
        : cap(_cap), join(_join) {}
    };

    /* Callback function for PolyLineBuilder:
     *
     * @coord   tesselated output coordinate
     * @enormal extrusion vector of the output coordinate
     * @uv      texture coordinate of the output coordinate
     */
    virtual void addVertex(glm::vec3 coord, glm::vec2 enormal, glm::vec2 uv) = 0;

    virtual void sizeHint(size_t reserve) = 0;

    /* Build a tesselated polygon line of fixed width from line coordinates
     * @_line input coordinates describing the line
     * @_options parameters for polyline construction
     * @_ctx output vectors, see <PolyLineBuilder>
     */
    void build(const Line& _line, Context& _ctx);

    void addFan(const glm::vec3& _pC,
                const glm::vec2& _nA, const glm::vec2& _nB, const glm::vec2& _nC,
                const glm::vec2& _uA, const glm::vec2& _uB, const glm::vec2& _uC,
                int _numTriangles, Context& _ctx);

    void addCap(const glm::vec3& _coord, const glm::vec2& _normal, int _numCorners,
                bool _isBeginning, Context& _ctx);

    // /* Build a tesselated outline that follows the given line while skipping
    //  * tile boundaries.
    //  */
    // void buildOutline(const Line& _line, Context& _ctx);
};


/* Callback function for SpriteBuilder
 * @coord tesselated coordinates of the sprite quad in screen space
 * @screenPos the screen position
 * @uv texture coordinate of the ouptput coordinate
 */
using SpriteBuilderFn = std::function<void(const glm::vec2& coord,
                                           const glm::vec2& screenPos,
                                           const glm::vec2& uv)> ;

/* SpriteBuidler context
 */
struct SpriteBuilder {
    std::vector<uint16_t> indices;
    SpriteBuilderFn addVertex;
    size_t numVerts = 0;

    SpriteBuilder(SpriteBuilderFn _addVertex) : addVertex(_addVertex) {}
};

class Builders {

public:

    /* Build a tesselated quad centered on _screenOrigin
     * @_screenOrigin the sprite origin in screen space
     * @_size the size of the sprite in pixels
     * @_uvBL the bottom left UV coordinate of the quad
     * @_uvTR the top right UV coordinate of the quad
     * @_ctx output vectors, see <SpriteBuilder>
     */
    static void buildQuadAtPoint(const glm::vec2& _screenOrigin, const glm::vec2& _size,
                                 const glm::vec2& _uvBL, const glm::vec2& _uvTR, SpriteBuilder& _ctx);

};

}
