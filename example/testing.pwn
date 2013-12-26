#include <a_samp>
#include <tfasamp>

main() {}

new timerCheck[MAX_PLAYERS];

#define 	DIALOG_SELECT_USERID	1000
#define 	DIALOG_VERIFY_TOKEN		1001

public OnGameModeInit()
{
	TFASAMP::prepareConnection("hostname", "password", "apikey", "production", true);
	return 1;
}

public OnPlayerConnect(playerid)
{	
	ShowPlayerDialog(playerid, DIALOG_SELECT_USERID, DIALOG_STYLE_INPUT, "Insert USER-ID", "This is a testing dialog. This phase should be voided for production gamemodes.\nInsert your Authy's USER-ID:", "Continue", "");
	return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) 
{
	switch(dialogid)
	{
		case DIALOG_SELECT_USERID:
		{
			SendClientMessage(playerid, 000000, "{FFFFFF}You have written your Authy's user id.");
			
			TFASAMP::setPlayerUserID(playerid, strval(inputtext));
			
			ShowPlayerDialog(playerid, DIALOG_VERIFY_TOKEN, DIALOG_STYLE_INPUT, "Insert TOKEN", "This is a testing dialog. This phase should be voided for production gamemodes.\nInsert the TOKEN:", "Verify", "");	
			return true;
		}
		
		case DIALOG_VERIFY_TOKEN:
		{
			SendClientMessage(playerid, 000000, "{FFFFFF}Loading..");
			
			TFASAMP::verifyToken(playerid, TFASAMP_getPlayerUserID(playerid), inputtext);
			
			timerCheck[playerid] = SetTimerEx("checkTokenVerification", 500, true, "i", playerid);
			return true;
		}		
	}
	
	return false;
}

forward checkTokenVerification(playerid);
public checkTokenVerification(playerid) 
{
	if(TFASAMP::isHTTPProcessing(playerid))
		return -1;
	else 
	{
		KillTimer(timerCheck[playerid]);
		
		if(TFASAMP::isTokenVerifiedValid(playerid))
			return SendClientMessage(playerid, 000000, "{FFFFFF}The token was valid!"), true;
		else
			return SendClientMessage(playerid, 000000, "{FF0000}The token was invalid!"), false;
	}
}