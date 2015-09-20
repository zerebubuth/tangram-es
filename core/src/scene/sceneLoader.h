#pragma once

#include <string>
#include <vector>
#include <memory>

/* Forward Declaration of yaml-cpp node type */
namespace YAML {
    class Node;
}

namespace Tangram {

class Scene;
class TileManager;
class SceneLayer;
class View;
class ShaderProgram;
class Material;
class Style;
struct StyleParam;
struct DrawRule;
struct MaterialTexture;
struct Filter;

using Mixes = std::vector<YAML::Node>;

struct SceneLoader {
    using Node = YAML::Node;

    static bool loadScene(const std::string& _sceneString, Scene& _scene);

    /*** public for testing ***/

    static void loadSource(const std::pair<Node, Node>& source, Scene& scene);
    static void loadTexture(const std::pair<Node, Node>& texture, Scene& scene);
    static void loadStyle(const std::pair<Node, Node>& style, Node styles, Scene& scene);
    static void loadLayer(const std::pair<Node, Node>& layer, Scene& scene);
    static void loadLight(const std::pair<Node, Node>& light, Scene& scene);
    static void loadFont(Node fontProps);
    static void loadCameras(Node cameras, Scene& scene);
    static void loadStyleProps(Style& style, Node styleNode, Scene& scene);
    static void loadMaterial(Node matNode, Material& material, Scene& scene);
    static void loadShaderConfig(Node shaders, ShaderProgram& shader);
    static SceneLayer loadSublayer(Node layer, const std::string& name, Scene& scene);
    static Filter generateAnyFilter(Node filter, Scene& scene);
    static Filter generateNoneFilter(Node filter, Scene& scene);
    static Filter generatePredicate(Node filter, std::string _key);

    // Style Mixing helper methods
    static Node mixStyle(const Mixes& mixes);

    static MaterialTexture loadMaterialTexture(Node matCompNode, Scene& scene);

    static void parseStyleParams(Node params, Scene& scene, const std::string& propPrefix,
                                 std::vector<StyleParam>& out);

    // Generic methods to merge properties
    static Node propMerge(const std::string& propStr, const Mixes& mixes);

    // Methods to merge shader blocks
    static Node shaderBlockMerge(const Mixes& mixes);

    // Methods to merge shader extensions
    static Node shaderExtMerge(const Mixes& mixes);
    static Tangram::Filter generateFilter(Node filter, Scene& scene);

    SceneLoader() = delete;
};

}
