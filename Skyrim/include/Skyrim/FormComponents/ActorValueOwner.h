#pragma once

#include "BaseFormComponent.h"

/*==============================================================================
class ActorValueOwner +0000 (_vtbl=010A5400)
0000: class ActorValueOwner
==============================================================================*/
// 04
class ActorValueOwner
{
public:
	virtual ~ActorValueOwner();											// 0055FF60

	// Argument is the ActorValue ID
	virtual float	GetCurrent(UInt32 arg);								// 0055FF50 { return 0.0f; }
	virtual float	GetMaximum(UInt32 arg);								// 0055FF50 { return 0.0f; }
	virtual float	GetBase(UInt32 arg);								// 0055FF50 { return 0.0f; }
	virtual void	SetBase(UInt32 arg0, float arg1);					// 004D43E0 { return; }
	virtual void	ModBase(UInt32 arg0, float arg1);					// 004D43E0 { return; }
	virtual void	DamageAV(UInt32 arg0, UInt32 idx, float amount);		// 00D62BE0 { return; } Force/Mod AV?
	virtual void	SetCurrent(UInt32 arg0, float arg1);				// 00560330
	virtual bool	ActorValueOwner_Unk_08(void);						// 0092D110 { return false; } PlayerCarachter={ return true; }

	//	void	** _vtbl;	// 00
};
STATIC_ASSERT(sizeof(ActorValueOwner) == 0x04);

/*
// @override class ActorValueOwner : (vtbl=010D1F58)
virtual ????   Unk_000(????) override;                           // 0074E660
virtual ????   Unk_001(????) override;                           // 006DF9C0
virtual ????   Unk_002(????) override;                           // 006DFA90
virtual ????   Unk_003(????) override;                           // 006DF850
virtual ????   Unk_004(????) override;                           // 006E06F0
virtual ????   Unk_005(????) override;                           // 006E0730
virtual ????   Unk_006(????) override;                           // 006E08C0
virtual bool   Unk_008(void) override;                           // 009B86F0 { return true; }
*/