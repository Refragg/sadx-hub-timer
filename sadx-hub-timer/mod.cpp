#include "stdafx.h"

#include <string>

#include <SADXModLoader.h>
#include <IniFile.hpp>

#define bool unsigned int
#define true 1
#define false 0

#define B_A Buttons_A
#define B_B Buttons_B
#define B_X Buttons_X
#define B_Y Buttons_Y
#define B_S Buttons_Start
#define B_S2 Buttons_D

#define B_DD Buttons_Down
#define B_DU Buttons_Up
#define B_DL Buttons_Left
#define B_DR Buttons_Right

#define B_LB Buttons_C
#define B_RB Buttons_Z

#define B_LT Buttons_L
#define B_RT Buttons_R

#define OPTIONS_A "A"
#define OPTIONS_B "B"
#define OPTIONS_X "X"
#define OPTIONS_Y "Y"
#define OPTIONS_S "Start"
#define OPTIONS_S2 "Back"
#define OPTIONS_DU "DUp"
#define OPTIONS_DD "DDown"
#define OPTIONS_DL "DLeft"
#define OPTIONS_DR "DRight"
#define OPTIONS_LB "LB"
#define OPTIONS_RB "RB"
#define OPTIONS_LT "LT"
#define OPTIONS_RT "RT"

extern "C"
{
	//DataPointer(int, GlobalFrameCounter, 0x03B2C6C8);
	static const char *TimerStateStrings[2] = 
	{
		"Stopped", "Running"
	};
	
	int previousGameState = 0;
	int previousHeldButtons = 0;
	int currentHeldButtons = 0;
	int previousFrameCounter = 0;
	
	int elapsedFrames = 0;
	
	int timer = 0;
	bool isStarted = false;
	
	int hubsToTime = 1;
	int currentHubTimed = 1;
	bool timeThePauses = false;
	
	int color = 0;

	Buttons resetTimerButton;
	Buttons stopTimerButton;

	Buttons hubsToTimePlusButton;
	Buttons hubsToTimeMinusButton;

	Buttons modifierButton;

	Buttons pausesTimingButton;
	Buttons manualStartButton;
	
	void DrawDebugRectangle(float leftchars, float topchars, float numchars_horz, float numchars_vert)
	{
		float FontScale;
		FontScale = 1.0f;
		
		njColorBlendingMode(0, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
		
		DrawRect_Queue(leftchars*FontScale*16.0f, topchars*FontScale*16.0f, numchars_horz*FontScale*16.0f, numchars_vert*FontScale*16.0f, 62041.496f, color, QueuedModelFlagsB_EnableZWrite);
		njColorBlendingMode(0, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
	}
	
	void ScaleDebugFont(int scale)
	{
		float FontScale;
		FontScale = 1.0;
		
		SetDebugFontSize(FontScale*scale);
	}

	Buttons GetButton(std::string buttonString)
	{
#define OPTIONS_COMP(option, button) if (buttonString == option) return button;
		OPTIONS_COMP(OPTIONS_A, B_A)
		OPTIONS_COMP(OPTIONS_B, B_B)
		OPTIONS_COMP(OPTIONS_X, B_X)
		OPTIONS_COMP(OPTIONS_Y, B_Y)
		OPTIONS_COMP(OPTIONS_S, B_S)
		OPTIONS_COMP(OPTIONS_S2, B_S2)
		OPTIONS_COMP(OPTIONS_A, B_A)
		OPTIONS_COMP(OPTIONS_DU, B_DU)
		OPTIONS_COMP(OPTIONS_DD, B_DD)
		OPTIONS_COMP(OPTIONS_DL, B_DL)
		OPTIONS_COMP(OPTIONS_DR, B_DR)
		OPTIONS_COMP(OPTIONS_LB, B_LB)
		OPTIONS_COMP(OPTIONS_RB, B_RB)
		OPTIONS_COMP(OPTIONS_LT, B_LT)
		OPTIONS_COMP(OPTIONS_RT, B_RT)
#undef OPTIONS_COMP
	}
	
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };
	
	__declspec(dllexport) void Init(const char* path, const HelperFunctions&)
	{
		const IniFile* configFile = new IniFile(std::string(path) + "\\config.ini");

		stopTimerButton = GetButton(configFile->getString("Settings", "StopTimerButton", OPTIONS_LB));
		resetTimerButton = GetButton(configFile->getString("Settings", "ResetTimerButton", OPTIONS_RB));

		hubsToTimePlusButton = GetButton(configFile->getString("Settings", "HubsToTimePlusButton", OPTIONS_RB));
		hubsToTimeMinusButton = GetButton(configFile->getString("Settings", "HubsToTimeMinusButton", OPTIONS_LB));

		modifierButton = GetButton(configFile->getString("Settings", "ModifierButton", OPTIONS_Y));

		pausesTimingButton = GetButton(configFile->getString("Settings", "PausesTimingButton", OPTIONS_LB));
		manualStartButton = GetButton(configFile->getString("Settings", "ManualStartButton", OPTIONS_RB));

		delete configFile;

		previousGameState = GameState;
	}

	inline bool IsButtonPressed(Buttons button)
	{
		return (currentHeldButtons & button) != 0 && (previousHeldButtons & button) == 0;
	}

	inline bool IsButtonHeld(Buttons button)
	{
		return (currentHeldButtons & button) != 0;
	}
	
	__declspec(dllexport) void __cdecl OnFrame()
	{
		int deltaTime = FrameCounter - previousFrameCounter;

		float time = timer / 60.0;
		if (GameState != 16)
		{
			color = 0xAF00AFFF;
			ScaleDebugFont(24);
			SetDebugFontColor(0xFFFF7E00);
			
			char buf[32];
			float recSize = strlen(itoa(time, buf, 10)) * 1.5;
			DrawDebugRectangle(1.0f, 1.0f, 15.6f + recSize, 3.5f);
			DisplayDebugStringFormatted(NJM_LOCATION(1, 1), "TIME: %.2f", time);
		}
		else
		{
			color = 0xFF524F4F;
			ScaleDebugFont(24);
			SetDebugFontColor(0xFFFF7E00);
			
			DrawDebugRectangle(1.0f, 1.0f, 26.0f, 8.0f);
			DisplayDebugStringFormatted(NJM_LOCATION(1, 1), "TIME: %.2f", time);
			DisplayDebugStringFormatted(NJM_LOCATION(1, 2), "State: %s", TimerStateStrings[isStarted]);
			DisplayDebugStringFormatted(NJM_LOCATION(1, 3), "Hubs to time: %d", hubsToTime);
			
			if (timeThePauses == false)
				DisplayDebugString(NJM_LOCATION(1, 4), "Time pauses: No");
			else
				DisplayDebugString(NJM_LOCATION(1, 4), "Time pauses: Yes");
			
		}
		
		if ((previousGameState == 9 || previousGameState == 17 || previousGameState == 22) && (GameState == 4 || GameState == 15) && isStarted == false)
		{
			if (timer == 0)
			{
				isStarted = true;
				currentHubTimed = 0;
				timer = 0;
			}
		}
		
		if (GameState == 9 && previousGameState != 9)
		{
			currentHubTimed++;
			if (currentHubTimed == hubsToTime)
				isStarted = false;
		}
		
		if (isStarted == true)
		{
			if (timeThePauses == true)
				timer++;
			if (GameState != 16 && timeThePauses == false)
				timer++;
		}

		currentHeldButtons = ControllerPointers[0]->HeldButtons;
		
		if (GameState != 16) // In-Game
		{
			if (IsButtonPressed(stopTimerButton))
				isStarted = false;

			if (IsButtonPressed(resetTimerButton))
				timer = 0;
		}
		else // Paused
		{
			if (IsButtonPressed(stopTimerButton) && !IsButtonHeld(modifierButton) && !IsButtonHeld(B_S))
				isStarted = false;

			if (IsButtonPressed(resetTimerButton) && !IsButtonHeld(modifierButton) && !IsButtonHeld(B_S))
				timer = 0;

			if (IsButtonPressed(hubsToTimeMinusButton) && IsButtonHeld(B_S))
			{
				if (hubsToTime > 1)
					hubsToTime--;
			}
			if (IsButtonPressed(hubsToTimePlusButton) && IsButtonHeld(B_S))
			{
				if (hubsToTime < 9)
					hubsToTime++;
			}

			if (IsButtonPressed(pausesTimingButton) && IsButtonHeld(modifierButton))
			{
				if (timeThePauses == false)
					timeThePauses = true;
				else
					timeThePauses = false;
			}
			
			if (IsButtonPressed(manualStartButton) && IsButtonHeld(modifierButton))
				isStarted = true;
		}

		previousGameState = GameState;
		previousHeldButtons = ControllerPointers[0]->HeldButtons;
		previousFrameCounter = FrameCounter;
	}
}