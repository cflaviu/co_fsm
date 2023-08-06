import qbs

StaticLibrary {
    name: "co_fsm"
    Depends {
        name: "cpp"
    }
    files: [
        "co_fsm/automaton.hpp",
        "co_fsm/event_base.hpp",
        "co_fsm/headers.hpp",
        "co_fsm/state.hpp",
    ]
    cpp.cxxLanguageVersion: "c++20"
    cpp.enableRtti: false
    cpp.includePaths: ["co_fsm"]

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
