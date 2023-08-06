#pragma once
#include <iostream>
#include <mutex>
#include <thread>

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

namespace co_fsm::rgb
{
    extern std::mutex led_mutex;

    // Mockup for Blue LED controller
    class led_control
    {
    public:
        enum class Status
        {
            Off,
            On
        };

    private:
        Status _ledStatus = Status::Off;
#if USE_KEYBOARD_LED
        static constexpr unsigned char _enableCode = 0x1;
        int _fdConsole = -1; // File descriptor for linux console
#endif

        const char* name_;

    public:
        led_control(const char* name): name_(name)
        {
#if USE_KEYBOARD_LEDS
            _fdConsole = open("/dev/console", O_WRONLY);
            if (_fdConsole == -1)
            {
                std::cerr << "Error opening console file descriptor.\n"
                          << "Run the application with sudo or 'make clean' and make without linux argument.\n";
                std::terminate();
            }
            unsigned char led;
            ioctl(_fdConsole, KDGETLED, &led);
            ioctl(_fdConsole, KDSETLED, led & ~_enableCode); // Turn off
#endif
        }

        void set(Status value)
        {
            std::lock_guard<std::mutex> lock(led_mutex);
            _ledStatus = value;

#if USE_KEYBOARD_LEDS
            {
                unsigned char led;
                ioctl(_fdConsole, KDGETLED, &led);
                switch (value)
                {
                    case Status::On:
                        ioctl(_fdConsole, KDSETLED, led | _enableCode);
                        break;
                    case Status::Off:
                        ioctl(_fdConsole, KDSETLED, led & ~_enableCode);
                        break;
                }
            }
#endif
            std::cout << "Blue  LED = " << (_ledStatus == Status::Off ? "Off" : "On") << '\n';
        }
        Status get() const { return _ledStatus; }
    };
}
