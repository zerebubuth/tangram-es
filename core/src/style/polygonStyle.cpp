#include "polygonStyle.h"

#include "tangram.h"
#include "util/builders.h"
#include "gl/shaderProgram.h"
#include "tile/tile.h"
#include "geom.h"

#include <cmath>

namespace Tangram {

#include <chrono>

PolygonStyle::PolygonStyle(std::string _name, GLenum _drawMode) : Style(_name, _drawMode) {
}

void PolygonStyle::constructVertexLayout() {

    // TODO: Ideally this would be in the same location as the struct that it basically describes
    m_vertexLayout = std::shared_ptr<VertexLayout>(new VertexLayout({
        {"a_position", 3, GL_FLOAT, false, 0},
        {"a_normal", 3, GL_FLOAT, false, 0},
        {"a_texcoord", 2, GL_FLOAT, false, 0},
        {"a_color", 4, GL_UNSIGNED_BYTE, true, 0},
        {"a_layer", 1, GL_FLOAT, false, 0}
    }));

}

void PolygonStyle::constructShaderProgram() {

    std::string vertShaderSrcStr = stringFromResource("polygon.vs");
    std::string fragShaderSrcStr = stringFromResource("polygon.fs");

    m_shaderProgram->setSourceStrings(fragShaderSrcStr, vertShaderSrcStr);
}

PolygonStyle::Parameters PolygonStyle::parseRule(const DrawRule& _rule) const {
    Parameters p;
    _rule.getColor(StyleParamKey::color, p.color);
    _rule.getValue(StyleParamKey::order, p.order);
    _rule.getValue(StyleParamKey::extrude, p.extrude);

    return p;
}

void PolygonStyle::buildLine(const Line& _line, const DrawRule& _rule, const Properties& _props, VboMesh& _mesh, Tile& _tile) const {
    std::vector<PolygonVertex> vertices;

    Parameters params = parseRule(_rule);

    GLuint abgr = params.color;
    GLfloat layer = params.order;

    PolyLineBuilder builder = {
        [&](const glm::vec3& coord, const glm::vec2& normal, const glm::vec2& uv) {
            float halfWidth =  0.2f;

            glm::vec3 point(coord.x + normal.x * halfWidth, coord.y + normal.y * halfWidth, coord.z);
            vertices.push_back({ point, glm::vec3(0.0f, 0.0f, 1.0f), uv, abgr, layer });
        },
        [&](size_t sizeHint){ vertices.reserve(sizeHint); }
    };

    Builders::buildPolyLine(_line, builder);

    auto& mesh = static_cast<PolygonStyle::Mesh&>(_mesh);
    mesh.addVertices(std::move(vertices), std::move(builder.indices));
}

#define COMPARE 1

#ifdef COMPARE
static std::mutex s_staticstics;
static double s_total1 = 0;
static double s_total2 = 0;
static double s_maxDuration = 0;
static uint64_t s_sumVertices = 0;
#endif

void PolygonStyle::buildPolygon(const Polygon& _polygon, const DrawRule& _rule, const Properties& _props, VboMesh& _mesh, Tile& _tile) const {

    std::vector<PolygonVertex> vertices;
    std::vector<PolygonVertex> vertices2;

    Parameters params = parseRule(_rule);

    GLuint abgr = params.color;
    GLfloat layer = params.order;
    auto& extrude = params.extrude;

    if (Tangram::getDebugFlag(Tangram::DebugFlags::proxy_colors)) {
        abgr = abgr << (_tile.getID().z % 6);
    }

    const static std::string key_height("height");
    const static std::string key_min_height("min_height");

    float height = _props.getNumeric(key_height) * _tile.getInverseScale();
    float minHeight = _props.getNumeric(key_min_height) * _tile.getInverseScale();

    PolygonBuilder builder = {
        [&](const glm::vec3& coord, const glm::vec3& normal, const glm::vec2& uv){
            vertices.push_back({ coord, normal, uv, abgr, layer });
        },
        [&](size_t sizeHint){ vertices.reserve(sizeHint); }
    };

#ifndef COMPARE
    if (extrude.first != 0.0f || extrude.second != 0.0f) {
        float ht = std::isnan(extrude.second) ? ( std::isnan(extrude.first) ? height : extrude.first ) : extrude.second;
        float minHt = std::isnan(extrude.second) ? minHeight : extrude.first;
        Builders::buildPolygonExtrusion(_polygon, minHt, ht, builder);
        Builders::buildPolygon(_polygon, ht, builder);
    } else {
        Builders::buildPolygon(_polygon, 0.0f, builder);
    }

#else
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10; i++) {
        builder.indices.clear();
        builder.numVertices = 0;
        vertices.clear();
        Builders::buildPolygon(_polygon, height, builder);
    }

    auto end1 = std::chrono::high_resolution_clock::now();

    double duration1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1).count();

    PolygonBuilder builder2 = {
        [&](const glm::vec3& coord, const glm::vec3& normal, const glm::vec2& uv){
            vertices2.push_back({ coord, normal, uv, abgr, layer });
        },
        [&](size_t sizeHint){ vertices2.reserve(sizeHint); }
    };

    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10; i++) {
        builder2.indices.clear();
        builder2.numVertices = 0;
        vertices2.clear();
        Builders::buildPolygonTess(_polygon, height, builder2);
    }

    auto end2 = std::chrono::high_resolution_clock::now();

    double duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2).count();

    double allMax = 0;

    {
        std::lock_guard<std::mutex> lock(s_staticstics);
        s_total1 += double(duration1 / 1e9);
        s_total2 += double(duration2 / 1e9);
        s_sumVertices += vertices.size();

        s_maxDuration = std::max(s_maxDuration, duration1);
        s_maxDuration = std::max(s_maxDuration, duration2);
        allMax = s_maxDuration;
    }

    if (minHeight != height) {
        Builders::buildPolygonExtrusion(_polygon, minHeight, height, builder);
    }

    double max = std::max(duration2, duration1);

    if (max > 0) {
        int d = CLAMP(255.0 * ((duration2 - duration1) / max), -255.0, 255.0) * CLAMP((max / 50000), 0.2, 1.0);

        uint32_t c = 0xff000000 | (d > 0 ? (d << 16) : -d);

        for (auto& v : vertices) {
            v.abgr = c;
        }
    }
#endif

    auto& mesh = static_cast<PolygonStyle::Mesh&>(_mesh);
    mesh.addVertices(std::move(vertices), std::move(builder.indices));
}

void PolygonStyle::onEndBuildTile(Tile& _tile) const {
#ifdef COMPARE
       std::lock_guard<std::mutex> lock(s_staticstics);

       auto vps1 = double(s_sumVertices) / s_total1;
       auto vps2 = double(s_sumVertices) / s_total2;

       logMsg("%f \t %f \t %f - %f\n", vps1, vps2, (s_total1 / s_total2), s_maxDuration);
#endif
}

}
