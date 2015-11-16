#pragma once

#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"
#include <climits> // needed in aabb.h
#include "isect2d.h"
#include "glm_vec.h" // for isect2d.h
#include "fadeEffect.h"
#include "util/types.h"
#include "util/hash.h"
#include "data/properties.h"

#include <string>
#include <limits>
#include <memory>

namespace Tangram {

class LabelMesh;

class Label {

public:

    using OBB = isect2d::OBB<glm::vec2>;
    using AABB = isect2d::AABB<glm::vec2>;

    enum class Type {
        point,
        line,
        debug
    };

    enum State {
        fading_in       = 1,
        fading_out      = 1 << 1,
        visible         = 1 << 2,
        sleep           = 1 << 3,
        out_of_screen   = 1 << 4,
        wait_occ        = 1 << 5, // state waiting for first occlusion result
        dead            = 1 << 6,
    };

    struct Vertex {
        Vertex(glm::vec2 pos, glm::vec2 uv, glm::vec3 extrude, uint32_t color, uint32_t stroke = 0)
            : pos(pos), uv(uv), extrude(extrude), color(color), stroke(stroke) {}

        glm::vec2 pos;
        glm::vec2 uv;
        glm::vec3 extrude;
        uint32_t color;
        uint32_t stroke;
        struct State {
            glm::vec2 screenPos;
            float alpha = 0.f;
            float rotation = 0.f;
        } state;
    };

    struct Transform {
        Transform(glm::vec2 _pos) : modelPosition1(_pos), modelPosition2(_pos) {}
        Transform(glm::vec2 _pos1, glm::vec2 _pos2) : modelPosition1(_pos1), modelPosition2(_pos2) {}

        glm::vec2 modelPosition1;
        glm::vec2 modelPosition2;

        Vertex::State state;
    };

    struct Transition {
        FadeEffect::Interpolation ease = FadeEffect::Interpolation::sine;
        float time = 0.2;
    };

    struct Options {
        glm::vec2 offset;
        uint32_t priority = std::numeric_limits<uint32_t>::max();
        bool interactive = false;
        std::shared_ptr<Properties> properties;
        bool collide = true;
        Transition selectTransition;
        Transition hideTransition;
        Transition showTransition;
    };

    Label(Transform _transform, glm::vec2 _size, Type _type, LabelMesh& _mesh, Range _vertexRange, Options _options);

    virtual ~Label();

    const Transform& getTransform() const { return m_transform; }

    /* Update the transform of the label in world space, and project it to screen space */
    void updateTransform(const Transform& _transform, const glm::mat4& _mvp, const glm::vec2& _screenSize);

    /* Gets the oriented bounding box of the label */
    const OBB& getOBB() const { return m_obb; }

    /* Gets the extent of the oriented bounding box of the label */
    const AABB& getAABB() const { return m_aabb; }

    /* Gets for label options: color and offset */
    const Options& getOptions() const { return m_options; }

    bool update(const glm::mat4& _mvp, const glm::vec2& _screenSize, float _dt, float _zoomFract);

    /* Push the pending transforms to the vbo by updating the vertices */
    void pushTransform();

    /* Update the screen position of the label */
    bool updateScreenTransform(const glm::mat4& _mvp, const glm::vec2& _screenSize,
                               bool _testVisibility = true);

    virtual void updateBBoxes(float _zoomFract) = 0;


    /* Sets the occlusion */
    void setOcclusion(bool _occlusion);

    /* Checks whether the label is in a state where it can occlusion */
    bool canOcclude();

    /* Mark the label as resolved */
    void occlusionSolved();

    void skipTransitions();

    bool occludedLastFrame() { return m_occludedLastFrame; }

    State getState() const { return m_currentState; }

    /* Checks whether the label is in a visible state */
    bool visibleState() const;

    void resetState();

private:

    bool offViewport(const glm::vec2& _screenSize);

    inline void enterState(const State& _state, float _alpha = 1.0f);

    bool updateState(const glm::mat4& _mvp, const glm::vec2& _screenSize, float _dt, float _zoomFract);

    void setAlpha(float _alpha);

    // the current label state
    State m_currentState;
    // the label fade effect
    FadeEffect m_fade;
    // whether the label was occluded on the previous frame
    bool m_occludedLastFrame;
    // whether or not the occlusion has been solved by the occlusion manager
    bool m_occlusionSolved;
    // whether or not we need to update the mesh visibilit (alpha channel)
    bool m_updateMeshVisibility;
    // label options
    Options m_options;
    bool m_skipTransitions;

protected:

    // set alignment on _screenPosition based on anchor points _ap1, _ap2
    virtual void align(glm::vec2& _screenPosition, const glm::vec2& _ap1, const glm::vec2& _ap2) = 0;

    // the label type (point/line)
    Type m_type;
    // the label oriented bounding box
    OBB m_obb;
    // the label axis aligned bounding box
    AABB m_aabb;
    // whether the label is dirty, this determines whether or no to update the geometry
    bool m_dirty;
    // the label transforms
    Transform m_transform;
    // the dimension of the label
    glm::vec2 m_dim;
    // Back-pointer to owning container
    LabelMesh& m_mesh;
    // first vertex and count in m_mesh vertices
    Range m_vertexRange;
};

}

namespace std {
    template <>
    struct hash<Tangram::Label::Options> {
        size_t operator() (const Tangram::Label::Options& o) const {
            std::size_t seed = 0;
            hash_combine(seed, o.offset.x);
            hash_combine(seed, o.offset.y);
            hash_combine(seed, o.priority);
            hash_combine(seed, o.interactive);
            hash_combine(seed, o.collide);
            hash_combine(seed, o.selectTransition.ease);
            hash_combine(seed, o.selectTransition.time);
            hash_combine(seed, o.hideTransition.ease);
            hash_combine(seed, o.hideTransition.time);
            hash_combine(seed, o.showTransition.ease);
            hash_combine(seed, o.showTransition.time);
            return seed;
        }
    };
}

