This is the FileSwitcher plugin, allowing you to quickly switch files in
Notepad++.

Type any part of the filename, and the list of options shows filenames
containing that text.  Options to only use the start of the filename, case
sensitivity and to include the path of the file rather than just the filename
are available in the Options dialog.

## How to build

Visual Studio 2012 or later is required, and will compile the code without
requiring any further dependencies.  You will need to customize a few build
properties before compiling, though.  To do this *without* interfering with this
Git repository, create a file named `LocalSettings.props` in the `projects\2012`
subdirectory, using the following template:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
  </PropertyGroup>
</Project>
```

Inside the `<PropertyGroup>` tag, you can set the following:

* `<PlatformToolset>`: Necessary for compiling on non-2012 Visual Studio
versions. For a list of all platform toolsets available on your system, open the
Project Properties dialog (`Alt+F7`) and refer to the drop-down menu at
*Configuration Properties → General → Platform Toolset*.

* `<NPPBinRoot>`: Path to your Notepad++ test instance. The plugin DLL will be
put into `$(NPPBinRoot)\plugins\`, and debugging from Visual Studio will start
`$(NPPBinRoot)\notepad++.exe`.

* `<NPPSourceRoot>`: If you want to test with your own Notepad++ build that you
compiled using its Visual Studio projects, you can set this to the directory or
your Notepad++ source tree (the directory that contains the `.git`,
`PowerEditor`, and `scintilla` subdirectories).  Doing this will automatically
set `$(NPPBinRoot)` to the correct output directory depending on the
configuration/platform selected for compiling this plugin, according to the
following table:

|               | Win32                                     | x64                                              |
|--------------:|-------------------------------------------|--------------------------------------------------|
| ANSI Debug    | `.\PowerEditor\visual.net\ANSI Debug\`    | `.\PowerEditor\visual.net\x64\ANSI Debug\`       |
| Unicode Debug | `.\PowerEditor\visual.net\Unicode Debug\` | `.\PowerEditor\visual.net\x64\Unicode Debug\`    |
| Release       | `.\PowerEditor\bin\`                      | `.\PowerEditor\bin64\`                           |

For debugging, make sure that you've compiled Notepad++ with the *identical*
configuration/platform combination, which will have put a `notepad++.exe` into
the correct path.

### Example templates

#### Testing with your own Notepad++ builds from a source tree, on Visual Studio 2013 with XP targeting

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <PlatformToolset>v120_xp</PlatformToolset>
    <NPPSourceRoot>C:\path\to\notepad-plus-plus\</NPPSourceRoot>
  </PropertyGroup>
</Project>
```

#### Testing with official Notepad++ binaries, on Visual Studio 2015

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <PlatformToolset>v140</PlatformToolset>
    <NPPBinRoot Condition="'$(Platform)'=='Win32'">C:\path\to\32-bit\notepad++\</NPPBinRoot>
    <NPPBinRoot Condition="'$(Platform)'=='x64'">C:\path\to\64-bit\notepad++\</NPPBinRoot>
  </PropertyGroup>
</Project>
```
