import qbs

CppApplication {
    consoleApplication: true
    Depends {
        name: "co_fsm"
    }
    files: [
        "common.hpp",
        "diagram/connected.png",
        "diagram/separated.png",
        "blue.cpp",
        "green.cpp",
        "led_control.hpp",
        "my_event.hpp",
        "red.cpp",
        "rgb.cpp",
        "rgb.hpp",
        "simple_logger.hpp",
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
