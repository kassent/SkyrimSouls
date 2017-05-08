#include "Skyrim.h"
#include "Skyrim/ExtraData/ExtraMapMarker.h"
#include "Skyrim/Memory.h"

static const UInt32 s_ExtraMapMarkerVtbl = 0x01079D2C;

ExtraMapMarker* ExtraMapMarker::Create()
{
	MarkerData* markerData = FormHeap_Allocate<MarkerData>();
	if (markerData)
	{
		markerData->ctor();
		markerData->flags1 = 0x03;
		markerData->flags2 = 0x03;
		markerData->icon = 0x19;

		ExtraMapMarker* xMarker = (ExtraMapMarker*)BSExtraData::Create(sizeof(ExtraMapMarker), s_ExtraMapMarkerVtbl);
		if (xMarker != nullptr)
		{
			xMarker->next = nullptr;
			xMarker->data = markerData;
			return xMarker;
		}
		else
			FormHeap_Free(markerData);
	}
	return nullptr;
	//0040F960    A1 54EBBB01     mov eax,dword ptr ds:[0x1BBEB54]  BSExtraList::MapMarker(Data*);
}

void ExtraMapMarker::Release()
{
	next = nullptr;
	data->fullName.impl_Release();
	FormHeap_Free(data);
	FormHeap_Free(this);
}