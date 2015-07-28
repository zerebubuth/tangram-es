#include "labels/labelMesh.h"
#include "labels/label.h"

namespace Tangram {

LabelMesh::LabelMesh(std::shared_ptr<VertexLayout> _vertexLayout, GLenum _drawMode)
    : TypedMesh<Label::Vertex>(_vertexLayout, _drawMode, GL_DYNAMIC_DRAW) {
}

LabelMesh::~LabelMesh() {}


}
