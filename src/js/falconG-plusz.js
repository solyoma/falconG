// Copyright  2008 András Sólyom (alias Andreas Falco)
// email:   solyom at andreasfalco dot com, andreasfalco at gmail dot com). 


var showDesc = 1;


window.addEventListener("resize", ResizeThumbs)

function ResizeThumbs()
{
    const thumbs = document.querySelectorAll(".thumb");
    var wh = PageWidthHeight();
 // DEBUG
 //alert(wh.width + " x " + wh.height);
    thumbs.forEach( thumb => {
            var w = thumb.getAttribute('w'), 
                h = thumb.getAttribute('h');
            var aspect = w / h;
            
            thumb.style.width = w + "px";       // default
            thumb.style.height = h + "px";
            
            console.log("window: " + wh.width + " x " + wh.height + ", thumb:" + w + " x " + h + ", aspect:"+aspect);
            if(w > wh.width)
            {
                console.log("width > thumb width!");
                thumb.style.width = wh.width + "px"
                thumb.style.height = (wh.width / aspect)  + "px" 
            }
            if(h > wh.height)
            {
                console.log("height > thumb height!");
                thumb.style.height = wh.height + "px"
                thumb.style.width = (wh.height * aspect)  + "px" 
            }
            console.log("   -> thumb:" + thumb.style.width + " x " + thumb.style.height + " <=");
            console.log("   ->    height:" + thumb.style.height + " <=");
    })
    
}

// var t1,t2,t3;

// document.onreadystatechanged = function() {     

        // if (document.readyState == "interactive") 
            // t2 = Date.time;
        // if (document.readyState == "complete") 
        // {
            // t3 = Date.time;
            // alert("onload: " +t1 + "sec\n"+
                    // "interactive: " +t2 + "sec\n"+
                    // "complete: " +t3 + "sec\n");
        // }
// }

function ShowHide()
{
	var d = document.getElementsByClassName("desc");
	for(var i = 0; i < d.length; ++i)
	{
	//    alert("showDesc ="+showDesc+", d.length="+ d.length +"\nI= "+i);
		d[i].style.display = (showDesc == 0 ? "none" : "block");
	}
	showDesc ^= 1;
}

//from http://www.quirksmode.org/js/cookies.html
// falco_ is added

function falco_createCookie(name,value,days) {
	if (days) {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();
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

// When link is JavaScript

function FGOffset(el) {
    // Get the current top/left coordinates of an element relative to the document.
    // returns object with left and top
    // The result of this helpers is equal to jQuery's $.offset(). getBoundingClientRect() 
    // from https://plainjs.com/javascript/styles/get-the-position-of-an-element-relative-to-the-document-24/
    var rect = el.getBoundingClientRect(),
    scrollLeft = window.pageXOffset || document.documentElement.scrollLeft,
    scrollTop = window.pageYOffset || document.documentElement.scrollTop;
    return { top: rect.top + scrollTop, left: rect.left + scrollLeft }
}

function FGLink(link,id)   // when the user clicks a link
{
    // < hraf="javascript::FGLink(link, aid)" id="aid">link text</a>
    var src = window.location.href.split('#');
//    console.log(src);
    sessionStorage.setItem(src[0], id); 
    console.log("Jump to " + link + ", from page "+ src[0] + ", element ID: " + id);
    window.location.href=link
}

function FGJumpBack()
{
    var id = sessionStorage.getItem(window.location.href);
    if(id != "undefined" && id != null)
    {
        var rect = FGOffset(document.getElementById(id));
        document.body.scrollTo(rect.left, rect.top-50);
//        window.location.hash = id;
        console.log("Jump to ID = '"+id+"' at (x,y)=("+rect.left +", "+ rect.top+ ") on page '" + window.location.href + "'")        
    }
}

// when link uses onbeforeunload

function BeforeUnload()
{
    console.log( "From page '"+ window.location.href + "' (width, height):" + PageWidthHeight().width + ", " + PageWidthHeight().height + " pos=" +(document.documentElement.scrollTop || document.body.scrollTop));
    
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
//    t1 = Date.time;
    ResizeThumbs();
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
    

// when scroll by onbeforeunload
    
    var pos = sessionStorage.getItem(window.location.href);
    if(pos !== undefined && pos > 0)
        document.body.scrollTo(0, pos);
    
// when scroll by explicit javascript link    
//    FGJumpBack();
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
