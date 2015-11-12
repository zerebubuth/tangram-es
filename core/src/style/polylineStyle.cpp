#include "polylineStyle.h"

#include "tangram.h"
#include "platform.h"
#include "material.h"
#include "gl/shaderProgram.h"
#include "gl/typedMesh.h"
#include "scene/stops.h"
#include "scene/drawRule.h"
#include "tile/tile.h"
#include "util/mapProjection.h"
#include "glm/vec3.hpp"

namespace Tangram {

struct PolylineVertex {
    glm::vec3 pos;
    glm::vec2 texcoord;
    glm::vec4 extrude;
    GLuint abgr;
    GLfloat layer;
};

using Mesh = TypedMesh<PolylineVertex>;

PolylineStyle::PolylineStyle(std::string _name, Blending _blendMode, GLenum _drawMode)
    : Style(_name, _blendMode, _drawMode) {
}

void PolylineStyle::constructVertexLayout() {

    // TODO: Ideally this would be in the same location as the struct that it basically describes
    m_vertexLayout = std::shared_ptr<VertexLayout>(new VertexLayout({
        {"a_position", 3, GL_FLOAT, false, 0},
        {"a_texcoord", 2, GL_FLOAT, false, 0},
        {"a_extrude", 4, GL_FLOAT, false, 0},
        {"a_color", 4, GL_UNSIGNED_BYTE, true, 0},
        {"a_layer", 1, GL_FLOAT, false, 0}
    }));

}

void PolylineStyle::constructShaderProgram() {

    std::string vertShaderSrcStr = stringFromFile("shaders/polyline.vs", PathType::internal);
    std::string fragShaderSrcStr = stringFromFile("shaders/polyline.fs", PathType::internal);

    m_shaderProgram->setSourceStrings(fragShaderSrcStr, vertShaderSrcStr);
}

VboMesh* PolylineStyle::newMesh() const {
    return new Mesh(m_vertexLayout, m_drawMode);
}

PolylineStyle::Parameters PolylineStyle::parseRule(const DrawRule& _rule) const {
    Parameters p;

    uint32_t cap = 0, join = 0;

    _rule.get(StyleParamKey::extrude, p.extrude);
    _rule.get(StyleParamKey::color, p.color);
    _rule.get(StyleParamKey::cap, cap);
    _rule.get(StyleParamKey::join, join);
    _rule.get(StyleParamKey::order, p.order);

    p.cap = static_cast<CapTypes>(cap);
    p.join = static_cast<JoinTypes>(join);

    p.outlineOrder = p.order; // will offset from fill later

    if (_rule.get(StyleParamKey::outline_color, p.outlineColor) |
        _rule.get(StyleParamKey::outline_order, p.outlineOrder) |
        _rule.contains(StyleParamKey::outline_width) |
        _rule.get(StyleParamKey::outline_cap, cap) |
        _rule.get(StyleParamKey::outline_join, join)) {
        p.outlineOn = true;
        p.outlineCap = static_cast<CapTypes>(cap);
        p.outlineJoin = static_cast<JoinTypes>(join);
    }

    return p;
}

void PolylineStyle::buildPolygon(const Polygon& _poly, const DrawRule& _rule, const Properties& _props,
                                 VboMesh& _mesh, Tile& _tile) const {

    for (const auto& line : _poly) {
        buildLine(line, _rule, _props, _mesh, _tile);
    }
}

double widthMeterToPixel(int _zoom, double _tileSize, double _width) {
    // pixel per meter at z == 0
    double meterRes = _tileSize / (2.0 * MapProjection::HALF_CIRCUMFERENCE);
    // pixel per meter at zoom
    meterRes *= exp2(_zoom);

    return _width * meterRes;
}

bool evalStyleParamWidth(StyleParamKey _key, const DrawRule& _rule, const Tile& _tile,
                         float& width, float& dWdZ){

    int zoom  = _tile.getID().z;
    double tileSize = _tile.getProjection()->TileSize();

    // NB: 0.5 because 'width' will be extruded in both directions
    double tileRes = 0.5 / tileSize;


    auto& styleParam = _rule.findParameter(_key);
    if (styleParam.stops) {

        width = styleParam.stops->evalWidth(zoom);
        width *= tileRes;

        dWdZ = styleParam.stops->evalWidth(zoom + 1);
        dWdZ *= tileRes;
        // NB: Multiply by 2 for the outline to get the expected stroke pixel width.
        if (_key == StyleParamKey::outline_width) {
            width *= 2;
            dWdZ *= 2;
        }

        dWdZ -= width;

        return true;
    }

    if (styleParam.value.is<StyleParam::Width>()) {
        auto& widthParam = styleParam.value.get<StyleParam::Width>();

        width = widthParam.value;

        if (widthParam.isMeter()) {
            width = widthMeterToPixel(zoom, tileSize, width);
            width *= tileRes;
            dWdZ = width * 2;
        } else {
            width *= tileRes;
            dWdZ = width;
        }

        if (_key == StyleParamKey::outline_width) {
            width *= 2;
            dWdZ *= 2;
        }

        dWdZ -= width;

        return true;
    }

    LOGD("Invalid type for Width '%d'\n", styleParam.value.which());
    return false;
}

void PolylineStyle::buildLine(const Line& _line, const DrawRule& _rule, const Properties& _props,
                              VboMesh& _mesh, Tile& _tile) const {

    std::vector<PolylineVertex> vertices;

    Parameters params = parseRule(_rule);
    GLuint abgr = params.color;

    float dWdZ = 0.f;
    float width = 0.f;

    if (!evalStyleParamWidth(StyleParamKey::width, _rule, _tile, width, dWdZ)) {
        return;
    }

    if (width <= 0.0f && dWdZ <= 0.0f ) { return; }

    if (Tangram::getDebugFlag(Tangram::DebugFlags::proxy_colors)) {
        abgr = abgr << (_tile.getID().z % 6);
    }

    float height = 0.0f;
    auto& extrude = params.extrude;

    if (extrude[0] != 0.0f || extrude[1] != 0.0f) {
        const static std::string key_height("height");

        height = _props.getNumeric(key_height) * _tile.getInverseScale();
        if (std::isnan(extrude[1])) {
            if (!std::isnan(extrude[0])) {
                height = extrude[0] * _tile.getInverseScale();
            }
        } else { height = extrude[1] * _tile.getInverseScale(); }
    }

    PolyLineBuilder builder {
        [&](const glm::vec3& coord, const glm::vec2& normal, const glm::vec2& uv) {
            glm::vec4 extrude = { normal.x, normal.y, width, dWdZ };
            vertices.push_back({ {coord.x, coord.y, height}, uv, extrude, abgr, (float)params.order });
        },
        [&](size_t sizeHint){ vertices.reserve(sizeHint); },
        params.cap,
        params.join
    };

    Builders::buildPolyLine(_line, builder);

    if (params.outlineOn) {

        GLuint abgrOutline = params.outlineColor;
        float outlineOrder = std::min(params.outlineOrder, params.order) - .5f;

        float widthOutline = 0.f;
        float dWdZOutline = 0.f;

        if (evalStyleParamWidth(StyleParamKey::outline_width, _rule, _tile, widthOutline, dWdZOutline) &&
            ((widthOutline > 0.0f || dWdZOutline > 0.0f)) ) {

            widthOutline += width;
            dWdZOutline += dWdZ;

            if (params.outlineCap != params.cap || params.outlineJoin != params.join) {
                // need to re-triangulate with different cap and/or join
                builder.cap = params.outlineCap;
                builder.join = params.outlineJoin;
                Builders::buildPolyLine(_line, builder);
            } else {
                // re-use indices from original line
                size_t oldSize = builder.indices.size();
                size_t offset = vertices.size();
                builder.indices.reserve(2 * oldSize);

                for(size_t i = 0; i < oldSize; i++) {
                    builder.indices.push_back(offset + builder.indices[i]);
                }
                for (size_t i = 0; i < offset; i++) {
                    const auto& v = vertices[i];
                    glm::vec4 extrudeOutline = { v.extrude.x, v.extrude.y, widthOutline, dWdZOutline };
                    vertices.push_back({ v.pos, v.texcoord, extrudeOutline, abgrOutline, outlineOrder });
                }
            }
        }
    }

    auto& mesh = static_cast<Mesh&>(_mesh);
    mesh.addVertices(std::move(vertices), std::move(builder.indices));
}

}
