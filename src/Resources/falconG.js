// Copyright  2008 András Sólyom (alias Andreas Falco)
// email:   solyom at andreasfalco dot com, andreasfalco at gmail dot com). 

var showDesc = 0;

window.addEventListener("resize", ResizeThumbs);

function DebugProperties(className, obj)
{
    let style= getComputedStyle(obj)
}


function SetPropertyForSelector(selector, propertyName, propValue) 
{
    var x, i, r, id = '';
    try 
    {
        if((i=selector.indexOf('#')) >= 0) // then a single element with id
        {
            id = selector.substring(i+1)
            selector = selector.substring(0,i);
            x = document.getElementById(id);
			
            if (propValue == '')  
                x.style.removeProperty(propertyName)
            else
				x.style.setProperty(propertyName, propValue);
        }
		else
		{
			x = document.getElementsByClassName(selector)
			if(x.length != 0) 
			{	
				if (propValue == '' && propertyName in x[0].style) 
				{ 
					i = -9999
					for (i = 0; i < x.length; ++i)
					{
						if(typeof x[i] !== "undefined")
						{
							r = x[i].style.removeProperty(propertyName);
						}
					}
				}
				else 
				{
					i = -8000
					if (propertyName in x[0].style)
					{
						r = x[0].style.getPropertyValue(propertyName)   // previous value (for debugging)
						for (i = 0; i < x.length; ++i)
						{
							if(typeof x[i] !== "undefined")
								x[i].style.setProperty(propertyName, propValue);
						}
					}        
					else
						r = "No '" + propertyName + "' in class '" + selector + "'"
				}
			}
        }

    }
    catch (err) {
        console.log('*** EXCEPTION at #' + i + " " + selector + id + '.' + propertyName + ':' + propValue + " err.: '" + err + "'")
    }
}

function _resizeThumbs(name)
{
    const thumbs = document.querySelectorAll(name);
    thumbs.forEach(thumb => {
        var w = thumb.getAttribute('w'),
            h = thumb.getAttribute('h');
        var aspect = w / h;
    
        if (w > wh.width) {
            w = wh.width
            h = (wh.width / aspect)
        }
        if (h > wh.height) {
            w = wh.height
            h = (wh.height * aspect)
        }
        thumb.style.setProperty('width', w + 'px'); 
        thumb.style.setProperty('height', h + "px");
    })
}
function ResizeThumbs() {
    var wh = PageWidthHeight();
    _resizeThumbs("thumb");
    _resizeThumbs("athumb");

}

function ShowHide() {
    var d = document.getElementsByClassName("desc");
    for (var i = 0; i < d.length; ++i) {
        //    alert("showDesc ="+showDesc+", d.length="+ d.length +"\nI= "+i);
        d[i].style.display = (showDesc == 0 ? "none" : "block");
    }
    showDesc ^= 1;
    sessionStorage.setItem("showDescription", showDesc);
}

// ********************* SCROLL BACK TO POSITION *************

function PageWidthHeight() {
    var body = document.body,
        html = document.documentElement;

    var h = Math.max(body.scrollHeight, body.offsetHeight,
        html.clientHeight, html.scrollHeight, html.offsetHeight);
    var w = Math.max(body.scrollWidth, body.offsetWidth,
        html.clientWidth, html.scrollWidth, html.offsetWidth);
    return { height: h, width: w }
}

// when link uses onbeforeunload

function BeforeUnload() {

    sessionStorage.setItem(window.location.href, document.documentElement.scrollTop || document.body.scrollTop);
}

const imgOptions = {
    treshold: 0,
    rootMargin: "0px 0px 500px 0px"
};
//****************************************************
function preloadImage(img) {
    const src = img.getAttribute("data-src");
    if (!src) { return; }

    img.src = src;
    img.removeAttribute("data-src");
}

function falconGLoad(latest) {
    ResizeThumbs();
    showDesc = sessionStorage.getItem("showDescription");
    if (!showDesc)       // e.g. not defined
        showDesc = 0;
    else
        showDesc ^= 1;  // invert stored

	if(latest === 1)
		SetRandomLastImage()	// in 'latest.js'
    // console.log("showDesc=" + showDesc)    
    ShowHide(showDesc);
    t1 = Date.time;
    const images = document.querySelectorAll("[data-src]");
    cnt = 0;

    const imgObserver =
        new IntersectionObserver((entries, imgObserver) => {
            entries.forEach(entry => {
                if (!entry.isIntersecting) { return; }
                else {
                    preloadImage(entry.target);
                    imgObserver.unobserve(entry.target);
                }
            })
        }, imgOptions);

    images.forEach(image => {
        imgObserver.observe(image);
    })


    var pos = sessionStorage.getItem(window.location.href);
    if ( typeof pos !== "undefined" && pos > 0)
        document.body.scrollTo(0, pos);
}

// ********************************************************* Show lightbox

function FadeInOut(fadeIn) {
    var elem = document.getElementById('lightbox');

    var op, sop, eop, dop;
    if (fadeIn) {
        sop = 0;    // start opacity
        eop = 1;    // end opacity
        dop = 0.1   // delta opacity
    }
    else {
        sop = 1;    // start opacity
        eop = 0;    // end opacity
        dop = -0.1   // delta opacity
    }
    op = sop;       // opacity
    elem.style.opacity = sop;
	elem.style.display = "block";
	
	console.log(elem.style.display);

    var intv = setInterval(showlb, 100); // timer id
    function showlb() {
        if (Math.abs(op - eop) > 0.01) {
            op += dop;
            elem.style.opacity = op;
        }
        else {
            clearInterval(intv);
            elem.style.opacity = eop;
            if (!fadeIn)
                elem.style.display = "none"
        }
    }
}
function LightboxFadeIn() {
    if (document.getElementById('lightbox').opacity == 1)
        return;
    FadeInOut(true)
}
function LightboxFadeOut() {
    if (document.getElementById('lightbox').opacity < 1)
        return;
    FadeInOut(false)
}

function LoadAlbum(album) {
    window.location.href=album;
}

function ShowImage(img, caption) {
    const vw = Math.max(document.documentElement.clientWidth || 0, window.innerWidth || 0);
    const vh = Math.max(document.documentElement.clientHeight || 0, window.innerHeight || 0);

    var image = document.getElementById("lightbox-img");
    image.onload = function () {
        var iw = this.width;
        var ih = this.height;
        if (iw <= vw && ih <= vh) {
            this.style.position = 'static';
        }
        else {
            this.style.position = 'absolute';
            if (iw > vw)
                this.style.left = 0;
            if (ih > vh)
                this.style.top = 0;
        }
    }
    image.setAttribute('src', img);
	
    LightboxFadeIn();
}
