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
This utility will scan all installed Windowblinds skins on a machine,
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

__fidelity.calcs__ - Calculate periodic interest earned on my Fidelity fixed annuities.  
Last Update: __February 10, 2023__

__fund_history__ - do some computations on Fidelity Purchase History page
Last Update: __December 12, 2024__

```
seek FFRHX in Portfolio Positions.htm
Found FFRHX
Line 2136, length: 28254: Found [Acquired]
found start of data table
ready to start scanning data elements
data row: 1
   0: Nov-29-2024</td
   1: Short</td
   2: -$1.71</td
   3: -0.41%</td
   4: $417.15</td
   5: 44.663</td
   6: $9.38</td
   7: $418.86</td
[...]
data row: 35
   0: Apr-21-2022</td
   1: Long</td
   2: -$223.63</td
   3: -0.41%</td
   4: $54,803.59</td
   5: 5,867.622</td
   6: $9.38</td
   7: $55,027.22</td
data row: 36
data rows: 36, end of table found
max line length: 795642 chars
Current value:    66859.32
Cost basis total: 67132.14
```
