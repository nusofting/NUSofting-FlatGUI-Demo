#pragma once

#if MAC
#include "time.h"
#endif

#if WINDOWS
#include "windows.h"
#endif

// Beta Testing End Dates
#define BetaEndDay 16
#define BetaEndMonth 12
#define BetaEndYear 2022

struct Beta_Switch
{
	bool HasExpired;
	bool isComms;
	int Day, Month, Year;
	Beta_Switch()
	{
		Day = BetaEndDay;
		Month = BetaEndMonth;
		Year = BetaEndYear;
		HasExpired = false;
		isComms = true;
	}

	void CheckDate_betaProtection2() 
	{ 
#if MAC
		time_t t; time(&t); tm* Time=gmtime(&t); 
		if ((Time->tm_mday>Day && (Time->tm_mon+1)>=Month && (Time->tm_year+1900)>=Year) || ((Time->tm_mon+1)>Month && (Time->tm_year+1900)>=Year) || (Time->tm_year+1900)>Year) 
		{ HasExpired = true; } 
#else
		SYSTEMTIME Time;	GetSystemTime(&Time); 
		if ((Time.wDay > Day && Time.wMonth >= Month && Time.wYear >= Year) || (Time.wMonth > Month && Time.wYear >= Year) || Time.wYear > Year ) 
		{ HasExpired = true; } 
#endif		
	}
	void sendBetaMesssage()
	{
		if(HasExpired && isComms) 
		{
			isComms = false;
			Platform::PopupMessage("This plugin beta version has expired! \n Please stop playing to close it.","Warning!");			
		}
	}
	bool getState() { return HasExpired;}

		void sendNFRMesssage()
	{
		if(HasExpired && isComms) 
		{
			isComms = false;
			Platform::PopupMessage("This plugin NFR version has expired! \n Please stop playing to close it.","Warning!");			
		}
	}

};	


#define Beta_Active_Time 1010 // 15+ minutes 
