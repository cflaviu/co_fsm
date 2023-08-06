#pragma once

// Include headers needed for keyboard LED control is LINUX flag is defined.
#if defined(LINUX) && __has_include(<sys/ioctl.h>) && __has_include(<sys/kd.h>) && __has_include(<fcntl.h>) && __has_include(<unistd.h>)
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include <sys/kd.h>
    #include <unistd.h>
    #define USE_KEYBOARD_LEDS 1
#else
    #define USE_KEYBOARD_LEDS 0
#endif

namespace co_fsm::morse
{
    class sound_controller
    {
    public:
        enum class status
        {
            off,
            on
        };

        sound_controller();
        void set(status value);

    private:
        status sound_status_ {};
#if USE_KEYBOARD_LEDS
        static constexpr unsigned char enable_code_ = 0x7;
        int fd_console_ = -1; // File descriptor for linux console
#endif
    };
}
