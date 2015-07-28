#include "textBuffer.h"
#include "labels/textLabel.h"

#include "gl/texture.h"
#include "gl/vboMesh.h"

namespace Tangram {

TextBuffer::TextBuffer(std::shared_ptr<VertexLayout> _vertexLayout)
    : LabelMesh(_vertexLayout, GL_TRIANGLES) {

    m_dirtyTransform = false;
    m_bufferPosition = 0;
}

void TextBuffer::init(uint32_t _fontID, float _size, float _blurSpread) {
    m_fontID = _fontID;
    m_fontSize = _size;
    m_fontBlurSpread = _blurSpread;
}

TextBuffer::~TextBuffer() {
}

int TextBuffer::addLabel(const std::string& _text, Label::Transform _transform, Label::Type _type) {

    auto fontContext = FontContext::GetInstance();

    fontContext->lock();

    auto ctx = fontContext->getFontContext();

    fonsSetSize(ctx, m_fontSize);
    fonsSetFont(ctx, m_fontID);

    if (m_fontBlurSpread > 0){
        fonsSetBlur(ctx, m_fontBlurSpread);
        fonsSetBlurType(ctx, FONS_EFFECT_DISTANCE_FIELD);
    } else {
        fonsSetBlurType(ctx, FONS_EFFECT_NONE);
    }

    if (!fontContext->rasterize(_text)) {
        fontContext->unlock();
        return 0;
    }

    auto& quads = fontContext->getQuads();

    size_t bufferPosition = m_vertexLayout->getStride() * m_vertices.size();

    size_t numGlyphs = quads.size();

    m_vertices.reserve(m_vertices.size() + numGlyphs * 6);

    float inf = std::numeric_limits<float>::infinity();
    float x0 = inf, x1 = -inf, y0 = inf, y1 = -inf;

    for (auto& q : quads) {
        x0 = std::min(x0, q.x0);
        x0 = std::min(x0, q.x1);

        x1 = std::max(x1, q.x0);
        x1 = std::max(x1, q.x1);

        y0 = std::min(y0, q.y0);
        y0 = std::min(y0, q.y1);

        y1 = std::max(y1, q.y0);
        y1 = std::max(y1, q.y1);

        m_vertices.push_back({{q.x0, q.y0}, {q.s0, q.t0}});
        m_vertices.push_back({{q.x1, q.y1}, {q.s1, q.t1}});
        m_vertices.push_back({{q.x1, q.y0}, {q.s1, q.t0}});

        m_vertices.push_back({{q.x0, q.y0}, {q.s0, q.t0}});
        m_vertices.push_back({{q.x0, q.y1}, {q.s0, q.t1}});
        m_vertices.push_back({{q.x1, q.y1}, {q.s1, q.t1}});
    }
    fontContext->unlock();

    std::unique_ptr<TextLabel> label(new TextLabel(*this, _transform, _text, _type));
    label->m_bufferOffset = bufferPosition;
    label->m_dim = glm::vec2(x1 - x0, y1 - y0);
    label->m_numGlyphs = numGlyphs;

    LabelMesh::addLabel(std::move(label));

    return numGlyphs;
}

void TextBuffer::addBufferVerticesToMesh() {

    addVertices(std::move(m_vertices), {});
}

}
