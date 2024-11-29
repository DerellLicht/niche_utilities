## Niche utilities

These are all small, typically command-line utilities that I have found
to be useful.  However, unlike the Snippets collection, these are all specialized,
niche utilities that I wrote to solve specific problems of my own; thus, most of these
won't be of much use to the common user.

All are compiled using the MinGW compiler package. This is an excellent,
*FREE* compiler package which is based on the GNU compiler, but links to Windows
libraries. I recommend the [TDM](http://tdm-gcc.tdragon.net/) distribution, 
to avoid certain issues with library accessibility. 

All programs here are licensed under Creative Commons CC0 1.0 Universal;  
https://creativecommons.org/publicdomain/zero/1.0/

*** 

__read_wb_skins__ - Display summary of Windowblinds skins authors/filenames
__read_wb_skins__ will display scan all installed Windowblinds skins on a machine,
and display author names sorted by number of skins from each author.
It optionally will also display the skin names for each author.
Last Update: __November 25, 2024__

``` 
D:\SourceCode\Git\niche_utilities Yes, Master?? > read_wb_skins -l
source: [C:\Users\Public\Documents\Stardock\WindowBlinds\skins.nbd]
# skins   author
=======   ==========================================
 57       Total number of installed skins
 28       don5318
          : Blue Saphire
          : Cargo Ship
          : Celtic
          : Celtic Warrior
          : Copper 2
          : Country_Christmas
          : Dark Rogue
          : Dark SteamPunk
          : Fall Color
          : Force 940
          : GreenTan
          : Ja VorCha
          : Jedi Interceptor
          : Jungle
          : Nautical Navigation
          : Nuetron II
          : Radio
          : Retro Radio
          : Rustic Metal
          : Steampunk 2023
          : Steampunk Radio
          : Steampunk Tech
          : Tech Audio
          : Viking Warrior
          : Warp Core
          : Bluenose
          : Gears
          : Fluids
```  
*** 

__heron__ - This program implements Heron's Formula,
which calculates the area of an irregular triangle,
given only the lengths of the three sides, but no angles.  
Last Update: __February 10, 2023__

``` 
Usage: heron side1_len side2_len side3_len

This program implements Heron's Formula,
which calculates the area of an irregular triangle,
given only the lengths of the three sides, but no angles.

parameters: side1: 950.000, side2: 510.000, side3: 1200.000
intermediate sum parameter: 1330.000
area: 232111.266
```   
