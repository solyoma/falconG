#include "csscreator.h"


bool CssCreator::_Open(QString name)
{
	_f.setFileName(name);
	if (!_f.open(QIODevice::WriteOnly))
		return false;

	_ofs.setDevice(&_f);
	return true;
}

void CssCreator::_CssForElement(QString classStr, _CElem &elem)
{
	QString qs = elem.ForStyleSheet(true);
	if (!qs.isEmpty())
	{
		if (elem.spaceAfter > 0)
			qs += QString("margin-bottom:%1px;\n").arg(elem.spaceAfter);
		_ofs << classStr << qs << "}\n\n";
	}
	if (elem.font.IsFirstLineDifferent())
		_ofs << elem.font.FirstLineClassStr(elem.ClassName());
}

/*TODO: scan font directoryand embed fonts from it into here
 *		e.g. @font-face{
 *		font-family: 'RieslingRegular';
 *		src: url('fonts/riesling.eot');
 *		src: local('Riesling Regular'), local('Riesling'), url('fonts/riesling.ttf') format('truetype');
 * usage example : h1{ font-family: 'RieslingRegular', Arial, sans-serif; }
*/
void CssCreator::_CreateGlobals()
{
	QString s = config.sGoogleFonts;
	if (!s.isEmpty())
		s = "@import url('https://fonts.googleapis.com/css?family=" + s + "&display=swap');\n";
	_ofs << s;

	s = config.dsFontDir.ToString();
	if (!s.isEmpty())
	{
		if (s.right(1) != '/')
			s += "/";
/*
* TODO
		QString qs = config.sDefFonts.ToString();
		if (!qs.isEmpty())
		{
			qs.replace(QRegularExpression("[ \t]*,[ \t]*"), "|");
			_ofs << "@font-face(" << s + qs + "\");\n\n";
		}
*/
	}

	_ofs << 
		// universal selector to include padding and border in box size
		R"(
*, *::before, *::after {
	box-sizing:border-box;
	padding:0;
	margin:0;
}

html, main {
)";
	_ofs 
		<< R"(	height: 100%;
	width: 100%;
}
		
a, a:visited {
	text-decoration:none;
}

.area {
	margin:0 6px;
}
)";

	s = config.backgroundImage.ForStyleSheet(true);

	if (s.at(s.indexOf(':')+1) == ';')		// no image
	{
		_ofs << "body {\n"
			 << s
			 << config.Web.ColorsForStyleSheet(true) 
			 << "}\n\n";
	}

	_ofs << R"(
.main{
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
	max-width: 100%;
}

)";
}

void CssCreator::_CreateForMenuButtons()
{
	/* %1:menu color %2:var(--menu-background), 
	   %3:var(--serif-fonts), %4: font-size*/
	_ofs << R"(
nav {
	display: inline-block;
	position: sticky;
	position: -webkit-sticky;
	top:10px;
	margin-bottom: 1rem;
	z-index: 1;
}

.menu-item {
	display: inline-block;
	margin: 2px 3px 2px 2px;
	padding: 4px 10px 3px;
)";
	QString qs = config.Menu.ForStyleSheet(true);
	if(!qs.isEmpty())
		_ofs << qs << "}\n"	// for menus no different first-line accepted
			<< "\n.menu-item:hover {\n"
			<< config.Menu.background.ForStyleSheet(true, true)
			<< config.Menu.color.ForStyleSheet(true, false) << "}\n";
	_ofs << R"(/* up icon */
.menu-item#uplink {
	background-image: url("../res/up-icon.png");
	background-size:14px;
	background-repeat:no-repeat;
	background-position: center;
}

)";
}

void CssCreator::_CreateForLangButton()
{
	QString s = ".langs {\n";
	_CssForElement(s ,config.Lang); 
}

void CssCreator::_CreateForHeader()
{
	QString s = ".header {\n";
	if (_forFalconG)
		s += "	cursor:pointer;\n";

	_CssForElement(s, config.Header);
}

void CssCreator::_CreateForSmallTitle()
{
	QString s = R"(
.falconG {
	margin-left: 10px;
	display:inline-block;
)";
	if (_forFalconG)
		s += "    cursor:pointer;\n";

	_CssForElement(s, 	config.SmallGalleryTitle);
}

void CssCreator::_CreateForGalleryTitle()
{
	QString s = ".gallery-title {\n";
	if (_forFalconG)
		s += "    cursor:pointer;\n";
	_CssForElement(s, config.GalleryTitle);
}

void CssCreator::_CreateForGalleryDesc()
{
	QString s = ".gallery-desc {\n";
	if (_forFalconG)
		s += "    cursor:pointer;\n";
	_CssForElement(s, config.GalleryDesc);
}

void CssCreator::_CreateForSection()
{
	QString s = QString(R"(.fgsection {
	margin: auto;
	padding:%1;
)").arg(config.Section.font.SizeStr());

	_CssForElement(s, config.Section);
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
}

.img-container > div {
	position:relative;
}

.img-container img {
	display:block;			/* get rid of bottom space */
	margin:auto;
	max-width:99vw;
}

.thumb {
	display: inline-block;
)";
	// image border is used for <a class="thumb">
	if(config.imageBorder.Used())
		_ofs << config.imageBorder.ForStyleSheetShort(true);
	_ofs << "\n}\n\n";

	_ofs << "[data-src] {\n	min-width:" << config.thumbWidth.ToString()
		 << "px;\n	min-height:" << config.thumbHeight.ToString()
		 << "px;\n}\n";
}

void CssCreator::_CreateForImageTitle()
{
	QString s =	QString(".title:hover{\n"
				   "	font-weight: ") + (config.ImageTitle.font.Bold() ? "normal;\n":"700;\n")+
				   "	font-style: " + (config.ImageTitle.font.Italic() ? "normal;\n":"italic;\n")+
				   "}\n\n"
				   ".title {\n";
//	if (_forFalconG)
		s += "	cursor:pointer;\n";

	_CssForElement(s, config.ImageTitle);
}

void CssCreator::_CreateForImageDesc()
{
	QString s = R"(.desc {
	hyphens: auto;
	-moz-hyphens: auto;
	-webkit-hyphens: auto;
	-ms-hyphens: auto;
)";
	if (_forFalconG)
		s += "    cursor:pointer;\n";
	_CssForElement(s, config.ImageDesc );
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

.lightbox-caption { 
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
	QString s = ".footer {\n";
	if (_forFalconG)
		s += "    cursor:pointer;\n";
	_CssForElement( s,config.Footer);
}

void CssCreator::_CreateMediaQueries()
{
	_ofs << R"(		 	/* -- media queries ***/
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
		margin: auto;
	}
}

	/* large screens */
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
		margin:auto;
	}
}
)";
}


void CssCreator::_Create()
{
	_CreateGlobals();
	_CreateForMenuButtons();
	_CreateForLangButton();
	_CreateForHeader();
	_CreateForSmallTitle();
	_CreateForGalleryTitle();
	_CreateForGalleryDesc();
	_CreateForSection();
	_CreateForImages();
	_CreateForImageTitle();
	_CreateForImageDesc();
	_CreateForLightboxTitle();
	_CreateForLightboxDescription();
	_CreateForFooter();
	_CreateMediaQueries();
}


bool CssCreator::Create(QString name, bool forFalconG)
{
	if (!_Open(name))
		return false;

	_forFalconG = forFalconG;
	_Create();
	_Close();

	return true;
}
