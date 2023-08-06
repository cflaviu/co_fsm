import qbs

CppApplication {
    consoleApplication: true
    Depends {
        name: "co_fsm"
    }
    files: [
        "diagram.png",
        "simple_logger.hpp",
        "ping_pong.cpp",
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
