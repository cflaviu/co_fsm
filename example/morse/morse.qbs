import qbs

CppApplication {
    consoleApplication: true
    Depends {
        name: "co_fsm"
    }
    files: [
        "common.hpp",
        "diagram.png",
        "event.hpp",
        "morse.cpp",
        "morse.hpp",
        "simple_logger.hpp",
        "sound_controller.cpp",
        "sound_controller.hpp",
        "sound_on_state_handler.cpp",
        "sound_on_state_handler.hpp",
        "transmission_in_progress_state_handler.cpp",
        "transmission_in_progress_state_handler.hpp",
        "transmission_ready_state_handler.cpp",
        "transmission_ready_state_handler.hpp",
    ]
    cpp.cxxLanguageVersion: "c++20"
    cpp.enableRtti: false
    cpp.includePaths: ["../../source"]

    Properties {
        condition: qbs.buildVariant === "release"
        cpp.cxxFlags: ["-Ofast"]
    }
    Properties {
        condition: qbs.buildVariant === "debug"
        cpp.defines: ["ASAN_OPTIONS=abort_on_error=1:report_objects=1:sleep_before_dying=1"]
        cpp.cxxFlags: "-fsanitize=address"
        cpp.staticLibraries: "asan"
    }
}
