#pragma once

#include "labels/label.h"
#include "text/textBuffer.h"
#include "text/fontContext.h"
#include "style/labelProperty.h"

namespace Tangram {

class TextLabel : public Label {

public:
    TextLabel(Label::Transform _transform, Type _type, glm::vec2 _dim, TextBuffer& _mesh, Range _vertexRange,
              Label::Options _options, FontContext::FontMetrics _metrics, int _nLines, LabelProperty::Anchor _anchor,
              size_t _hash);

    void updateBBoxes(float _zoomFract) override;

    size_t getHash() { return m_hash; };

protected:

    void align(glm::vec2& _screenPosition, const glm::vec2& _ap1, const glm::vec2& _ap2) override;
    FontContext::FontMetrics m_metrics;
    int m_nLines;

private:

    glm::vec2 m_anchor;
    size_t m_hash;

};

}
