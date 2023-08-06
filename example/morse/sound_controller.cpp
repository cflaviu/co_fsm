#include "sound_controller.hpp"

#if USE_KEYBOARD_LEDS
    #include <sys/ioctl.h>
#endif

namespace co_fsm::morse
{
    sound_controller::sound_controller()
    {
#if USE_KEYBOARD_LEDS
        fd_console_ = open("/dev/console", O_WRONLY);
        if (fd_console_ == -1)
        {
            std::cerr << "Error opening console file descriptor.\n"
                      << "Run the application with sudo or 'make clean' and make without linux argument.\n";
            std::terminate();
        }
        unsigned char led;
        ioctl(fd_console_, KDGETLED, &led);
        ioctl(fd_console_, KDSETLED, led & ~enable_code_); // Turn off
#endif
    }

    void sound_controller::set(status value)
    {
        sound_status_ = value;
#if USE_KEYBOARD_LEDS
        {
            unsigned char led;
            ioctl(fd_console_, KDGETLED, &led);
            switch (value)
            {
                case status::on:
                    ioctl(fd_console_, KDSETLED, led | enable_code_);
                    break;
                case status::off:
                    ioctl(fd_console_, KDSETLED, led & ~enable_code_);
                    break;
            }
        }
#endif
        // std::cout << "Sound = " << (sound_status_ == status::off ? "off" : "on") << '\n';
    }
}
