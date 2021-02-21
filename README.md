# falconG
Hierarchical, multi language, multi platform photo gallery creator
    written in C++ and Qt 5

**falconG** is a graphical program which can be compiled and 
run on any operating system (Windows, OS-X, Linux, etc) where 
Qt is running to create WEB galleries. 

## Screenshots
![image](https://user-images.githubusercontent.com/37068759/108631608-e444c680-746a-11eb-87ff-e7c7b14d1bc7.png)


It is easy to use and completely free!

## Features of Galleries Generated by **falconG**:

 - a gallery can be embedded into any WEB page
 - a gallery may contain any number of albums
 - each album may contain any number of images, videos and sub-albums
 - image and album titles and descriptions can appear in any 
    number of languages of which only a single one is used 
    for any the page
 - languages can be changed by clicking/tapping on their names
   on the WEB page
 - language selection is stored in the browser without using
   cookies
 - the generated galleries are *responsive*, they work on any 
   device (PCs, phones and tablets)
 - colors, fonts, etc are customizable with WYSIWYG album editor
 - each album has its own web address therefore can be referred
   to or shared on social media
 - the same image can appear in any number of albums with the same
   title and description without duplicating the files
 - Optional Google analytics code for the whole gallery and 
   Facebook like and share buttons
 - a watermark may be embedded into the images
 - bandwidth friendly albums: separate, smaller thumbnail 
   images are used. Only visible thumbnails are downloaded.
 - clicking on thumbnails shows larger image
 - it is possible to not resize certain images (e.g. panoramas)

## Look
 - separate light and dark modes
 - multi tab design
 - HTML WYSIWYG display in designer

## Album Generation

 - single click generation of whole gallery
 - unchanged 
 - gallery structure is stored in a single, human readable (UTF-8) file,
   style directory and font information in another ".ini" file
 - source image files from which the gallery is generated
   can be anywhere on the system
 - WYSIWYG editor for album design
 - Watermark editor
 - reorganize albums with drag & drop (not yet working)
 - simultaneous dual language display of image titles and 
   descriptions in editor for easy translation of texts.

## TODO:
  - image, texts and albums reorganization with D & D
  - drag and drop of source images and albums
  - capability to set different titles and texts to the 
    same album or image in different albums
  - automatically generate a special gallery of randomly 
    selected images from the ones uploaded last

 
See example gallery at https://andreasfalco.com

*This is a Work In Progress*.
All features not marked with 'TODO' are working. However 
to add new albums and images you need to edit the
generated gallery structure (\*.struct) file with any UTF8
capable ASCII editor.
Modifying image and album titles and descriptions or adding
new translations for these, however can be done in the program
