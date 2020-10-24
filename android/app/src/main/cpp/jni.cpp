#include <jni.h>
#include <memory>
#include <indoors/pipeline/platform.h>
#include <indoors/magnetics/task.h>
#include <indoors/magnetics/filter_task.h>
#include <streambuf>
#include <android/log.h>
#include <iostream>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

namespace {
    std::string jstring2string(JNIEnv *env, jstring jStr) {
        if (!jStr)
            return "";

        const jclass stringClass = env->GetObjectClass(jStr);
        const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
        const jbyteArray stringJbytes = (jbyteArray) env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

        size_t length = (size_t) env->GetArrayLength(stringJbytes);
        jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

        std::string ret = std::string((char *)pBytes, length);
        env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

        env->DeleteLocalRef(stringJbytes);
        env->DeleteLocalRef(stringClass);
        return ret;
    }

    class androidbuf : public std::streambuf {
    public:
        androidbuf(int prio, std::string tag) : prio{prio}, tag{std::move(tag)} { this->setp(buffer, buffer + bufsize - 1); }

    private:
        int overflow(int c) override {
            if (c == traits_type::eof()) {
                *this->pptr() = traits_type::to_char_type(c);
                this->sbumpc();
            }
            return this->sync() ? traits_type::eof() : traits_type::not_eof(c);
        }

        int sync() override {
            int rc = 0;
            if (this->pbase() != this->pptr()) {
                char writebuf[bufsize + 1];
                memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
                writebuf[this->pptr() - this->pbase()] = '\0';

                rc = __android_log_write(ANDROID_LOG_INFO, "std", writebuf) > 0;
                this->setp(buffer, buffer + bufsize - 1);
            }
            return rc;
        }

        static constexpr auto bufsize = 128;
        const int prio{ANDROID_LOG_INFO};
        const std::string tag;
        char buffer[bufsize];
    };

    template <typename T>
    std::shared_ptr<T> &handle_to_shared_ptr(JNIEnv *env, jobject thiz) {
        // TODO could also store a short hand handle to the platform
        jclass platformClass = env->GetObjectClass(thiz);
        jfieldID handleField = env->GetFieldID(platformClass, "handle", "J");
        jlong handle = env->GetLongField(thiz, handleField);

        auto platform = reinterpret_cast<std::shared_ptr<T> *>(handle);
        return *platform;
    }

    std::shared_ptr<StandardPlatform> &get_platform(JNIEnv *env, jobject thiz) {
        return handle_to_shared_ptr<StandardPlatform>(env, thiz);
    }

    std::shared_ptr<Task> &get_task(JNIEnv *env, jobject thiz) {
        return handle_to_shared_ptr<Task>(env, thiz);
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_create(JNIEnv *env, jclass clazz) {
    // TODO
    std::cout.rdbuf(new androidbuf(ANDROID_LOG_INFO, "cout"));
    std::cerr.rdbuf(new androidbuf(ANDROID_LOG_ERROR, "cerr"));

    auto tmp = std::make_shared<StandardPlatform>("android");
    auto platform = new std::shared_ptr<StandardPlatform>(std::move(tmp));
    return reinterpret_cast<jlong>(platform);
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_destroy(JNIEnv *env, jclass clazz, jlong handle) {
    auto platform = reinterpret_cast<std::shared_ptr<StandardPlatform> *>(handle);
    platform->reset();
    delete platform;
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushStart(JNIEnv *env, jobject thiz, jdouble time) {
    // TODO
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushStop(JNIEnv *env, jobject thiz, jdouble time) {
    // TODO
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushTick(JNIEnv *env, jobject thiz, jdouble time) {
    get_platform(env, thiz)->m_tick.push({time});
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushClock(JNIEnv *env, jobject thiz, jdouble time,
                                                       jdouble unix_utc) {
    get_platform(env, thiz)->m_clock.push({time, unix_utc});
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushAccelerometer(JNIEnv *env, jobject thiz,
                                                               jdouble time, jdouble x, jdouble y,
                                                               jdouble z) {
    get_platform(env, thiz)->m_accelerometer.push({time, x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushGyroscope(JNIEnv *env, jobject thiz, jdouble time,
                                                           jdouble x, jdouble y, jdouble z) {
    get_platform(env, thiz)->m_gyroscope.push({time, x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushMagnetometer(JNIEnv *env, jobject thiz,
                                                              jdouble time, jdouble x, jdouble y,
                                                              jdouble z) {
    get_platform(env, thiz)->m_magnetometer.push({time, x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushMagnetometerUncalibrated(JNIEnv *env, jobject thiz,
                                                                          jdouble time, jdouble x,
                                                                          jdouble y, jdouble z) {
    get_platform(env, thiz)->m_magnetometer_uncalibrated.push({time, x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushMagnetometerBias(JNIEnv *env, jobject thiz,
                                                                  jdouble time, jdouble x,
                                                                  jdouble y, jdouble z) {
    get_platform(env, thiz)->m_magnetometer_bias.push({time, x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_AndroidPlatform_pushOrientation(JNIEnv *env, jobject thiz,
                                                             jdouble time, jdouble w, jdouble x,
                                                             jdouble y, jdouble z) {
    get_platform(env, thiz)->m_orientation.push({time, w, x, y, z});
}

extern "C"
JNIEXPORT jlong JNICALL
Java_at_stefl_magnetics_core_NativeTask_createFilter_1(JNIEnv *env, jclass clazz,
                                                       jobject platform) {
    auto &&p = get_platform(env, platform);

    auto tmp = std::make_shared<FilterTask>(p);
    auto task = new std::shared_ptr<Task>(std::move(tmp));
    return reinterpret_cast<jlong>(task);
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_NativeTask_destroy(JNIEnv *env, jclass clazz, jlong handle) {
    auto task = reinterpret_cast<std::shared_ptr<Task> *>(handle);
    task->reset();
    delete task;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_at_stefl_magnetics_core_NativeTask_annotation(JNIEnv *env, jobject thiz) {
    auto &&annotation = get_task(env, thiz)->annotation();
    return env->NewStringUTF(annotation.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_NativeTask_start(JNIEnv *env, jobject thiz) {
    get_task(env, thiz)->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_at_stefl_magnetics_core_NativeTask_stop(JNIEnv *env, jobject thiz) {
    get_task(env, thiz)->stop();
}
