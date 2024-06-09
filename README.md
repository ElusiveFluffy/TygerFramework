# TygerFramework

## API Initialization
To use the API download `API.h` and `API.hpp` either from the zip in the releases tab or from the main branch and copy them into your C++ project folder. Then in visual studio add them as a existing item (If you don't copy them to the project folder it'll just reference the original location for the file which can make the include path long)

Then include API.hpp in any classes you'll use it in

To initialize the API create a export function either like
```C++
EXTERN_C bool TygerFrameworkPluginInitialize(TygerFrameworkPluginInitializeParam* param) {
    //Make sure to call this first before any API Functions
    API::Initialize(param);
```
or
```C++
extern "C" __declspec(dllexport) bool TygerFrameworkPluginInitialize(TygerFrameworkPluginInitializeParam* param) {
    //Make sure to call this first before any API Functions
    API::Initialize(param);
```

If you're using a C++ project file make sure to also add a `Source.def` file. To create one from the add new file menu go to the code section then Module-Definition File. To make sure it auto linked right click on `project` then `properties>linker>input` and the Source.def file should be in the module definition file section, if not just add it in there.

Next you need to specifiy the export in your `Source.def` file, open it up and make it look like this
```
LIBRARY "Example Plugin"

EXPORTS
TygerFrameworkPluginInitialize
```
Of course replacing `"Example Plugin"` with your own project name, the quotation marks are only needed if your project name has a space in it.

## API Usage
Now that you've added the API and initialized it you can use the functions within it.
To log a message just call the log function like this
```C++
API::Get()->LogPluginMessage("Hello World From Example Plugin!");
```
The log will default to info but if you want to specify a different log level you can like this
```C++
API::Get()->LogPluginMessage("Warning Test", Warning);
API::Get()->LogPluginMessage("Error Test", Error);
```

To get which Ty game is currently running call the function like this
```C++
API::Get()->param()->functions->WhichTyGame();
```
I would kinda recommend also starting up a new thread for any code within your plugin that is always running in the background, just in case to not lock up any other plugins

## Plugin Name for Log Function
The log will default to using the plugin's file name in the log but you can specify a custom name for the log in ``DllMain`` so its set before intilization of the API
```C++
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        API::PluginName = "Example Plugin Custom Name";
    }
    return TRUE;
}
```

## Post Build Command
I highly recommend adding a post build command to auto copy the built dll to the plugin folder, as it makes it alot more convenient to test the plugin without having to copy the dll to the plugin folder each time

To add one just right click on the project file then `properties>Build Events>Post-Build Event`, in the command line section I added this command for the example plugin
```
copy "$(OutDir)Example Plugin.dll" "D:\SteamLibrary\steamapps\common\TY the Tasmanian Tiger\Plugins\Example Plugin.dll"
```
Of course changing `Example Plugin.dll` at the start and in the directory to the name of your plugin, and changing the directory to where your installation of Ty and the plugin folder is

## Example Plugin
I've created a example plugin of how you would go about using the API if you need to see how it works in a actual project [https://github.com/ElusiveFluffy/Example-Plugin](https://github.com/ElusiveFluffy/Example-Plugin)
