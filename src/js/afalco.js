// Copyright  2009 - 2017 Andreas Falco (AndreasFalco at gmail dot com). Shawn Olson
// Javascript code to the matrix skin of jalbum Ver 0.0
// Set your top level matrix album directory in DoLang() before using this
// code

var lang = "en";

function SetTextsForLanguage()
{
			 
		lang=falco_readCookie("falco_lang");
		var which = lang == "hu" ? 1 : 0;
//alert("lang = "+ lang + "\nwhich = "+which);		
		if(which === 1)
		{
		   changecss(".lang-en", "display", "none"); 
		   changecss(".lang-hu", "display","inline");
		}
		var texts = [
				["come with me", "to a journey", "around the world", "or my country",   "Through reality",    "and fantasy",  "you travel"],
				["Gyere velem!",  "Utazzunk",    "a világban",       "és itthon!",      "Utazásunk",   "képzelt és valós", "tájakra visz."],
				["i1","i2","i3","i4","i5","i6","i7",]
		   ];
//alert("Texts = " +texts);		   
		for(var i = 0; i < 7; i++)
		{
			var txti = texts[2][i];
			var elem = document.getElementById(txti);
			// var elem = document.getElementById(texts[2][i]);
			
			var txt = texts[which][i];
			elem.innerHTML = txt; // texts[which][i];
			//alert("i:"+i + ", which="+which+", texts[which][i]="+texts[which][i]);
		}
		 

}

function SetLang(language)  // lang = 'hu' or 'en'
{
 if(language === 'hu' || language === 'en')
 {
	lang = language;
	falco_createCookie('falco_lang',language, 90);
 }
}
function DoLink(which, addr)
{
	SetLang(which);
	document.location.href=addr;
}

function JumpTo(addr)
{
	SetLang(lang);
	document.location.href=addr;
}

function DoLang(which)
{
  SetLang(which);
  document.location.href="album_turtle/index.html";
}

function JumpToAll(href)
{
  SetLang(lang);
  document.location.href="album_turtle/index.html";
}

function HideUp()
{
  if( document.location.href.indexOf("noup=1") != -1) // if present hide breadcrumbs, up arrow and sub folders
  {
    var a = document.getElementsByClassName("parent");
 	a[0].style.display = "none";
	a = document.getElementsByClassName("breadcrumb"); // NodeList array
	if(a.length > 0)
        a[0].style.display = "none";
	a = document.getElementsByClassName("folders");
//	if(a.length == 0)
//       	 alert("nincs folders");
	for(i = 0;  i < a.length; ++i)
	  a[i].style.display = "none";
  }
}


// --------- other's code follows --------------

 //Last Updated on June 23, 2009
 //documentation for this script at
 //http://www.shawnolson.net/a/503/altering-css-class-attributes-with-javascript.html
 
 // Example:
 //  <span class="exampleA">Example A</span> <span class="exampleB">Example B</span> 
 //  <span class="exampleA">Example A</span> <input type="button" value="Change A Red" onclick="changecss('.exampleA','color','red')"/>
 //  <input type="button" value="Change A Black" onclick="changecss('.exampleA','color','black')"/>
 function changecss(theClass,element,value) 
 {
   var cssRules;

   var added = false;
   for (var S = 0; S < document.styleSheets.length; S++)
   {
     if (document.styleSheets[S]['rules']) 
     {
        cssRules = 'rules';
     } 
     else if (document.styleSheets[S]['cssRules']) 
     {
        cssRules = 'cssRules';
     } 
     else 
     {
 //no rules found... browser unknown
     }

     for (var R = 0; R < document.styleSheets[S][cssRules].length; R++) 
     {
       if (document.styleSheets[S][cssRules][R].selectorText == theClass) 
       {
         if(document.styleSheets[S][cssRules][R].style[element])
         {
            document.styleSheets[S][cssRules][R].style[element] = value;
            added=true;
            break;
         }
       }
     }
     if(!added)
     {
       if(document.styleSheets[S].insertRule)
       {
		try {
          document.styleSheets[S].insertRule(theClass+' { '+element+': '+value+'; }',document.styleSheets[S][cssRules].length);
		 }
		 catch(err)
		 {
			// alert(theClass+' { '+element+': '+value+'; }');
		 }
       } 
       else if (document.styleSheets[S].addRule) 
       {
          document.styleSheets[S].addRule(theClass,element+': '+value+';');
       }
     }
   }
 }

//from http://www.quirksmode.org/js/cookies.html
// falco_ is added

function falco_createCookie(name,value,days) {
	if (days) {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toUTCString();
	}
	else var expires = "";
	document.cookie = name+"="+value+expires+"; path=/";
}

function falco_readCookie(name) {
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function falco_eraseCookie(name) {
	createCookie(name,"",-1);
}
