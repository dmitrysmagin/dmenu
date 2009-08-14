                               Dmenu 0.3
                               =========

Dmenu is a simple menu system for Dingux (Linux on Dingoo A320). It resembles the
original XMB-like menu comes with the stock Dingoo A320 firmware.


Installation
------------
All files of Dmenu need to go to /usr/local/dmenu. To start dmenu on system start,
use a shell script as /usr/local/sbin/main to start dmenu, or copy dmenu to
/usr/local/sbin/main.


Keys
----
In main menu,

left, right key - select menu
up, down key - select menu item
A key - run the selected menu item
Select key - exit dmenu


In file selector menu,

up, down key - select file
Y + up/down key - page up/down
left - change to the parent directory
right - go to the selected directory
A key - run the selected file or go to the selected directory
B key - exit file selector menu


Theme support
-------------
Dmenu on startup reads /usr/local/dmenu/main.cfg, which contains a line to specify
the theme in the form of "Theme = <theme name>".

Dmenu reads /usr/local/dmenu/themes/<theme name>/theme.cfg to load the theme.

Check theme 'default' to find out what are the options available in theme config
file.

Due to miniSD file system corruption bug in Dingux, theme selection within Dmenu is
disabled by default. To enable theme selection, add

        AllowDynamicThemeChange = Yes

to main.cfg, and add below MenuItem to one of the menu,

        MenuItem ThemeSelect
        {
            Icon = <icon file name>
            Name = "Select Theme"
            Executable = "!themeselect"
        }

Once you select this MenuItem in Dmenu, it will display a list of themes available
and allow you to switch the theme.

************************************************************************************** 
A word of caution, this will update main.cfg with the new theme name you have selected.
There is a possibility of file system corruption. Use it at your own risk.

The recommended method of changing theme is manually edit main.cfg outside Dingux,
until the bug is fixed.
**************************************************************************************


Theme config file includes
--------------------------
Theme config file can include other config files. See theme.cfg in the sample theme
called 'include_sample'.


SearchPath
----------
Dmenu is able to search additional path for config files. Use SearchPath option in
main.cfg to specify the path to search. Applications can provide its own dmenu config
file (must be named dmenu.cfg) in the following format,

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

After Dmenu read this file, it will add the MenuItem into the Menu specified by <Menu name>.

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

And "SearchDir = <Some Dir>" is specified in main.cfg, Dmenu will read all dmenu.cfg files
in subdirectories of <Some Dir>. Currently Dmenu will only search to the subdirectories
immediately below <Some Dir>. It does not search deeper into the directory structure.

In the individual dmenu.cfg files, if the icon file path is relative (i.e. the first char is
not '/'), Dmenu will treat this as relative to the directory where this dmenu.cfg is in.

Take a look at <dmenu install dir>/sample_search_dir to see how this works.


