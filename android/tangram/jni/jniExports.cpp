#include <jni.h>
#include "tangram.h"
#include "platform.h"

extern "C" {

    JNIEXPORT void JNICALL Java_com_mapzen_tangram_MapController_screenToWorldCoordinates(JNIEnv* jniEnv, jobject obj, jdoubleArray screenPos) {
        jdouble* arr = jniEnv->GetDoubleArrayElements(screenPos, NULL);
        Tangram::screenToWorldCoordinates(arr[0], arr[1]);
        jniEnv->ReleaseDoubleArrayElements(screenPos, arr, 0);
    }

    JNIEXPORT void JNICALL Java_com_mapzen_tangram_MapController_init(JNIEnv* jniEnv, jobject obj, jobject tangramInstance, jobject assetManager, jstring stylePath) {
        setupJniEnv(jniEnv, tangramInstance, assetManager);
        const char* cStylePath = jniEnv->GetStringUTFChars(stylePath, NULL);
        Tangram::initialize(cStylePath);
        jniEnv->ReleaseStringUTFChars(stylePath, cStylePath);
    }

    JNIEXPORT void JNICALL Java_com_mapzen_tangram_MapController_onUrlSuccess(JNIEnv* jniEnv, jobject obj, jbyteArray fetchedBytes, jlong callbackPtr) {
        onUrlSuccess(jniEnv, fetchedBytes, callbackPtr);
    }

    JNIEXPORT void JNICALL Java_com_mapzen_tangram_MapController_onUrlFailure(JNIEnv* jniEnv, jobject obj, jlong callbackPtr) {
        onUrlFailure(jniEnv, callbackPtr);
    }

    JNIEXPORT void JNICALL Java_com_mapzen_tangram_MapController_pickFeature(JNIEnv* jniEnv, jobject obj, jfloat posX, jfloat posY) {
        auto& items = Tangram::pickFeaturesAt(posX, posY);
        if (!items.empty()) {
            featureSelectionCallback(jniEnv, items);
        }
    }

}

