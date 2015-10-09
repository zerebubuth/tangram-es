#pragma once

#include "duktape.h"
#include "tileData.h"
#include "scene/drawRule.h"
#include "util/fastmap.h"

#include <string>
#include <functional>

namespace Tangram {

class Scene;
enum class StyleParamKey : uint8_t;
struct StyleParam;

class StyleContext {

public:

    using FunctionID = uint32_t;

    StyleContext();
    ~StyleContext();

    bool addFunction(const std::string& _name, const std::string& _func);

    bool evalFilterFn(const std::string& _name);
    bool evalFilter(FunctionID id) const;

    bool evalStyleFn(const std::string& _name, StyleParamKey _key, StyleParam::Value& _val);
    bool evalStyle(FunctionID id, StyleParamKey _key, StyleParam::Value& _val) const;

    void setFeature(const Feature& _feature);

    void setGlobal(const std::string& _key, const Value& _value);
    void setGlobalZoom(float _zoom);

    const Value& getGlobal(const std::string& _key) const;
    float getGlobalZoom() const { return m_globalZoom; }

    void clear();

    void initFunctions(const Scene& _scene);


private:
    static duk_ret_t jsPropertyGetter(duk_context *_ctx);
    static duk_ret_t jsPropertySetter(duk_context *_ctx);

    bool parseStyleResult(StyleParamKey _key, StyleParam::Value& _val) const;

    void setAccessors() const;
    void addAccessor(const std::string& _name) const;

    mutable duk_context *m_ctx;

    const Feature* m_feature = nullptr;
    mutable bool m_featureIsReady;

    struct Accessor {
        std::string key;
        const StyleContext* ctx;
    };

    mutable fastmap<std::string, Accessor> m_accessors;

    fastmap<std::string, Value> m_globals;

    int32_t m_sceneId = -1;

    float m_globalZoom = -1;
};

}
