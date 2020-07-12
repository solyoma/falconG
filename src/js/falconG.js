// Copyright  2008 András Sólyom (alias Andreas Falco)
// email:   solyom at andreasfalco dot com, andreasfalco at gmail dot com). 


var showDesc = 1;

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
//****************************************************
function preloadImage(img) {
    const src=img.getAttribute("data-src");
    if(!src) { return; }
    
    img.src = src;
    img.removeAttribute("data-src");
}

const imgOptions = {
    treshold: 0,
    rootMargin: "0px 0px -10px 0px"
};

function BeforeUnload()
{
//    console.log( "From page '"+ window.location.href + "' y = " + (document.documentElement.scrollTop || document.body.scrollTop));
    
    sessionStorage.setItem(window.location.href, document.documentElement.scrollTop || document.body.scrollTop); 
}

function falconGLoad()
{
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
    
    pos  = sessionStorage.getItem(window.location.href);
    if( pos > 0)
    { 
//        console.log( "scroll to y= " + pos);
        setTimeout(function() {
                                document.body.scrollTo(0, pos); 
//                                console.log( "scrolled to y= " + (document.documentElement.scrollTop || document.body.scrollTop) + "\n" );
        }, 20);
    }
    
}

// Show lightbox

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
