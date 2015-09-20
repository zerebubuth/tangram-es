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

class SceneLoader {

    static void loadSources(YAML::Node sources, Scene& scene);
    static void loadFont(YAML::Node fontProps);
    static void loadLights(YAML::Node lights, Scene& scene);
    static void loadCameras(YAML::Node cameras, Scene& scene);
    static void loadLayers(YAML::Node layers, Scene& scene);
    static void loadStyles(YAML::Node styles, Scene& scene);
    static void loadStyleProps(Style* style, YAML::Node styleNode, Scene& scene);
    static void loadTextures(YAML::Node textures, Scene& scene);
    static void loadMaterial(YAML::Node matNode, Material& material, Scene& scene);
    static void loadShaderConfig(YAML::Node shaders, ShaderProgram& shader);
    static SceneLayer loadSublayer(YAML::Node layer, const std::string& name, Scene& scene);
    static Filter generateAnyFilter(YAML::Node filter, Scene& scene);
    static Filter generateNoneFilter(YAML::Node filter, Scene& scene);
    static Filter generatePredicate(YAML::Node filter, std::string _key);

    // Style Mixing helper methods
    static YAML::Node mixStyle(const Mixes& mixes);

public:
    SceneLoader() = delete;

    static bool loadScene(const std::string& _sceneString, Scene& _scene);

    static MaterialTexture loadMaterialTexture(YAML::Node matCompNode, Scene& scene);

    // public for testing
    static std::vector<StyleParam> parseStyleParams(YAML::Node params, Scene& scene, const std::string& propPrefix = "");

    // Generic methods to merge properties
    static YAML::Node propMerge(const std::string& propStr, const Mixes& mixes);

    // Methods to merge shader blocks
    static YAML::Node shaderBlockMerge(const Mixes& mixes);

    // Methods to merge shader extensions
    static YAML::Node shaderExtMerge(const Mixes& mixes);
    static Tangram::Filter generateFilter(YAML::Node filter, Scene& scene);
};

}
