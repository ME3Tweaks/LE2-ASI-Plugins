/*
#############################################################################################
# Mass Effect 2 (Legendary Edition) (2.0.0.48602) SDK
# Generated with TheFeckless UE3 SDK Generator v1.4_Beta-Rev.53-MELE
# ========================================================================================= #
# File: SFXGameDLC_PRE_General_classes.h
# ========================================================================================= #
# Credits: uNrEaL, Tamimego, SystemFiles, R00T88, _silencer, the1domo, K@N@VEL
# Thanks: HOOAH07, lowHertz
# Forums: www.uc-forum.com, www.gamedeception.net
#############################################################################################
*/


#pragma once
#include <Windows.h>
#include <cstdio>
#include "../SdkInitializer.h"



#ifdef _MSC_VER
	#pragma pack ( push, 0x4 )
#endif

/*
# ========================================================================================= #
# Constants
# ========================================================================================= #
*/


/*
# ========================================================================================= #
# Enums
# ========================================================================================= #
*/


/*
# ========================================================================================= #
# Classes
# ========================================================================================= #
*/

// Class SFXGameDLC_PRE_General.SFXGameEffect_Gear_InfernoNegotiationBonus
// 0x0000 (0x0094 - 0x0094)
class USFXGameEffect_Gear_InfernoNegotiationBonus : public USFXGameEffect_PassiveNegotiationBonus
{
public:

private:
	static UClass* pClassPointer;

public:
	static UClass* StaticClass();

};

// Class SFXGameDLC_PRE_General.SFXGameEffect_Gear_InfernoPowerDamage
// 0x0000 (0x0098 - 0x0098)
class USFXGameEffect_Gear_InfernoPowerDamage : public USFXGameEffect_PowerDamageBonus
{
public:

private:
	static UClass* pClassPointer;

public:
	static UClass* StaticClass();

};

// Class SFXGameDLC_PRE_General.SFXGameEffect_Gear_InfernoStormSpeed
// 0x0000 (0x0098 - 0x0098)
class USFXGameEffect_Gear_InfernoStormSpeed : public USFXGameEffect_PassiveStormSpeedBonus
{
public:

private:
	static UClass* pClassPointer;

public:
	static UClass* StaticClass();

};


#ifdef _MSC_VER
	#pragma pack ( pop )
#endif