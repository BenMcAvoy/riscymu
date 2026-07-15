#ifdef RISCYMU_WINDOWS
#include <Windows.h>

#include <print>

void platform_setup()
{
    std::println("Setting up platform-specific configurations...");
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
}
#else
void platform_setup()
{
    std::println("No platform-specific configurations needed for this platform.");
}
#endif