#include "labels/labelMesh.h"
#include "labels/label.h"
#include "gl/renderState.h"
#include "gl/shaderProgram.h"

namespace Tangram {

static GLuint s_quadIndexBuffer = 0;
static int s_quadGeneration = -1;

const size_t maxVertices = 16384;

LabelMesh::LabelMesh(std::shared_ptr<VertexLayout> _vertexLayout, GLenum _drawMode)
    : TypedMesh<Label::Vertex>(_vertexLayout, _drawMode, GL_DYNAMIC_DRAW) {
}

LabelMesh::~LabelMesh() {}

void LabelMesh::addLabel(std::unique_ptr<Label> _label) {
    m_labels.push_back(std::move(_label));
}

void LabelMesh::reset() {
    for (auto& label : m_labels) {
        label->resetState();
    }
}

void LabelMesh::loadQuadIndices() {
    if (s_quadGeneration == s_validGeneration) {
        return;
    }

    s_quadGeneration = s_validGeneration;

    std::vector<GLushort> indices;
    indices.reserve(maxVertices / 4 * 6);

    for (size_t i = 0; i < maxVertices; i += 4) {
        indices.push_back(i + 2);
        indices.push_back(i + 0);
        indices.push_back(i + 1);
        indices.push_back(i + 1);
        indices.push_back(i + 3);
        indices.push_back(i + 2);
    }

    glGenBuffers(1, &s_quadIndexBuffer);
    RenderState::indexBuffer(s_quadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort),
                 reinterpret_cast<GLbyte*>(indices.data()), GL_STATIC_DRAW);
}

void LabelMesh::compileVertexBuffer() {
    size_t sumVertices = 0;
    int stride = m_vertexLayout->getStride();
    m_glVertexData = new GLbyte[stride * m_nVertices];

    for (auto& vertices : m_vertices) {
        size_t nVertices = vertices.size();

        std::memcpy(m_glVertexData + (sumVertices * stride),
                    reinterpret_cast<GLbyte*>(vertices.data()), nVertices * stride);
        sumVertices += nVertices;
    }

    for (size_t offset = 0; offset < sumVertices; offset += maxVertices) {
        size_t nVertices = maxVertices;
        if (offset + maxVertices > sumVertices) {
            nVertices = sumVertices - offset;
        }
        m_vertexOffsets.emplace_back(nVertices / 4 * 6, nVertices);
    }

    // clear vertices...
    std::vector<std::vector<Label::Vertex>>().swap(m_vertices);

    m_isCompiled = true;
}

void LabelMesh::draw(ShaderProgram& _shader) {
    bool valid = checkValidity();

    if (!m_isCompiled) { return; }
    if (m_nVertices == 0) { return; }

    // Ensure that geometry is buffered into GPU
    if (!m_isUploaded) {
        upload();
    } else if (m_dirty) {
        subDataUpload();
    }

    if (!valid) {
        loadQuadIndices();
    }

    // Bind buffers for drawing
    RenderState::vertexBuffer(m_glVertexBuffer);
    RenderState::indexBuffer(s_quadIndexBuffer);

    // Enable shader program
    _shader.use();

    size_t vertexOffset = 0;

    for (size_t i = 0; i < m_vertexOffsets.size(); ++i) {
        auto& o = m_vertexOffsets[i];
        uint32_t nIndices = o.first;
        uint32_t nVertices = o.second;

        size_t byteOffset = vertexOffset * m_vertexLayout->getStride();

        m_vertexLayout->enable(_shader, byteOffset);

        glDrawElements(m_drawMode, nIndices, GL_UNSIGNED_SHORT, 0);

        vertexOffset += nVertices;
    }
}

}

