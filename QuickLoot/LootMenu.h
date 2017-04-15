#pragma once

#include <SKSE.h>
#include <SKSE/GameTypes.h>
#include <SKSE/GameInput.h>
#include <SKSE/GameEvents.h>
#include <SKSE/GameMenus.h>
#include <SKSE/GameReferences.h>
#include <vector>
#include "ItemData.h"

class LootMenu : public IMenu,
	public MenuEventHandler,
	public BSTEventSink<TESContainerChangedEvent>,
	public BSTEventSink<MenuOpenCloseEvent>,
	public BSTEventSink<SKSECrosshairRefEvent>
{
protected:
	TESObjectREFR *				m_targetRef;
	TESObjectREFR *				m_containerRef;
	TESForm *					m_owner;
	SInt32						m_selectedIndex;
	//std::vector<ItemData>		m_items;
	BSTArray<ItemData>			m_items;
	const char *				m_mcRoot;
	bool						m_bNowTaking;
	bool						m_bOpenAnim;
	bool						m_bUpdateRequest;
	bool						m_bDisabled;

	static LootMenu *			ms_pSingleton;
	static BSSpinLock			ms_lock;

public:
	using GRefCountBaseStatImpl::operator new;
	using GRefCountBaseStatImpl::operator delete;

	LootMenu();
	virtual ~LootMenu();

	static bool Install();
	static bool SendRequestStart();
	static bool SendRequestStop();
	static LootMenu * GetSingleton()	{ return ms_pSingleton; }

	void Setup();

	static bool CanOpen(TESObjectREFR *a_ref);
	bool IsOpen() const;
	bool IsVisible() const;
	bool IsDisabled() const				{ return m_bDisabled; }
	void SetDisabled(bool disabled)		{ m_bDisabled = disabled; }
	const char * GetMcRoot()			{ return m_mcRoot; }

	bool Open(TESObjectREFR *a_ref);
	void Close();
	void Update();
	void Sort();
	void SetIndex(SInt32 index);
	void TakeItem();

	// for debugging purpose
	void Dump();

protected:
	// @override IMenu
	virtual UInt32 ProcessMessage(UIMessage *message) override;

	// @override MenuEventHandler
	virtual bool CanProcess(InputEvent *evn) override;
	virtual bool ProcessButton(ButtonEvent *evn) override;

	// @override BSTEventSink
	virtual EventResult ReceiveEvent(TESContainerChangedEvent *evn, BSTEventSource<TESContainerChangedEvent> *src) override;
	virtual EventResult ReceiveEvent(MenuOpenCloseEvent *evn, BSTEventSource<MenuOpenCloseEvent> *src) override;
	virtual EventResult ReceiveEvent(SKSECrosshairRefEvent *evn, BSTEventSource<SKSECrosshairRefEvent> *src) override;

	void OnMenuOpen();
	void OnMenuClose();

	void Clear();

	// scaleform
	void InvokeScaleform_Open();
	void InvokeScaleform_Close();
	void InvokeScaleform_SetIndex();
	void SetScaleformArgs_Open(std::vector<GFxValue> &args);
	void SetScaleformArgs_Close(std::vector<GFxValue> &args);
	void SetScaleformArgs_SetIndex(std::vector<GFxValue> &args);

	// misc
	static void SendItemsPickpocketedEvent(UInt32 numItems);
	static void SendChestLootedEvent();
	void PlaySound(TESForm *item);
	void PlayAnimation(const char *fromName, const char *toName);
	void PlayAnimationOpen();
	void PlayAnimationClose();

};
