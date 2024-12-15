// Copyright  2008 András Sólyom (alias Andreas Falco)
// email:   solyom at andreasfalco dot com, andreasfalco at gmail dot com). 

// variables
//const imd='%1%2';"		// image dir
//					 "thd='%3%4',"		// thumbnail dir thumbs/
//					 "rsd='%5%6',"		// resource dir
//					 "ald='%7%8',"		// albumdir - only used for root level
//					 "alb='%9%10',		// base name
//                   "lng=%11;"        // language abbreviation like 'en' or 'hu'
// const imgs -array for images
// const vids -array for videos
// must be set before falconG.js is included

var showDesc = 0;
var lastIndex = -1;
var touchStartX = 0, touchStartY = 0;
var lightboxContainer = false;

window.addEventListener("resize", ResizeThumbs);

function DebugProperties(className, obj)
{
    let style= getComputedStyle(obj)
}

function DecodeText(text)
{
    text = text.replace(//g,'<br>');
    text = text.replace(//g,'\'');
    text = text.replace(//g,'"');
    text = text.replace(//g,'\\');
	return text;
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
// ********************************************************* Show lightbox

function tstrFunc(event) {
        touchStartX = event.changedTouches[0].screenX;
}
function tstpFunc(event) {
        touchEndX = event.changedTouches[0].screenX;
        const swipeDistance = touchEndX - touchStartX;
        if (swipeDistance > 50) moveToPreviousImage(); // Swipe right
        if (swipeDistance < -50) moveToNextImage();   // Swipe left
    };

function keyFunc(event) {
        if (event.key === 'Escape') LightboxFadeOut();
        if (event.key === 'ArrowLeft') PrevImage();
        if (event.key === 'ArrowRight') NextImage();
}

function InitLightbox()
{
	if(typeof lightboxContainer == 'undefined') 
		return;

	lightboxContainer.addEventListener('touchstart', tstrFunc);
	lightboxContainer.addEventListener('touchend', tstpFunc);
	document.body.classList.add('no-scroll'); // Disable background scrolling
	document.addEventListener('keydown',keyFunc)
}

function StopLightbox()
{
    if(typeof lightboxContainer=='undefined')
        return;
    lightboxContainer.removeEventListener('touchstart', tstrFunc);
    lightboxContainer.removeEventListener('touchend', tstpFunc);
	document.body.classList.remove('no-scroll'); // Re-enable scrolling
    document.removeEventListener('keydown', keyFunc);
}

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
	
    //	console.log(elem.style.display);
    
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
    if(fadeIn)
        InitLightbox();
    else
        StopLightbox();
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

function ImageForIndex(index)
{
    lastIndex = index;
    let id= imgs[index];
    if(typeof id == 'undefined')
        return "undefined";
    //console.log('ImageForIndex(',index, "):"+`${imd}${id.i}`+".jpg'");
    return `${imd}${id.i}`+".jpg";
}

function ShowImage(index, caption) {
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
    var img = ImageForIndex(index);
//    console.log('img:',img);
    image.setAttribute('src', img);
	
    LightboxFadeIn();
}

function NextImage()
{
    if(++lastIndex >= icnt)
        lastIndex = 0;
    var image = document.getElementById("lightbox-img");
    image.setAttribute('src', ImageForIndex(lastIndex));
//    console.log('NextIndex:', lastIndex);
}
function PrevImage()
{
    if(lastIndex ==0)
        lastIndex = icnt-1;
    else
    --lastIndex;
    var image = document.getElementById("lightbox-img");
    image.setAttribute('src', ImageForIndex(lastIndex));
//    console.log('PrevIndex:', lastIndex);
}



// Loop through the 'arr' array and create the HTML structure for each image
// for images and videos the array contains records with fiels 'i' - ID, 'w' - thumbnail width, 'h': thumbnail height, 't' - title, 'd' - description
/*
<div class = "img-container">
    <div class="imatte">
        <div id="3305654134" >
            <img w=600 h=400 class="thumb" src="../thumbs/3305654134.jpg" alt="Látkép" onclick="javascript:ShowImage('../imgs/3305654134.jpg', 'Látkép')" >
        </div>
    </div>
    <div class="desc">
        <p class='desc' lang='hu'>Ez a látvány tárul elénk a magasból egy kb 350mm-es teleobjektíven keresztül a világ legnagyobb jégbarlangja a Werfeni Jégwilág bejáratának közeléből</p>
    </div>

    <div class="links">
        <div class="title" onclick="javascript:ShowImage('../imgs/3305654134.jpg', 'Látkép')">Látkép</div>
    </div>
</div>
...
*/
// for albums an additional field  'l' - thumbnail ID is used
/*
    <div class = "img-container">
        <div class="amatte">
            <div id="85945584" >
                <img w=593 h=400 class="athumb" data-src="../thumbs/85945584.jpg" alt="Salzburg -2004" onclick="javascript:LoadAlbum('album801908683_en.html')" >
            </div>
        </div>
        <div class="desc">
            <p class='desc' lang='en'>Salzburg is a wonderful city full of interesting  people and buildings</p>
        </div>

        <div class="links">
            <div class="title" onclick="javascript:LoadAlbum('album801908683_en.html')">Salzburg -2004</div>
        </div>
    </div>
...
 */
function PrepareSection(lang, arr, elemID, forAlbum, isRootLevel) // example PrepareSection('hu',imgs,'images-section',false,0)
{
    let _ald = isRootLevel ? "" : ald;       // no album path unless for index_XX.html
    // Get the parent container where the images will be added
    const galleryContainer = document.getElementById(elemID);  // elemID MUST exist
    if(typeof galleryContainer == 'undefined' )//|| galleryContainer.length == 0)
        return;
    // console.log("galleryContainer", galleryContainer.length);
//    console.log('Prep.: isRootL.: ', isRootLevel, 'ald: ', ald, ', _ald: ',_ald);

    arr.forEach(
        (item, index) => {
        // Create the main img-container div
        const imgContainer = document.createElement("div"); // <div class='img-container'></div>
        imgContainer.className = "img-container";

        // Create the imatte div
        const imatte = document.createElement("div");       //  <div class='imatte'></div>
        imatte.className = "imatte";

        // Create the inner div with the ID
        const innerDiv = document.createElement("div");     //          <div id='1234'></div>  will be added as child to imatte
        innerDiv.id = item.i;

        const sTitle = item.t != '' ? DecodeText(item.t) : "&nbsp;";       // image title if any

        const onclk = forAlbum ? "javascript:LoadAlbum('"+`${_ald}${alb}${item.i}_${lang}.html`+"')" : 
                                "javascript:ShowImage("+index+",'" + sTitle + "')";
        const thumbnail = forAlbum? `${thd}${item.l}` : `${thd}${item.i}`

        // Create the img element
        const imgElement = document.createElement("img");  // <img w:600 h:400 class="thumb" src='../thumbs/123456.jpg' onclick=javascript:ShowImage('../imgs/3305654134.jpg', 'Látkép')"
        imgElement.setAttribute("w", item.w);
        imgElement.setAttribute("h", item.h);
        imgElement.setAttribute("x", `${item.index}`)
        imgElement.className = "thumb";
        imgElement.src = `${thumbnail}`+".jpg";
        imgElement.alt = "&nbsp;";
        imgElement.setAttribute("onclick", onclk);

        // Append the img element to the inner div
        innerDiv.appendChild(imgElement);

        // Append the inner div to the imatte div
        imatte.appendChild(innerDiv);

        // create the description
        if(item.d)
        {
            const descDiv = document.createElement("div");
            descDiv.className = "desc";
            const descPar = document.createElement("p")
            descPar.className = "desc";
            descPar.setAttribute("lang",`${lang}`);
            descPar.innerHTML = DecodeText(item.d);
            descDiv.appendChild(descPar);
        }

        // Create the links div
        const linksDiv = document.createElement("div");
        linksDiv.className = "links";

        // Create the title div
        const titleDiv = document.createElement("div");
        titleDiv.className = "title";
        titleDiv.setAttribute("onclick", onclk);
        titleDiv.innerHTML = sTitle; 

        // Append the title div to the links div
        linksDiv.appendChild(titleDiv);

        // Append imatte and links to the main img-container
        imgContainer.appendChild(imatte);
        imgContainer.appendChild(linksDiv);

        // Append the img-container to the gallery container
        galleryContainer.appendChild(imgContainer);
    })
};
function SetRandomLastImage()
{
	let elem = document.getElementById('latest');
	if(typeof elem === 'undefined' || !elem)
		return;
	let i = Math.floor(Math.random()*ids.length); // index:0..# of found items
	elem.src= thd + ids[i].i + '.jpg';
	//console.log("thumbsPath:"+thd + "i: "+i+"\nsrc: "+elem.src)
}

function falconGLoad(latest,isRootLevel=false) {
    ResizeThumbs();
    showDesc = sessionStorage.getItem("showDescription");
    if (!showDesc)       // e.g. not defined
        showDesc = 0;
    else
        showDesc ^= 1;  // invert stored
    if(latest === 1)	// them ids is set in latest_XX.js
	{
		imgs = [];              // empty previous array
		if(cnt > ids.length)
			cnt = ids.length;

// DEBUG
		let maxiter = 500

		let remaining = cnt;    // # of images to randomly select for display
	
		while(remaining) {
// debug		
			if(--maxiter ==  0)
			{
				alert("maxiter timeout");
				break;
			}
		
			let i = Math.floor(Math.random()*ids.length); // index:0..# of found items
			if(!imgs.includes(ids[i]))
			{
		// DEBUG
		//console.log("i: "+ i +", remaining: "+remaining);
				imgs.push(ids[i]);
				--remaining;
			}
		}
	}
	else
		SetRandomLastImage();

    lightboxContainer = document.getElementById('lb-container');

    // console.log("showDesc=" + showDesc)    
    ShowHide(showDesc);
    //	console.log('*****PrepareSection load started')
    if(typeof imgs != 'undefined')
        PrepareSection(lng,imgs,'images-section',0, isRootLevel);
    if(latest===0)
    {
        if(typeof vids != 'undefined')
            PrepareSection(lng,vids,'videos-section',1, isRootLevel);
        if(typeof albs != 'undefined')
            PrepareSection(lng,albs,'albums-section',2, isRootLevel);
    //	console.log('*****PrepareSection load finished')
    }

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

