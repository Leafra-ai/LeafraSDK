#include "leafra/platform_utils.h"
#include <string>

namespace leafra {

std::string PlatformUtils::get_platform_name() {
#ifdef __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        return "iOS";
    #elif TARGET_OS_MAC
        return "macOS";
    #else
        return "Apple";
    #endif
#elif defined(__ANDROID__)
    return "Android";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__linux__)
    return "Linux";
#else
    return "Unknown";
#endif
}

std::string PlatformUtils::get_architecture() {
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__i386) || defined(_M_IX86)
    return "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#else
    return "unknown";
#endif
}

bool PlatformUtils::is_mobile_platform() {
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        return true;
    #else
        return false;
    #endif
#elif defined(__ANDROID__)
    return true;
#else
    return false;
#endif
}

bool PlatformUtils::is_desktop_platform() {
    return !is_mobile_platform();
}

std::string PlatformUtils::get_platform_info() {
    return get_platform_name() + " (" + get_architecture() + ")";
}

} // namespace leafra 