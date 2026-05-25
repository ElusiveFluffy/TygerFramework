# TygerFramework

## Documentation

Full documentation on getting started with making your own plugin, and documentation of all the functions in the API, is available on the modding wiki here: [https://tymoddingwiki.github.io/TygerFramework/](https://tymoddingwiki.github.io/TygerFramework/)

## Build Instructions

If you want to build TygerFramework from source you just need to install MinHook, which I recommend installing with vcpkg. 

After getting vcpkg all set up to work with visual studio just enter this command for it to install Minhook

```
vcpkg install minhook --triplet x86-windows-static-md
```

You'll probably also need to edit the post build command to your Ty directory (if its not on the D drive), after that you should be able to build the project.

## Toolchain-independent plugins (extern "C" surface)

As of plugin API 1.3.0, plugins recompiled against `TygerFrameworkAPI.hpp` are
binary-compatible with TygerFramework regardless of the toolchain/STL they were
built with (e.g. MinGW cross-compiles from Linux). The public `API` class is
unchanged; only the plugin entry points change — declare them with one macro:

    void MyVersion(TygerFrameworkPluginVersion& v) { /* ... */ }
    bool MyInit(const TygerFrameworkPluginInitializeParam* p) { API::Initialize(p); /* ... */ return true; }
    TYGERFRAMEWORK_PLUGIN(MyVersion, MyInit)

When recompiling an existing plugin against the 1.3.0 header you must switch its
entry points to the `TYGERFRAMEWORK_PLUGIN` macro: the `API::` helper methods now
route through the C surface, which the macro binds, and they will throw if it is
not used.

Plugins compiled against earlier headers continue to load unchanged.
