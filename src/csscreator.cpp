#include "csscreator.h"


bool CssCreator::_Open(QString name)
{
	_f.setFileName(name);
	if (!_f.open(QIODevice::WriteOnly))
		return false;

	_ofs.setDevice(&_f);
	return true;
}

void CssCreator::_SaveHeader()
{
	QString qs;

	if (!_fonts._qsGoogleFonts.isEmpty())
		_ofs << "@import url(\"https://fonts.googleapis.com/css?family=" + _fonts._qsGoogleFonts.replace(',', '|') + "\")";
	if (!_fonts._qsDefaultFonts.isEmpty())
	{
		if (!_fonts._qsFontDir.isEmpty() && _fonts._qsFontDir.right(1) != '/')
			qs = _fonts._qsFontDir + "/";
		_ofs << "@import url(\"https://fonts.googleapis.com/css?family=" + qs + _fonts._qsDefaultFonts.replace(',', '|') + "\")";
	}

	_ofs << 
R"(
*, *::before, *::after {
	box - sizing:border - box;"
	padding : 0;\n"
	margin : 0;\n"
}

)";
}
void CssCreator::_CssForElement(QString classStr, _CElem &elem)
{
	QString qs = elem.ForStyleSheet(true);
	if (!qs.isEmpty())
		_ofs << classStr << qs << "}\n\n";
	if (elem.font.IsFirstLineDifferent())
		_ofs << elem.font.FirstLineClassStr(elem.ClassName());
}

void CssCreator::_CreateFirstLineForElement(QString what, _CElem* pElem)
{
	_ofs << pElem->font.FirstLineClassStr(what);
}

void CssCreator::_CreateGlobals()
{
	/* color: %1, background-color:%2,  */
	_ofs << QString(R"(
html,
body,
main{
	
    color:%1;
    background-color: %2;
	height: 100% ;
	width: 100%;
	overflow: auto;
}

 a, a:visited{
	text-decoration: none;
}
)").arg(config.Web.color.Name()).arg(config.Web.background.Name());

	// ------- background image -----
	if (config.backgroundImage.v == _CBackgroundImage::hNotUsed)
		return;

	QString qsS = "auto",			// background-size
			qsR = "no-repeat";		// background-repeat

	if (config.backgroundImage.fileName.isEmpty())
		return;

	switch (config.backgroundImage.v)
	{
		case _CBackgroundImage::hAuto:		break;
		case _CBackgroundImage::hCover:		qsS = "cover"; break;
		case _CBackgroundImage::hContain:	qsS = "contain";break;
		case _CBackgroundImage::hTile:		qsR = "repeat"; break;
	}
	_ofs << QString(R"(
body {
	background-image: %1;
	background-size: %2;
	background-repeat: %2;
}

main{
	display: flex;
	flex-direction: column;
	flex-wrap:wrap;
	width: 100% ;
}

section {
	display: flex;
	flex-flow: row wrap;
	justify-content: center;
    align-items:baseline;
	max-width: 100% ;
}



)").arg(config.backgroundImage.fileName).arg(qsS).arg(qsR);

}

void CssCreator::_CreateForMenuButtons()
{
	/* %1:menu color %2:var(--menu-background), 
	   %3:var(--serif-fonts), %4: font-size*/
	_ofs << R"(
nav {
	margin:10px;
	position: sticky;
	position: -webkit-sticky;
	display: inline-block;
	margin-bottom: 1rem;
    z-index:1;
}

.menu-item {
	display: inline-block;
	margin: 2px 3px 2px 2px;
	padding: 4px 10px 3px;
)";
	QString qs = config.Menu.ForStyleSheet(true);
	if(!qs.isEmpty())
		_ofs << qs << "\n}\n"	// for menus no different first-line accepted
			<< "\n.menu-item:hover {\n"
			<< config.Menu.background.ForStyleSheet(true, true)
			<< "\n"<<	config.Menu.color.ForStyleSheet(true, false) << "\n}\n";
	_ofs << "/* up icon */\n"
		    ".menu-item#uplink {"
			"	.background-image: url(\"../res/up-icon.png" << R"(\");
	background-size:14px;
	background-repeat:no-repeat;
	background-position: center;
}
)";
}

void CssCreator::_CreateForLangButton()
{
	_CssForElement(".lang {\n",config.Lang); 
}

void CssCreator::_CreateForHeader()
{
	_CssForElement("header {\n", config.Lang);
}

void CssCreator::_CreateForSmallTitle()
{
	_CssForElement(".falconG {\n	display:inline-block;\n", config.SmallGalleryTitle);
}

void CssCreator::_CreateForGalleryTitle()
{
	_CssForElement(".gallery-title {\n", config.Lang);
}

void CssCreator::_CreateForGalleryDesc()
{
	_CssForElement(".gallery-desc {\n", config.Lang);
}

void CssCreator::_CreateForSection()
{
	_CssForElement(R"(section{
	display: flex;
	flex - flow: row wrap;
	justify - content: center;
	max-width: 100 %;
}

)", config.Section);
}

void CssCreator::_CreateForImages()
{
	_ofs << R"(		/* ---- img----- */
.img-container {
	display:flex;
	flex-direction: column;
	justify-content:center;
	align-items:center;
	position: relative;
	padding:0 1px;
}

.img-container > div {
	position:relative;
}

.img-container img {
	margin:auto;
	max-width:99vw;
	padding: 1px;
)";
	if(config.imageBorder.Used())
		_ofs << config.imageBorder.ForStyleSheet(true);
	_ofs << "}\n\n";

	_ofs << "[data-src] {\n	min-width:" << config.thumbWidth.ToString()
		 << "\n	min-height:" << config.thumbHeight.ToString()
		 << "\n}";
}

void CssCreator::_CreateForImageTitle()
{
	_CssForElement(QString(".title:hover{\n"
				   "	font-weight: ") + (config.ImageTitle.font.Bold() ? "normal;\n":"700;\n")+
				   "    font-style: " + (config.ImageTitle.font.Italic() ? "normal;\n":"italic;\n")+
				   "}\n\n"
				   ".title {\n", config.ImageTitle);
}

void CssCreator::_CreateForImageDesc()
{
	_CssForElement(R"(.desc {
	hyphens: auto;
	-moz - hyphens: auto;
	-webkit - hyphens: auto;
	-ms - hyphens: auto;
)",
	config.ImageDesc );
}

void CssCreator::_CreateForLightboxTitle()
{
	_CssForElement(R"(/* Lightbox */
.lightbox {
    display: none;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0, 0, 0, 0.9);
    position: fixed;
    z-index: 20;
    padding-top: 30px;
    box-sizing: border-box;
	overflow:auto;
}

#lb-flex {
    display: flex;
    flex-direction:column;
    align-items:center;
    justify-content:center;
    width: 100%;
    height: 100%;
}
.lightbox-caption {" 
	display:none;
)",
	config.LightboxTitle);
}

void CssCreator::_CreateForLightboxDescription()
{
	_CssForElement( ".lightbox-desc {\n",config.LightboxDesc);
}

void CssCreator::_CreateForFooter()
{
	_CssForElement( ".footer {\n",config.Footer);
}

void CssCreator::_CreateMediaQueries()
{
	_ofs << R"(		 	/* -- media queries *** /
@media only screen and (orientation: landscape) {
	.img-container img {
		max-height: 95vh; 
	}
}

@media only screen and (min-width:700px) {
	.img-container img {
		max-width:99vw;
		height:400px;
	}

	p{
		font-size:12pt;
	}

	.gallery-desc {
		width:50vw;
		text-align:center;
		margin: auto;
	}
}

	/* large screens * /
@media only screen and (min-width:1200px) {
	.img-container
	{
		max-width:99vw;
		flex-direction:column;
		padding:0 1px;
	}

	.desc {
		max-width:600px;
		margin:auto;
	}

	.gallery-desc {
		width:800px;
		text-align:center;
		margin:auto;
	}
}
)";
}


void CssCreator::Create()
{
	_CreateGlobals();
	_CreateForMenuButtons();
	_CreateForLangButton();
	_CreateForHeader();
	_CreateForSmallTitle();
	_CreateForGalleryTitle();
	_CreateForGalleryDesc();
	_CreateForSection();
	_CreateForImageTitle();
	_CreateForImageDesc();
	_CreateForLightboxTitle();
	_CreateForLightboxDescription();
	_CreateForFooter();
	_CreateMediaQueries();
}


bool CssCreator::Save(QString name)
{
	if (!_Open(name))
		return false;

	Create();

	return true;
}


/* area only used for editing * /
nav {
    margin: 10px 10px 10px 10px;
}
	/* small title color%3, font-size:%4, font-maily:%5, font-weight %6* / 
.falconG {
	display:inline-block;
    color:%3
	font-size:18pt;
	font-family:var(--serif-fonts);
	font-weight:bold;
	font-style:italic;
    margin-left:10px;
}

.falconG::first-line {
	font-size:18pt;
}

header{
    color:var(--header-color);
	font-size:12pt;
	font-family:var(--script-fonts),var(--serif-fonts);
	text-align: left;
	text-shadow: 1px 1px 1px rgba(0, 0, 0, 0.2);
	cursor:pointer;				/* only for design! * /
}
/* if I am not using a separate div, but 
    put these to the NAV, then the image will scroll
    over the menu
* /    
nav {
	top: 10px;
	position: sticky;
	position: -webkit-sticky;
	display: inline-block;
	margin-bottom: 1rem;
    z-index:1;
}

.menu-item {
    color:var(--menu-color);
    background:var(--menu-background);
	font-size:10pt;
	font-family:var(--serif-fonts);
	font-weight:bold;
	line-height: 18px;
	box-shadow: 0 1px 1px rgba(0, 0, 0, 0.1);
	display: inline-block;
	margin: 2px 3px 2px 2px;
	padding: 4px 10px 3px;
    border-width:1px;
    border-color:var(--menu-border-color);
    border-style: solid;
	border-radius: var(--menu-border-radius);
}

.menu-item:hover {
	color:rgba(45,64,89,0.8);
    background-color:var(--menu-color);
    border: 1px solid var(--menu-border-color);
}

/* up icon * /
.menu-item#uplink {
	background: url("file:///res/up-icon-sample.png");
	background-size:14px;
	background-repeat:no-repeat;
	background-position: center;
}

/* if it is in one declaration then can I set it individually from javascript? * /
.menu-item#uplink, .menu-item#about, .menu-item#desc, .menu-item#captions, .manu-items#contact {
	display:inline-block;
}

.langs{
	color:#dddd00;
	font-size: 8pt;
	border:0;
	box-shadow: 0;
}

/* --- main section --* /
main{
	display: flex;
	flex-direction: column;
	flex-wrap:wrap;
	width: 100% ;
}
/* --- end main section --* /

.gallery-title {
    color:var(--lightbox-color);
	font-size:24pt;
	font-family:var(--script-font);
	font-weight:bold;
	text-align:center;
}

/* valtozott * /
.gallery-title::first-line {
	font-size:24pt;
}

.gallery-desc {
    color:var(--base-color);
    margin:auto;
    text-align:center;
    margin: 20px 0 20px 0;
	cursor:pointer;				/* only for design! * /
}

.gallery-desc::first-line {
	font-size:13pt;
}

section {
	display: flex;
	flex-flow: row wrap;
	justify-content: center;
    align-items:baseline;
	max-width: 100% ;
}

.fgsection {
	font-family:var(--script-font);
    color:var(--base-color);
	font-size:34pt;
	text-align:center;
    margin: 20px 0 20px 0;
	cursor:pointer;				/* only for design! * /
}

.fgsection::first-line {
	font-size:34pt;
	cursor:pointer;				/* only for design! * /
}

.title, .title a {
    color:var(--base-color);
	font-size:18pt;
	font-family:var(--script-fonts),var(--serif-fonts);
	font-weight:bold;
    margin:0 0 20px 0;
}

.title::first-line {
	font-size:24pt;
	cursor:pointer;				/* only for design! * /
}

.title:hover{
	font-weight: 700;
	font-style: italic;
}


.desc {
    color:#ffd460;
	display:block;				/* csak a debug miatt, marad none * /
	hyphens: auto;
	-moz-hyphens: auto;
	-webkit-hyphens: auto;
	-ms-hyphens: auto;
	font-family:var(--sans-serif-fonts);
	font-size:12pt;
	text-align: center;
	padding-top:10px;
	cursor:pointer;				/* only for design! * /
}

		/* ---- img----- * /
.img-container {
	display:flex;
	flex-direction: column;
	justify-content:center;
	align-items:center;
	padding:0 1px;
}

.img-container  {
	position:relative;
}

.img-container img {
	max-width:99vw;
	border:1px solid var(--base-color);
	padding:1px;
}

.thumb{
	display: inline-block;
	padding: 1px;
	margin: 0 2;
	padding:1px;
}	

[data-src] {
	min-width:600;
	min-height:400;
}	

.desc p {
	font-size: 12pt;
	line-height: 12pt;
	text-align: center;
	max-width:90%;
	margin:auto;
}	

footer{
    color:var(--footer-color);
	font-size:12pt;
	font-family:var(--script-fonts),var(--serif-fonts);
	text-align: center;
	margin:auto;
	cursor:pointer;				/* only for design! * /
	font-style:italic;
}

/* Lightbox * /
.lightbox {
    display: none;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0, 0, 0, 0.9);
    position: fixed;
    z-index: 20;
    padding-top: 30px;
    box-sizing: border-box;
	overflow:auto;
}

#lb-flex {
    display: flex;
    flex-direction:column;
    align-items:center;
    justify-content:center;
    width: 100%;
    height: 100%;
}

.lightbox img {
    display: block;
    cursor:pointer;
}
  
.lightbox-caption {
    color:var(--lightbox-color);
    margin: 15px auto;
    width: 50%;
    text-align: center;
    font-size: 1.5rem;
    line-height: 1.5;
    font-weight: 700;
}
/* /lightbox * /


)"
}


*/