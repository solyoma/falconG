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

function ShowImage(image)
{
	document.location.href = "LargeImage.php?img="+image;
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

function falconGLoad() {
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
}