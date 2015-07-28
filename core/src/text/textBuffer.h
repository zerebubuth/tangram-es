#pragma once

#include "gl.h"
#include "labels/labelMesh.h"
#include "text/fontContext.h" // for FontID
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"

#include <memory>

namespace Tangram {

class FontContext;

/*
 * This class represents a text buffer, each text buffer has several text ids
 */
class TextBuffer : public LabelMesh {

public:

    TextBuffer(std::shared_ptr<VertexLayout> _vertexLayout);
    ~TextBuffer();

    /* creates a text buffer and bind it */
    void init(FontID _fontID, float _size, float _blurSpread);

    /* ask the font rasterizer to rasterize a specific text.
     * Returns number of glyphs > 0 on success.
     * @_size is set to the text extents
     * @_bufferOffset is set to the byteOffset of the first glyph-vertex */
    int addLabel(const std::string& _text, Label::Transform _transform, Label::Type _type);

    /* get the vertices from the font context and add them as vbo mesh data */
    void addBufferVerticesToMesh();

private:

    FontID m_fontID;
    float m_fontSize;
    float m_fontBlurSpread;

    bool m_dirtyTransform;
    int m_bufferPosition;

    std::vector<Label::Vertex> m_vertices;
};

}
