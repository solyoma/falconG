# ![falconG-icon](https://user-images.githubusercontent.com/37068759/136534722-d22d903b-57bf-4dc9-8618-d746848eb892.png) falconG  
# Hierarchical, multi language, multi platform photo gallery creator
    written in C++ and Qt 5

(Hirerchikus többnyelvű, platform független fotó galéria készítő
    c++ és QT 5 forrással. A magyar leírást ld. az angol alatt)

**falconG** is a graphical program which can be compiled and 
run on any operating system (Windows, OS-X, Linux, etc) where 
Qt is running and the Qt WebEngine is supported (currently
Windows, MacOS and linux) to create WEB galleries. 

## Screenshots
![image](https://user-images.githubusercontent.com/37068759/108631608-e444c680-746a-11eb-87ff-e7c7b14d1bc7.png)


It is easy to use and completely free!
Two program languages can be used: US English and Hungarian.

## Features of Galleries Generated by ***falconG***:

 - the generated galleries are *responsive*, they work on any 
   device (PCs, phones and tablets)
 - a gallery is a hierarchical system of albums, images and videos
 - a gallery can be embedded into any WEB page
 - a gallery may contain any number of albums
 - every album may contain any number of images, videos and sub-albums
 - may contain an album with random selection of images from 
   the latest upload which also displays different images at each
   visit
 - photo galleries are multi-language. Image and album 
   titles and descriptions can appear in any number of languages.
   Uses separate Web pages for each.
 - each album has its own web address therefore can be referred
   to or shared on social media. Currently a facebook share button
   may appear on each page.
 - the same image with the same title and description can appear 
   in any number of albums.
 - languages can be changed by clicking/tapping directly
   on the WEB page
 - language selection is stored in the browser without using
   cookies
 - optional Google analytics code for the whole gallery and 
   Facebook like and share buttons (they may save cookies)
 - a watermark may be embedded into the images
 - bandwidth friendly albums uses smaller thumbnail 
   images and larger images on demand (click). 
   Only thumbnails that are or were visible in the browser
   are downloaded.
 - clicking^tapping on thumbnails shows "large" images
 
## Deploy generated gallery on the WEB at the first time

 You need access to the **public_html** folder on the web server
 - Create the folder you set in **falconG** inside public_html
 - upload all of the folders and files inside the generated 
   gallery folder on your machine
   (For multi language galleries the index.html generated 
   on your machine by **falconG** is a bare-bone version
   You might want to create a better one for yourself)
   
## refresh the gallery with new albums   
 - when new images and folders are added or texts changed
	- copy the new files from folders **album**, **imgs**,
	  **thumbs**,**vids** to the same named folders on the 
	  server, plus the **latestlist\*.js** fájlokat.

## Features of ***falconG***:

 - themeable. there are separate light and dark modes (themes)
   with built-in theme editor
 - multi tab design
 - HTML based WYSIWYG editor for album design, in which
	- colors, fonts, etc. are customizable with WYSIWYG album editor
		title and description without duplicating the files
	- simultaneous dual language display of image titles and 
		descriptions in editor for easy translation of texts.
 - watermark editor
 - single click generation of whole gallery,
   unchanged albums and images are not re-processed
 - it is possible not to resize certain images (e.g. panoramas)
   but leave them at their source sizes for thw larger image display
 - gallery structure is stored in a single, human readable (UTF-8)
   text file, style directory and font information in another 
   ".ini" file
 - source image files from which the gallery is generated
   can be anywhere on the system
 - images and folders can be added to any album. If a folder
   is added its sub folders and images are also added
 - reorganize albums with drag & drop (only inside the 
   same album as of yet and not between albums)
 
## Program Compilation and Installation
 - as this is a WIP (Work In Progress, a.k.a. alpha software)
   you must compile the program yourself, However the
   compiled program can be used on computers without Qt.
   To do that you must run the tool that creates the Qt
   redistributable package. For windows this requires to run
   **windeployqt.exe**, If you used Microsoft Visual Studio
   and compiled the program as a 64 bit executable then you need 
   to use the commands in a command shell window (cmd or Powershell):

   **"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"**
   (the quotes are required and this command must only run once) followed by 
   d:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe  --no-quick-import <path to falconG.exe>
   
   
 - no installer exists at this point (TODO), so manual
   installation need to be used. The steps:
   
	- run the commands above on your sdevelopment machine 
	  if they were not yet run using the path of the release version 
	  of ***falconG*** (falconG.exe on Windows)
	- create a folder on the destination machine with
	  no Qt for the program
	- copy the file **'falconG-icon.png'** and folders **css**, **js**, 
	  **res**, into this directory  
	- copy the **.lang** files you want to use there too
	- copy everything from the Release folder to that folder on 
	  the destination machine
	- create a shortcut on the desktop of the destination machine
	  to the falconG executable
		  
	Important: *Qt WebEngine will not run from a network drive!*

## TODO (in the order of importance):
  - image, texts and albums reorganization with D & D
    between folders
  - automatic upload of generated album onto the WEB
  - drag and drop of source images and albums (now they can onluy be added
    using menu
  - mark any album in gallery to be re-processed unconditionally
    (if you change any title or description texts or add new
	folders and or images, or reorder a folder
	then that album will be re-processed)
  - installer
  - capability to set different titles and texts to the 
    same album or image in different albums

 
See example gallery at https://andreasfalco.com

*This is a Work In Progress*.
All features not marked with 'TODO' should be working. 


******************************************************************


## Magyar nyelvű leírás
-----------------------
***falconG*** egy grafikus felhasználói felületű alkalmazás, ami
bármely olyan operációs rendszeren használható, amelyiken
a QT is és a QtWebEngine is fut (Windows, OS-X, linux, stb)

## Képernyő képek

ld. fent a **Screenshots** pont alatt

Ez a nagyon könnyen használható program teljesen ingyenes
(a jelenlegi állapotról a végén olvashatsz)

A program felülete jelenleg két nyelven: magyarul és (amerikai) 
angolul használható.

## A ***falconG***-vel készített galériák tulajdonságai

 - a generált albumok minden eszközön (PC, telefon, tablet, stb)
   az illető eszköznek megfelelően jelennek meg
 - egy galéria albumok, képek és videók hierarchikus rendszere,
 - ezek beágyazhatóak bármely WEB oldalba, és
 - akárhány képet, videót és albumot tartalmazhatnak. Minden albumban
   lehetnek képek, videók és albumok is, tetszőleges mélységig.
 - lehet benne egy olyan speciális galéria, amelyben 
   megadható számú, minden betöltésnél más és más, véletlenszerűen
   kiválasztott kép jeleneik meg az utóljára feltöltöttek közül.
 - a képeknek és albumoknak címeket és leírásokat adhatunk. Ezek 
   több (akárhány) nyelven megadhatóak, és mindegyik nyelvhez egy
   külön WEB oldal generálódik.
 - minden oldalről elérhető a többi nyelvű oldal is
 - a nyelvválasztást a böngésző megjegyzi, az oldalak sütiket nem 
   használnak
 - Ugyanaz a kép akárhány albumban megjelenhet ugyanazokkal a 
   szövegekkel, és csak egy példányban tárolódik
 - mivel minden albumnak saját WEB oldala van könnyű rájuk hivatkozni
   a szociális médiumokon (facebook hivatkozási link beágyazható az oldalba)
 - Választható a Google Analytics és a facebook 'like' gomb használata az 
   albumokhoz
 - a képekbe egy vízjelet helyezhetünk el
 - a sávszélesség minimalizálására kisebb méretű képek vannak az oldalakon
   a nagyobb felbontású képek csak a kis képekre kattintásra töltődnek le
 - a **falconG** átméretezi a forrásképeket, de egyes képekkel (pl. panorámákkal)
   kivételt tehetünk. Azoknak a "nagyfelbontású" változata eredeti méretű lesz.
   
## A **falconG**-vel létrehozott galéria telepítése a szerverre  

 Hozzáféréssel kell rendelkezned a szerveren a könyvtáradhoz
 - hozd létre a mappát, amit beállítottál a programban, mint web forráskönyvtárat
 - másold be ide az összes fájlt és mappát a galéria mappájából
   (Többnyelvű galériákhoz a falconG által létrehoztt index.html egy nagyon 
    csupasz verzió. Valószínűleg egy jobbat fogsz használni.)
	
## Feltöltött galéria frissítése

 - másold fel az új fájlokat a galéria következő mappáiból:	**albums**,**imgs**,
   **thumbs**, **vids**, továbbá a **latestlist\*.js** fájlt/fájlokat a **js**
   könyvtárból az ugyanilyen nevű mappákba a szerveren

## A **falconG** tulajdonságai

 **A program által használt Qt WebEngine nem működik hálózati meghajtón!**
 
 - "Bőrök" - beépített rendszerszíneket használó, sötét, fekete és kék kinézet (séma)
   választható, de saját sémákat is definiálhatunk
 - HTML - Amit Látsz Azt Kapod WEB oldal szerkesztő 
 - az oldalakon használt színek és betükészletek szabadon 
   szerkeszthetőek az oldalak képernyőn megjelenő előképevel
 - képcímek és leírások fordításához egyszerre két nyelvű szöveget 
   mutató cím és leírás szerkesztő ad segítséget
 - egy gombnyomásos generálás, a már feldolgozott képeket nem dolgozza fel újra
 - a galéria szerkezete egy darab, ember által is olvasható és szerkeszthető UTF-8
   kódolású fájlban (**.struct**) van. A stílusok és egyéb információk 
   egy másik fájlban (**.ini**) vannak
 - a galériában szereplő képek forrása bárhol lehet, nem szükséges azokat 
   átmásolni sehová. Az eredeti képeket a program nem változatja, nem másolja
   át és ha nem akarjuk nem is törli le.
 - vízjel szerkesztő
 - képek "Húzd és Dobd" 'trendez;se egy albumban
 - képek "Húzd és Dobd" hozzáadása albumhoz (még nem működik)
 
## A lefordított **falconG** program telepítése
 - mivel ez egy fejlesztés alatt álló ("WIP - Work In Progress", másként 
   alfa szoftver) neked kell lefordítani. A lefordított program azonban 
   használható olyan gépeken is, ahol sem fordító program sem a Qt nincs
   telepítve.
 - ehhez elő kell állítani a futtató környezetet. Windows-on ehhez
   a **windeployqt** programot kell futtatni. Ha a Microsoft Visual Studio
   a fordítóprogram és a 64 bites verziót használtuk, akkor a parancsok:
   "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
   (ezt csak egyszer szabad futtatni), majd
   d:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe  --no-quick-import <path to falconG.exe>

 - nincs automatikus telepítés, ezért csak a kézi telepítés lehetséges
   a lépések:
	- futtasd a fenti parancsokat, vagy a linuxos és Mac OS-es megfelelőjüket
	  a *falconG* könyvtárába
	- másold a *falconG-icon.png* fájlt is oda
	- másold a kívánt *.lang* fájlokat is oda
	- hozz létre egy mappát a célgépen, amin nincs Qt telepítve
	- másolj át minden fájlt a falconG mappájából a célgép mappájába
	- hozz létre egy parancsikont a célgépen a **falconG** programra
	Fontos: a Qt WebEngine nem fut hálózati meghajtókról, akkor sem
			ha azok magán a helyi gépen levő mappák.

## TODO (in the order of importance):
- képek és albumok mozgatása a Fogd És Vidd módszerrel más mappákba
- bármely album megjelölése újra feldolgozáshoz
- telepítő program
- a lehetőség, hogy ugyanannak a képnek más albumokban más címe és
	  leírása lehessen
- a WEB tervező és generáló beépülő modul rendszerben működjön
	
 
Egy minta galériát találsz a  https://andreasfalco.com oldalba ágyazva.
