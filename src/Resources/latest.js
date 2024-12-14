// Expects: falconG.js and the suitable 'latestList_xx.j' are loaded 
//          before this, therefore
//          lng - short language code like "en" is set
//          ids: array of a number of latest images defined
//--------------------------------------------------------------------
function select() {
	// DEBUG
	// console.log(window.location.pathname)
	
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
    falconGLoad(1,true); // in falconG.js

/*
	const section = document.getElementsByTagName('section')[0];

    imgs.forEach(item => {
        const divImgContainer = document.createElement('div');
        divImgContainer.classList.add('img-container');
        // DEBUG
		// console.log('item:'+item)
            const divId = document.createElement('div');
            divId.id=item.id;
            divId.w= item.w;
            divId.h= item.h;
            
                const a = document.createElement('a');
                a.classList.add('thumb');
                a.href=`javascript:ShowImage('../${imagePath}${item.id}.jpg','')`;
        // DEBUG
		// console.log('href:'+a.href)
            
                const img = document.createElement('img');
                img.src = `../${thumbsPath}${item.id}.jpg`;
                img.alt= `Image ${item.id}.jpg`;
                img.classList.add('galleryImg');
            
            const divDesc = document.createElement('div');
            divDesc.classList.add('desc');
            
			let text = "&nbsp";
                const pw = document.createElement('p');
                pw.lang=lang;
				if(typeof item.d !== 'undefined')
					text = item.d;
				pw.innerHTML = text;

            const divLinks = document.createElement('div');
            divLinks.classList.add('links');
            
                const divTitle = document.createElement('div');
                divTitle.classList.add('title');
                divTitle.onclick = `javascript:ShowImage('../${imagePath}${item.id}.jpg','')`;
				text = "&nbsp";
				if(typeof item.t !== 'undefined')
					text = item.t;
				divTitle.innerHTML = text;
            
            divLinks.appendChild(divTitle);
            divDesc.appendChild(pw);
            a.appendChild(img);
            divId.appendChild(a);
        divImgContainer.appendChild(divId);
        divImgContainer.appendChild(divDesc);
        divImgContainer.appendChild(divLinks);

        section.appendChild(divImgContainer);

 //       console.log(img.src + ' - processed');
    })
 */
}
function SetRandomLastImage()
{
	let elem = document.getElementById('latest');
	if(typeof elem === 'undefined' || !elem)
		return;
	let i = Math.floor(Math.random()*ids.length); // index:0..# of found items
	elem.src= thumbsPath + ids[i].id + '.jpg';
//	console.log("thumbsPath:"+thumbsPath + "i: "+i+"\nsrc: "+elem.src)
}