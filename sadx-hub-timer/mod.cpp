#include "stdafx.h"

#include <string>

#include <SADXModLoader.h>
#include <IniFile.hpp>

#define bool unsigned int
#define true 1
#define false 0

extern "C"
{	
	//DataPointer(int, GlobalFrameCounter, 0x03B2C6C8);
	static const char *TimerStateStrings[2] = 
	{
		"Stopped", "Running"
	};
	
	int previousGameState = 0;
	int previousHeldButtons = 0;
	int previousFrameCounter = 0;
	
	int elapsedFrames = 0;
	
	int timer = 0;
	bool isStarted = false;
	
	int hubsToTime = 1;
	int currentHubTimed = 1;
	bool timeThePauses = false;
	
	int color = 0;
	
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
	
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };
	
	__declspec(dllexport) void Init(const char* path, const HelperFunctions&)
	{
		previousGameState = GameState;
	}
	
	__declspec(dllexport) void __cdecl OnFrame()
	{
		
		int deltaTime = FrameCounter - previousFrameCounter;
		if (GameState != 16)
		{
			color = 0xAF00AFFF;
			ScaleDebugFont(24);
			SetDebugFontColor(0xFFFF7E00);
			
			char buf[32];
			float recSize = strlen(itoa(timer, buf, 10)) * 1.5;
			DrawDebugRectangle(1.0f, 1.0f, 11.0f + recSize, 3.5f);
			DisplayDebugStringFormatted(NJM_LOCATION(1, 1), "TIME: %d", timer);
		}
		else
		{
			color = 0xFF524F4F;
			ScaleDebugFont(24);
			SetDebugFontColor(0xFFFF7E00);
			
			DrawDebugRectangle(1.0f, 1.0f, 26.0f, 8.0f);
			DisplayDebugStringFormatted(NJM_LOCATION(1, 1), "TIME: %d", timer);
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
		
		if (GameState != 16)
		{			
			if ((ControllerPointers[0]->HeldButtons & Buttons_C) != 0 && (previousHeldButtons & Buttons_C) == 0)
				isStarted = false;
			if ((ControllerPointers[0]->HeldButtons & Buttons_Z) != 0 && (previousHeldButtons & Buttons_Z) == 0)
				timer = 0;
		}
		else
		{
			if ((ControllerPointers[0]->HeldButtons & Buttons_C) != 0 && (previousHeldButtons & Buttons_C) == 0 && (ControllerPointers[0]->HeldButtons & Buttons_Y) == 0 && (ControllerPointers[0]->HeldButtons & Buttons_Start) == 0)
				isStarted = false;
			if ((ControllerPointers[0]->HeldButtons & Buttons_Z) != 0 && (previousHeldButtons & Buttons_Z) == 0 && (ControllerPointers[0]->HeldButtons & Buttons_Y) == 0 && (ControllerPointers[0]->HeldButtons & Buttons_Start) == 0)
				timer = 0;
			if ((ControllerPointers[0]->HeldButtons & Buttons_C) != 0 && (previousHeldButtons & Buttons_C) == 0 && (ControllerPointers[0]->HeldButtons & Buttons_Start) != 0)
			{
				if (hubsToTime > 1)
					hubsToTime--;
			}
			if ((ControllerPointers[0]->HeldButtons & Buttons_Z) != 0 && (previousHeldButtons & Buttons_Z) == 0 && (ControllerPointers[0]->HeldButtons & Buttons_Start) != 0)
			{
				if (hubsToTime < 9)
					hubsToTime++;
			}
			if ((ControllerPointers[0]->HeldButtons & Buttons_C) != 0 && (previousHeldButtons & Buttons_C) == 0 && (ControllerPointers[0]->HeldButtons & Buttons_Y) != 0)
			{
				if (timeThePauses == false)
					timeThePauses = true;
				else
					timeThePauses = false;
			}
			
			if ((((ControllerPointers[0]->HeldButtons & Buttons_Z) != 0) && (previousHeldButtons & Buttons_Z) == 0) && ((ControllerPointers[0]->HeldButtons & Buttons_Y) != 0))
				isStarted = true;
		}
		
		previousGameState = GameState;
		previousHeldButtons = ControllerPointers[0]->HeldButtons;
		previousFrameCounter = FrameCounter;
	}
}