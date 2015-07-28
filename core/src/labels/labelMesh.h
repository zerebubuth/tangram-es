#pragma once

#include "gl/typedMesh.h"
#include "labels/label.h"

#include <functional>
#include <memory>
#include <vector>

namespace Tangram {

class LabelMesh : public TypedMesh<Label::Vertex> {
public:
    LabelMesh(std::shared_ptr<VertexLayout> _vertexLayout, GLenum _drawMode);

    virtual ~LabelMesh();
    virtual void each(std::function<void(Label&)> _fn) = 0;
    virtual void appendLabels(std::vector<Label*>& _labels) = 0;
};

}
