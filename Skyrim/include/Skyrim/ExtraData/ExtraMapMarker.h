#pragma once

#include "BSExtraData.h"
#include "../FormComponents/TESFullName.h"

class TESForm;

/*==============================================================================
class ExtraMapMarker +0000 (_vtbl=01079D2C)
0000: class ExtraMapMarker
0000: |   class BSExtraData
==============================================================================*/
// 0C
class ExtraMapMarker : public BSExtraData
{
public:
	enum { kExtraTypeID = (UInt32)ExtraDataType::MapMarker };

	virtual ~ExtraMapMarker();

	class MarkerData : public TESFullName
	{
	public:

		enum
		{
			kFlag2_None = 0,
			kFlag2_Visible = 1 << 0,
			kFlag2_CantFastTravelTo = 1 << 1,
			kFlag2_HiddenWhenShowAll = 1 << 2,
		};

		UInt8			flags1;
		UInt8			flags2;
		UInt16			icon;

		void Enable(bool abEnabled) {
			if (abEnabled)
				flags1 |= 0x02;
			else
				flags1 &= 0xFD;
		}
		DEFINE_MEMBER_FN(ctor, MarkerData*, 0x00429050);
	};

	void Release();


	static ExtraMapMarker* Create();
	// @members
	//void	**					_vtbl	// 00 - 01079148
	MarkerData*					data;
};
