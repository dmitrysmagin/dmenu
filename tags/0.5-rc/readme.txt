                               Dmenu 0.5
                               =========

Dmenu is a simple menu system for Dingux (Linux on Dingoo A320). It resembles
the original XMB-like menu comes with the stock Dingoo A320 firmware.


Installation
------------
All files of Dmenu need to go to /usr/local/dmenu. To start dmenu on system
start, use a shell script as /usr/local/sbin/main to start dmenu, or copy dmenu
to /usr/local/sbin/main.


Keys
----
In main menu,

left, right key - select menu
left, B key - close SubMenu if you opened
up, down key - select menu item
A key - run the selected menu item / open SubMenu
Select key - exit dmenu
R key - SoundVolume +5%
L key - SoundVolume -5%
X key - Brightness level up
Y key - Brightness level down

In file selector menu,

up, down key - select file
Y + up/down key - page up/down
left - change to the parent directory
right - go to the selected directory
A key - run the selected file or go to the selected directory
B key - exit file selector menu
R key - page down
L key - page up

In color selector menu,

up, down key - select R/G/B
left - reduce selected color value
right - increase selected color value
A key - set color and exit color selector menu
B key - exit color selector menu


Theme support
-------------
Dmenu on startup reads /usr/local/dmenu/main.cfg, which contains a line to 
specify the theme in the form of "Theme = <theme name>".

Dmenu reads /usr/local/dmenu/themes/<theme name>/theme.cfg to load the theme.

Check theme 'default' to find out what are the options available in theme config
file.

Due to miniSD file system corruption bug in Dingux, theme selection within Dmenu
is disabled by default. To enable theme selection, add

        AllowDynamicThemeChange = true
        (You can use the keyword "yes" as same as "true")

to main.cfg, and add below MenuItem to one of the menu,

        MenuItem ThemeSelect
        {
            Icon = <icon file name>
            Name = "Select Theme"
            Executable = "!themeselect"
        }

Once you select this MenuItem in Dmenu, it will display a list of themes 
available and allow you to switch the theme.

******************************************************************************* 
A word of caution, this will update main.cfg with the new theme name you have 
selected. There is a possibility of file system corruption. Use it at your own
risk.

The recommended method of changing theme is manually edit main.cfg outside
Dingux, until the bug is fixed.
*******************************************************************************


Theme config file includes
--------------------------
Theme config file can include other config files. See theme.cfg in the sample 
theme called 'include_sample'.


SearchPath
----------
Dmenu is able to search additional path for config files. Use SearchPath option 
in main.cfg to specify the path to search. Applications can provide its own 
dmenu config file (must be named dmenu.cfg) in the following format,

Menu <Menu name>
{
    MenuItem <MenuItem name>
    {
        Icon = <Path to icon file>
        Name = <Progam name>
        Executable = <Path to executable>
        WorkDir = <Path to work directory>
    }
}

After Dmenu read this file, it will add the MenuItem into the Menu specified by 
<Menu name>.

For example, with the following directory structure,

<Some Dir>
|
 - App1
|  |
|   - dmenu.cfg
|
 - App2
|  |
|   - dmenu.cfg
.
.
.

And "SearchDir = <Some Dir>" is specified in main.cfg, Dmenu will read all 
dmenu.cfg files in subdirectories of <Some Dir>.

To specify multiple search directories, use the following format,

SearchDir = {"search dir 1", "search dir 2", etc}


In the individual dmenu.cfg files, if the icon file path is relative (i.e. the 
first char is not '/'), Dmenu will treat this as relative to the directory where
this dmenu.cfg is in.

Take a look at <dmenu install dir>/sample_search_dir to see how this works.



SubMenu
-------
To add a submenu, just add a menuitem without executable and workpath, but with
SubMenuItems. For example,

    MenuItem SubMenuTest
    {
        Icon = "res/game1.png"
        Name = "SubMenuTest"

        SubMenuItem SubTest1
        {
            Icon = "res/game2.png"
            Name = "SubTest1"
            Executable = "./duh"
            WorkDir = "/usr/local/duh"
        }
        SubMenuItem SubTest2
        {
            Icon = "res/game2.png"
            Name = "SubTest2"
            Executable = "./duh"
            WorkDir = "/usr/local/duh"
        }
    }

Once the submenu is selected, press A to open it, and press B to close. Use 
up/down to select the submenu items and press A to launch.

One caveat though, dmenu.cfg included from SearchPath does not support submenu.
dmenu will print a warning and ignore the menuitem if it's a submenu.



Selector and SelectorDir
------------------------
In MenuItem and SubMenuItem, you can use Selector and SelectorDir to get into
the filelist. For example,

    MenuItem foo
    {
        Icon = "res/emu1.png"
        Name = "SomeEmulator"
        Executable = "./emu"
        WorkDir = "/usr/local/emulator/emu"
        Selector = "true"
        SelectorDir = "/usr/local/data/roms"
    }

The filelist current folder is set to "SelectorDir".



Wallpaper Selector with thumbnail
---------------------------------
In Menu, you can make "built in Wallpaper Selector" menuitem.
For example,

    MenuItem BackgroundSelect
    {
        Icon = "res/star1.png"
        Name = "Select Wallpaper"
        Executable = "!backgroundselect"
    }



FontColor Selector
------------------
In Menu, you can make "FontColor Selector" menuitem.
For example,

    MenuItem FontColor
    {
        Icon = "res/tri4.png"
        Name = "FontColor"
        Executable = "!colorselect"
    }



UTF-8
-----
To display UTF8 characters, font file specified in theme.cfg should contain the 
necessary characters. UTF8 chars in theme.cfg name strings, and filenames in 
file selector will be displayed accordingly.



Building dmenu
--------------
There are 2 makefiles in dmenu source distribution, Makefile.host and
Makefile.dingoo.

Makefile.host will build dmenu to run on host pc running linux. This is a 
standard SDL application. You will need the SDL development libraries and
libconfuse to build.

Makefile.dingoo is the makefile for building dmenu to run in Dingux. You need
booboo's Dingux toolchain to build this. All the required libraries are already
included in the toolchain.
