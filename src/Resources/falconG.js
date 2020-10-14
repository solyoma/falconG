// Copyright  2008 András Sólyom (alias Andreas Falco)
// email:   solyom at andreasfalco dot com, andreasfalco at gmail dot com). 


var showDesc = 0;

window.addEventListener("resize", ResizeThumbs)

function SetPropertyForClass(className, propertyName, propValue) 
{
        var x,i; 
        if(propertyName.substring(0, 1) == ':')       // first line height
        {
            className += propertyName;
            propertyName = 'font-size'
        }
        x = document.getElementsByClassName(className)
        for(i = 0; i < x.length; ++i) 
            x[i].style.setProperty(propertyName, propValue);
}
function ResizeThumbs()
{
    const thumbs = document.querySelectorAll(".thumb");
    var wh = PageWidthHeight();
    thumbs.forEach( thumb => {
            var w = thumb.getAttribute('w'), 
                h = thumb.getAttribute('h');
            var aspect = w / h;
            
            thumb.style.width = w + "px";       // default
            thumb.style.height = h + "px";
            
            if(w > wh.width)
            {
                thumb.style.width = wh.width + "px"
                thumb.style.height = (wh.width / aspect)  + "px" 
            }
            if(h > wh.height)
            {
                thumb.style.height = wh.height + "px"
                thumb.style.width = (wh.height * aspect)  + "px" 
            }
    })
    
}

function ShowHide()
{
	var d = document.getElementsByClassName("desc");
	for(var i = 0; i < d.length; ++i)
	{
	//    alert("showDesc ="+showDesc+", d.length="+ d.length +"\nI= "+i);
		d[i].style.display = (showDesc == 0 ? "none" : "block");
	}
	showDesc ^= 1;
    sessionStorage.setItem("showDescription", showDesc);
}

// ********************* SCROLL BACK TO POSITION *************

function PageWidthHeight()
{
    var body = document.body,
        html = document.documentElement;

    var h = Math.max( body.scrollHeight, body.offsetHeight, 
                           html.clientHeight, html.scrollHeight, html.offsetHeight );    
    var w = Math.max( body.scrollWidth, body.offsetWidth, 
                           html.clientWidth, html.scrollWidth, html.offsetWidth );    
    return { height:h, width:w }
}

// when link uses onbeforeunload

function BeforeUnload()
{
//    console.log( "From page '"+ window.location.href + "' (width, height):" + PageWidthHeight().width + ", " + PageWidthHeight().height + " pos=" +(document.documentElement.scrollTop || document.body.scrollTop));
    
    sessionStorage.setItem(window.location.href, document.documentElement.scrollTop || document.body.scrollTop); 
}

const imgOptions = {
    treshold: 0,
    rootMargin: "0px 0px -10px 0px"
};
//****************************************************
function preloadImage(img) {
    const src=img.getAttribute("data-src");
    if(!src) { return; }
    
    img.src = src;
    img.removeAttribute("data-src");
}

function falconGLoad()
{
    ResizeThumbs();
    showDesc = sessionStorage.getItem("showDescription");
    if(!showDesc)       // e.g. not defined
        showDesc = 0;       
    else
        showDesc ^= 1;  // invert stored
    
// console.log("showDesc=" + showDesc)    
    ShowHide(showDesc);
    t1 = Date.time;
    const images = document.querySelectorAll("[data-src]");
    cnt = 0;

    const imgObserver = 
        new IntersectionObserver( (entries, imgObserver) => {
            entries.forEach(entry=> {
                if(!entry.isIntersecting) { return; }
                else {
                    preloadImage(entry.target);
                    imgObserver.unobserve(entry.target);
                    //document.getElementById("counter").innerHTML=(cnt+1) + " ("+images.length + ")";
                }
            })
        }, imgOptions);
        
    images.forEach(image => {
        imgObserver.observe(image);
	})
    

    var pos = sessionStorage.getItem(window.location.href);
    if(pos !== undefined && pos > 0)
        document.body.scrollTo(0, pos);
}

// ********************************************************* Show lightbox

function FadeInOut(fadeIn)
{
    var elem = document.getElementById('lightbox');
    
    var op, sop, eop,  dop;
    if(fadeIn)
    {
        sop = 0;    // start opacity
        eop = 1;    // end opacity
        dop = 0.1   // delta opacity
    }
    else
    {
        sop = 1;    // start opacity
        eop = 0;    // end opacity
        dop = -0.1   // delta opacity
    }
    op = sop;       // opacity
    elem.style.opacity = sop;
    elem.style.display = "block";

    var intv = setInterval(showlb,100); // timer id
    function showlb()
    {
        if(Math.abs(op - eop) > 0.01)
        {    
            op += dop;
            elem.style.opacity = op;
        }
        else
        {
            clearInterval(intv);
            elem.style.opacity = eop;
            if(!fadeIn)
                elem.style.display="none"
        }
    }
}
function LightBoxFadeIn()
{
    if(document.getElementById('lightbox').opacity == 1)
        return;
    FadeInOut(true)
}
function LightBoxFadeOut()
{
    if(document.getElementById('lightbox').opacity < 1)
        return;
    FadeInOut(false)
}

function ShowImage(img, caption)
{
    const vw = Math.max(document.documentElement.clientWidth || 0, window.innerWidth || 0);
    const vh = Math.max(document.documentElement.clientHeight || 0, window.innerHeight || 0);

    var image = document.getElementById("lightbox-img");
    image.onload = function() {
                            var iw = this.width;
                            var ih = this.height;
                                if(iw <= vw && ih <= vh)
                                {
                                    this.style.position='static';
                                }
                                else
                                {
                                    this.style.position='absolute';
                                    if(iw > vw)
                                        this.style.left = 0;
                                    if(ih > vh)
                                        this.style.top = 0;
                                }
                              }
    image.setAttribute('src', img);
    
    document.getElementById("lightbox-caption").innerHTML = caption
    LightBoxFadeIn();
}

/* these must be in sync with AlbumElements in 'falconG.h' */
const ELEM = {
	AE_WEB_PAGE             : 0,
	AE_HEADER               : 1, 
	AE_MENU_BUTTONS         : 2,
	AE_LANG_BUTTON          : 3,
	AE_SMALL_GALLERY_TITLE 	: 4,
	AE_GALLERY_TITLE		: 5,
	AE_GALLERY_DESC 		: 6,
	AE_SECTION              : 7,
	AE_ALBUM_TITLE			: 8,
	AE_ALBUM_DESC			: 9,
	AE_LIGHTBOX_TITLE       : 10,
	AE_LIGHTBOX_DESCRIPTION : 11,
	AE_FOOTER				: 12
};


function PageClick()               {document.location.href = ELEM.AE_WEB_PAGE }
function HeaderClick()             {document.location.href = ELEM.AE_HEADER   }
function MenuClick()               {document.location.href = ELEM.AE_MENU_BUTTONS }
function LangClick()               {document.location.href = ELEM.AE_LANG_BUTTON }
function SmallTitleClick()         {document.location.href = ELEM.AE_SMALL_GALLERY_TITLE }
function GalleryTitleClick()       {document.location.href = ELEM.AE_GALLERY_TITLE }
function GalleryDescClick()        {document.location.href = ELEM.AE_GALLERY_DESC }
function SectionClick()            {document.location.href = ELEM.AE_SECTION }
function LinkClick()               {document.location.href = ELEM.AE_ALBUM_TITLE }
function DescClick()               {document.location.href = ELEM.AE_ALBUM_DESC }
// lightbox title (10) and description (11) only from combo box
//                                                             ELEM.AE_LIGHTBOX_TITLE      
//                                                             ELEM.AE_LIGHTBOX_DESCRIPTION
function FooterClick()             {document.location.href = ELEM.AE_FOOTER }