# TygerFramework

## Documentation

Full documentation on getting started with making your own plugin, and documentation of all the functions in the API, is available on the modding wiki here: [https://tymoddingwiki.github.io/TygerFramework/](https://tymoddingwiki.github.io/TygerFramework/)

## Build Instructions

If you want to build TygerFramework from source you just need to install MinHook, which I recommend installing with vcpkg. 

After getting vcpkg all set up to work with visual studio just enter this command for it to install Minhook

```
vcpkg install minhook --triplet x86-windows-static
```

You'll probably also need to edit the post build command to your Ty directory (if its not on the D drive), after that you should be able to build the project.