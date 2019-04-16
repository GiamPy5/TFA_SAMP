#include <a_samp>
#include <sscanf2>
#include <a_json>
#include <tfasamp>
#include <strlib>

#if !defined isnull
    #define         isnull(%1)                  ((%1[0] == 0) || (%1[0] == 1 && %1[1] == 0))        
#endif

main() {}

#define     DIALOG_SELECT_USERID    1000
#define     DIALOG_VERIFY_PUSH     1001

public OnGameModeInit()
{
    TFASAMP_prepareConnection("localhost/", "testing", "api_key", "production", true);
    
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
            if(!response)
                ShowPlayerDialog(playerid, DIALOG_SELECT_USERID, DIALOG_STYLE_INPUT, "Insert USER-ID", "This is a testing dialog. This phase should be voided for production gamemodes.\nInsert your Authy's USER-ID:", "Continue", "");

            SendClientMessage(playerid, 000000, "{FFFFFF}You have written your Authy's user id.");
            
            TFASAMP_setPlayerUserID(playerid, strval(inputtext));
            TFASAMP_pushNotification(playerid, TFASAMP_getPlayerUserID(playerid));
            
            ShowPlayerDialog(playerid, DIALOG_VERIFY_PUSH, DIALOG_STYLE_MSGBOX, "Attention", "Please accept the push notification that has been send in your mobile phone! And then hit accept here.", "Verify", "");
            return true;
        }
        
        case DIALOG_VERIFY_PUSH:
        {
            SendClientMessage(playerid, 000000, "{FFFFFF}Loading..");
            new TFA_UUID:uuid[60];
            TFASAMP_getUUID(playerid, uuid, sizeof uuid);

            TFASAMP_getNotification(playerid, uuid);
            return true;
        }       
    }
    
    return false;
}

public TFASAMP_OnPushNotification(playerid, TFA_UUID:uuid[]){

    TFASAMP_setUUID(playerid, TFA_UUID:uuid);

    return 1;
}

public TFASAMP_OnCheckNotification(playerid, status){

    if(status){
        SendClientMessage(playerid, -1, "Push notification successfully approved.");
        ShowPlayerDialog(playerid, DIALOG_SELECT_USERID, DIALOG_STYLE_INPUT, "Insert USER-ID", "This is a testing dialog. This phase should be voided for production gamemodes.\nInsert your Authy's USER-ID:", "Continue", "");
    }
    else{
        SendClientMessage(playerid, -1, "Push notification not approved.");
        TFASAMP_pushNotification(playerid, TFASAMP_getPlayerUserID(playerid));
        ShowPlayerDialog(playerid, DIALOG_VERIFY_PUSH, DIALOG_STYLE_MSGBOX, "Attention", "Please accept the push notification that has been send in your mobile phone! And then hit accept here.", "Verify", "");
    }

    return 1;
}
