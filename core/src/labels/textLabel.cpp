#include "labels/textLabel.h"

namespace Tangram {

using namespace LabelProperty;

TextLabel::TextLabel(Label::Transform _transform, Type _type, glm::vec2 _dim, TextBuffer& _mesh, Range _vertexRange,
    Label::Options _options, FontContext::FontMetrics _metrics, int _nLines, Anchor _anchor) :
    Label(_transform, _dim, _type, static_cast<LabelMesh&>(_mesh), _vertexRange, _options),
    m_metrics(_metrics), m_nLines(_nLines)
{
    if (m_type == Type::point) {
        glm::vec2 halfDim = m_dim * 0.5f;
        switch(_anchor) {
            case Anchor::left: m_anchor.x -= halfDim.x; break;
            case Anchor::right: m_anchor.x += halfDim.x; break;
            case Anchor::top: m_anchor.y -= halfDim.y; break;
            case Anchor::bottom: m_anchor.y += halfDim.y; break;
            case Anchor::bottom_left: m_anchor += glm::vec2(-halfDim.x, halfDim.y); break;
            case Anchor::bottom_right: m_anchor += halfDim; break;
            case Anchor::top_left: m_anchor -= halfDim; break;
            case Anchor::top_right: m_anchor += glm::vec2(halfDim.x, -halfDim.y); break;
            case Anchor::center: break;
        }
    }
}

void TextLabel::updateBBoxes(float _zoomFract) {
    glm::vec2 t(1.0, 0.0);
    glm::vec2 tperp(0.0, 1.0);
    glm::vec2 obbCenter;

    if (m_type == Type::line) {
        t = glm::vec2(cos(m_transform.state.rotation), sin(m_transform.state.rotation));
        tperp = glm::vec2(-t.y, t.x);
    }

    obbCenter = m_transform.state.screenPos;
    // move forward on line by half the text length
    obbCenter += t * m_dim.x * 0.5f;
    // move down on the perpendicular to estimated font baseline
    obbCenter -= tperp * m_dim.y * 0.5f;
    // ajdust with baseline
    obbCenter += tperp * m_metrics.lineHeight * (float) m_nLines * 0.5f;

    m_obb = OBB(obbCenter.x, obbCenter.y, m_transform.state.rotation, m_dim.x, m_dim.y);
    m_aabb = m_obb.getExtent();
}

void TextLabel::align(glm::vec2& _screenPosition, const glm::vec2& _ap1, const glm::vec2& _ap2) {

    switch (m_type) {
        case Type::debug:
        case Type::point:
            // modify position set by updateScreenTransform()
            _screenPosition.x -= m_dim.x * 0.5f;
            _screenPosition += m_anchor;
            break;
        case Type::line: {
            // anchor at line center
            _screenPosition = (_ap1 + _ap2) * 0.5f;

            // move back by half the length (so that text will be drawn centered)
            glm::vec2 direction = glm::normalize(_ap1 - _ap2);
            _screenPosition += direction * m_dim.x * 0.5f;
            break;
        }
    }
}

}
