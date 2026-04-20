#pragma once
#include "common.h"

SCMap scMap;	// sample index to combo box index map


void SCMap::_InitIndices()
{
	struct _Relation
	{
		int sampleIndex, comboBoxIndex;
	};

	static QVector<_Relation> _relation = {
		{ nGlobal					,nCbIxGlobal			  },
		{ nBtnDnUp					,nCbIxMenu				  },
		{ nBtnDnAbout				,nCbIxMenu				  },
		{ nBtnDnContact				,nCbIxMenu				  },
		{ nBtnDnCaptDesc			,nCbIxMenu				  },
		{ nBtnDnDescription			,nCbIxMenu				  },
		{ nBtnDnToAlbums			,nCbIxMenu				  },
		{ nLblDnName				,nCbIxSmallTitle		  },
		{ nLblDnLang				,nCbIxLanguage			  },
		{ nLblDnGallery			 	,nCbIxGalleryTitle		  },
		{ nLblDnGalleryDescription 	,nCbIxGalleryDescr		  },
		{ nLblDnPictureSectionLabel	,nCbIxSection			  },
		{ nLblDnImageP			 	,nCbIxImageThumbnail	  },
		{ nLblDnDescriptionP		,nCbIxIADescription		  },
		{ nLblDnTitleP		 	 	,nCbIxIATitle			  },
		{ nLblDnTitlePO			 	,nCbIxIATitleO			  },
		{ nLblDnAlbumSectionLabel	,nCbIxSection			  },
		{ nLblDnImageA			 	,nCbIxAlbumThumbnail	  }, 
		{ nLblDnDescriptionA		,nCbIxIADescription		  },
		{ nLblDnTitleA		 	 	,nCbIxIATitle			  },
		{ nLblDnTitleAO	 		 	,nCbIxIATitleO			  },
		{ nLblDnFooter			 	,nCbIxFooter			  },
		{ nLblDnCopyright	 		,nCbIxFooter			  },
		{ nLblDnWatermark	 		,nCbIxWatermark			  },
		{ nLblDnHeader			 	,nCbIxHeader			  },
		{ nLblLightbox			 	,nCbIxLightbox			  },
		{ nLblLightboxDescription	,nCbIxLightBoxDescription  }
	};

	for (auto& a : _relation)
	{
		_sampleToComboboxIndex[a.sampleIndex] = a.comboBoxIndex;
		if(!_comboboxToSampleIndex.contains(a.comboBoxIndex))			// do not replace image for album
			_comboboxToSampleIndex[a.comboBoxIndex] = a.sampleIndex;	// because both will be set the same time
	}																	// remove if for separate  nCbIxAlbumThumbnail
}

int SCMap::SampleToCBIx(int nSx)
{
	if (nSx < 0 || nSx > _sampleToComboboxIndex.size())
		return -1;
	return _sampleToComboboxIndex[nSx];
}

int SCMap::CBIxToSample(int nCx, int *pix) // buffer pointed by pix must be large enough! NO CHECK!
{										   // returns count set in pix array
	if (nCx < 0)	// then all
	{
		for (int i = 0; i < _comboboxToSampleIndex.size(); ++i)
			*pix++ = _comboboxToSampleIndex[i];
		return _comboboxToSampleIndex.size();
	}
	*pix = nCx < _comboboxToSampleIndex.size()  ? _comboboxToSampleIndex[nCx] : 0;	// single item
	return 1;
}

// ----------------------- ID_t ------------------

ID_t::ID_t(QString idString, uint8_t flags, uint _dirIndex) : _flags(flags)
{
	if (idString.isEmpty())
		_uval = 0;
	else if (idString.lastIndexOf('i') > 0)
	{
		_uval = idString.left(idString.lastIndexOf('i')).toULongLong();
		_dirIndex = idString.mid(idString.lastIndexOf('i') + 1).toUInt();
	}
	else
		_uval = idString.toULongLong();
}
QString ID_t::ValToString() const
{
	if (_dirIndex && _dirIndex != NOT_SET)
		return QString("%1i%2").arg(_uval).arg(_dirIndex);
	else
		return QString().setNum(_uval);
}
