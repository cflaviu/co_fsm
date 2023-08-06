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
        "ready_state_handler.cpp",
        "ready_state_handler.hpp",
        "ring.cpp",
        "ring.hpp",
        "ring_state_handler.cpp",
        "ring_state_handler.hpp",
    ]
    cpp.cxxLanguageVersion: "c++20"
    cpp.enableRtti: false
    cpp.includePaths: ["../../source"]

    Properties {
        condition: qbs.buildVariant === "release"
        cpp.cxxFlags: ["-O3"]
    }
    Properties {
        condition: qbs.buildVariant === "debug"
        cpp.defines: ["ASAN_OPTIONS=abort_on_error=1:report_objects=1:sleep_before_dying=1"]
        cpp.cxxFlags: "-fsanitize=address"
        cpp.staticLibraries: "asan"
    }
}
