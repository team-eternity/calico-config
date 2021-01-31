# Calico Configurator
Calico Configurator is the configuration utility for Calico, a port of the 
[Atari Jaguar DOOM](https://doomwiki.org/wiki/Atari_Jaguar) codebase back 
to the PC.

## How to Use
Build this project and then place the executable alongside Calico in the same
directory. Command-line arguments passed to this program will be forwarded on
to Calico when it is launched. Both calico.cfg and eeprom.cal are loaded and
manipulated by this program from the same location where Calico expects to 
read and will save them.

## Credits
* Programming : James Haley, Simon Howard

## License

Code from chocolate-setup is available under the GNU General Public License. A
copy of this license is provided in the COPYING.txt file.

All code from Calico is available under the MIT license. A copy of this license
is provided in the COPYING-CALICO.txt file.

## Known Issues
This program is currently in beta so some things are not finished or have known
issues:
* There is no support for music yet, though the volume setting can be configured.
* There is no support for multiplayer yet.
* You can only pick between IWAD types (WAD or ROM file), and not yet actually choose the file on disk.
