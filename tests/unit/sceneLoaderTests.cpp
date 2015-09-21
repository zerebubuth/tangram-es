#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "yaml-cpp/yaml.h"
#include "sceneLoader.h"
#include "scene/scene.h"
#include "style/style.h"
#include "style/polylineStyle.h"
#include "style/polygonStyle.h"

#include "platform.h"

using namespace Tangram;
using YAML::Node;

TEST_CASE( "Test style loading" ) {
    Scene scene;

    scene.styles().emplace_back(new PolygonStyle("polygons"));
    scene.styles().emplace_back(new PolylineStyle("lines"));

    YAML::Node node = YAML::Load(R"END(
    roads:
      animated: true
      texcoords: true
      base: lines
      mix: tools
      material:
        diffuse: .9
        emission: 0.0
     )END");

    REQUIRE(node.IsMap());
    REQUIRE(node.size() == 1);

    // logMsg("Node:\n'%s'\n", Dump(node).c_str());

    const auto& n = node.begin();

    SceneLoader::loadStyle(*n, node, scene);

    auto& styles = scene.styles();

    REQUIRE(styles.size() == 3);
    REQUIRE(styles[0]->getName() == "polygons");
    REQUIRE(styles[1]->getName() == "lines");
    REQUIRE(styles[2]->getName() == "roads");

    REQUIRE(styles[2]->isAnimated() == true);

    REQUIRE(styles[2]->getMaterial()->hasEmission() == true);
    REQUIRE(styles[2]->getMaterial()->hasDiffuse() == true);
    REQUIRE(styles[2]->getMaterial()->hasAmbient() == true);
    REQUIRE(styles[2]->getMaterial()->hasSpecular() == false);
}
