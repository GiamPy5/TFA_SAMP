/*
 * The first Two-Factor Authentication Method for San Andreas Multiplayer.
 * Copyright (C) 2013 Giampaolo Falqui
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,  
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/

/*
 * Available Functions:
 * - TFASAMP_prepareConnection(host[], password[], api_key[], type[] = "development")
 * - TFASAMP_addUser(playerid, email[], cellphone[], area_code[] = 1)
 * - TFASAMP_verifyToken(playerid, user_id, token, bool: force = true)
 * - TFASAMP_isTokenVerifiedValid(playerid)
*/

/*
 * Includes
*/

#include 		<a_samp>
#include		<a_http>

/*
 * Macros
*/

// If 'isnull' is not defined we define it.
#if !defined isnull

	#define isnull(%1) \
		((%1[0] == 0) || (%1[0] == 1 && %1[1] == 0))
		
#endif

/*
 * Variables Declaration
*/

// Backward-compatibility define
#define 	TFASAMP_verifyTokenTime(%1) \
				TFASAMP_isTokenVerifiedValid(%1)

#define 	TFASAMP_DEBUG		1

enum TFASAMP_internalInformation {
	HOSTNAME[1024],
	PASSWORD[128],
	API_KEY[128],
	TYPE[128],
	bool: CONNECTION_PREPARED,
};

enum TFASAMP_playerInformation {
	USER_ID,
	TOKEN_CHECK,
	LAST_CHECK,
	bool: REQUESTING_API,
};

new TFASAMP_INTERNAL[TFASAMP_internalInformation];
new TFASAMP_PLAYER[MAX_PLAYERS][TFASAMP_playerInformation];
 
/*
 * Functions
*/ 

/*
 * TFASAMP_prepareConnection
 * This function prepares the connection to your hosting in order to request the APIs.
 *
 * host[] = Your webhosting link and the directory, if any (without http://).
 * password[] = The password to use the PHP files to communicate with the APIs.
 * api_key[] = You may find it in your Authy's dashboard.
 * type[] = Choose your connection type between 'development' and 'production'.
 *
 * @returns false if failed, true if success.
*/
stock TFASAMP_prepareConnection(host[], password[], api_key[], type[] = "development") 
{
	if(isnull(host))
		return print("TFASAMP: 'host' is invalid'."), 0;
		
	if(isnull(password))
		return print("TFASAMP: 'password' is invalid'."), 0;
		
	if(isnull(api_key))
		return print("TFASAMP: 'api_key' is invalid'."), 0;
		
	strcat(TFASAMP_INTERNAL[HOSTNAME], host, 1024);
	strcat(TFASAMP_INTERNAL[PASSWORD], password, 128);
	strcat(TFASAMP_INTERNAL[API_KEY], api_key, 128);
	strcat(TFASAMP_INTERNAL[TYPE], type, 128);
	
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: 'hostname' = '%s'", TFASAMP_INTERNAL[HOSTNAME]);
		printf("TFASAMP_DEBUG: 'password' = '%s'", TFASAMP_INTERNAL[PASSWORD]);
		printf("TFASAMP_DEBUG: 'api_key' = '%s'", TFASAMP_INTERNAL[API_KEY]);
		printf("TFASAMP_DEBUG: 'type' = '%s'", TFASAMP_INTERNAL[TYPE]);
	#endif

	TFASAMP_INTERNAL[CONNECTION_PREPARED] = true;
	
	print("TFASAMP: Connection prepared, awaiting for commands.");	
	return 1;
}

/*
 * TFASAMP_addUser
 * This function adds a new user to your Authy application.
 *
 * playerid = playerid of the player you wish to add in your Authy application.
 * email[] = email of the player you wish to add in your Authy application.
 * cellphone[] = Cellphone number of the player you wish to add in your Authy application.
 * area_code[] = International cellphone number prefix - you may find them at www.countrycode.org under the 'Country Code' column.
 *
 * @returns true if the function has been properly executed, false if not.
 * Notes: the callback TFASAMP_addUser_response will give you the userid of the player to be used for the token verification.
*/
stock TFASAMP_addUser(playerid, email[], cellphone[], area_code[] = "1") 
{
	if(!TFASAMP_INTERNAL[CONNECTION_PREPARED])
		return print("TFASAMP: The connection is not prepared yet."), 0;
		
	if(TFASAMP_PLAYER[playerid][REQUESTING_API])
		return print("TFASAMP: playerid %d has always an API request in progress, wait please."), 0;
		
	new _string[1024];
	
	format(_string, sizeof(_string), "%screate.php?password=%s&api_key=%s&email=%s&cellphone=%s&area_code=%s", TFASAMP_INTERNAL[HOSTNAME], TFASAMP_INTERNAL[PASSWORD], TFASAMP_INTERNAL[API_KEY], email, cellphone, area_code);
	
	if(!strcmp(TFASAMP_INTERNAL[TYPE], "development"))
		format(_string, sizeof(_string), "%s&development=true", _string);
		
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: '_string' = '%s'", _string);
	#endif		
	
	TFASAMP_PLAYER[playerid][REQUESTING_API] = true;
	
	HTTP(playerid, HTTP_GET, _string, "", "TFASAMP_addUser_response");
	return 1;
}

/*
 * TFASAMP_verifyToken
 * This function checks a token if valid or invalid.
 *
 * playerid = playerid of the player you wish to check the token.
 * user_id[] = userid of the player you wish to check the token (check TFASAMP_addUser_response or your Authy's dashboard for clarifications).
 * token[] = token to be checked.
 * force[] (bool: true) = It's recommended to leave this true. If user has not finished registration any token always works.
 *
 * @returns true if the function has been properly executed, false if not.
 * Notes: the callback TFASAMP_verifyToken_response will give you the userid of the player to be used for the token verification.
 * Notes: also, 20 seconds after the confirmation the player has to verify the token again for security reasons.
 * Notes: check TFASAMP_verifyTokenTime for clarifications.
*/
stock TFASAMP_verifyToken(playerid, user_id, token[], bool: force = true)
{
	if(!TFASAMP_INTERNAL[CONNECTION_PREPARED])
		return print("TFASAMP: The connection is not prepared yet."), 0;
			
	if(TFASAMP_PLAYER[playerid][REQUESTING_API])
		return printf("TFASAMP: playerid %d has always an API request in progress, wait please.", playerid), 0;		
		
	new _string[1024];
	
	format(_string, sizeof(_string), "%scheck.php?password=%s&api_key=%s&userid=%d&token=%s", TFASAMP_INTERNAL[HOSTNAME], TFASAMP_INTERNAL[PASSWORD], TFASAMP_INTERNAL[API_KEY], user_id, token);
	
	if(!strcmp(TFASAMP_INTERNAL[TYPE], "development"))
		format(_string, sizeof(_string), "%s&development=true", _string);
		
	if(!force)
		format(_string, sizeof(_string), "%s&force=false", _string);
		
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: '_string' = '%s'", _string);
	#endif
	
	TFASAMP_PLAYER[playerid][REQUESTING_API] = true;
		
	HTTP(playerid, HTTP_GET, _string, "", "TFASAMP_verifyToken_response");
	return 1;
}

/*
 * TFASAMP_isTokenVerifiedValid
 * This function checks if the token has been verified within 20 seconds and if the user has been verified properly.
 *
 * playerid = playerid of the player you wish to check the token.
 *
 * @returns true if it has been verified within 20 seconds, otherwise it returns false.
*/
stock TFASAMP_isTokenVerifiedValid(playerid)
{
	if(!TFASAMP_INTERNAL[CONNECTION_PREPARED])
		return print("TFASAMP: The connection is not prepared yet."), 0;
	
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: 'TOKEN_CHECK' = '%d'", TFASAMP_PLAYER[playerid][TOKEN_CHECK]);
		printf("TFASAMP_DEBUG: 'LAST_CHECK' = '%d'", TFASAMP_PLAYER[playerid][LAST_CHECK]);
		printf("TFASAMP_DEBUG: 'gettime()-10' = '%d'", gettime()-10);
	#endif	
	
	if(TFASAMP_PLAYER[playerid][TOKEN_CHECK] == 1)
		if(TFASAMP_PLAYER[playerid][LAST_CHECK] > (gettime()-10))
			return true;
			
	return false;
}
/*
 * Callbacks
*/

forward TFASAMP_addUser_response(index, response_code, data[]);
public TFASAMP_addUser_response(index, response_code, data[])
{
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: 'index' = '%d'", index);
		printf("TFASAMP_DEBUG: 'response_code' = '%d'", response_code);
		printf("TFASAMP_DEBUG: 'data' = '%s'", data);
	#endif
	
	TFASAMP_PLAYER[index][REQUESTING_API] = false;
	
    if(!TFASAMP_IsNumeric(data))
		return printf("TFASAMP ERROR: %s", data), 0;
	else
		TFASAMP_PLAYER[index][USER_ID] = strval(data);	
	return 1;
}

forward TFASAMP_verifyToken_response(index, response_code, data[]);
public TFASAMP_verifyToken_response(index, response_code, data[])
{
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: 'index' = '%d'", index);
		printf("TFASAMP_DEBUG: 'response_code' = '%d'", response_code);
		printf("TFASAMP_DEBUG: 'data' = '%s'", data);
	#endif
	
	TFASAMP_PLAYER[index][REQUESTING_API] = false;
	
    if(!TFASAMP_IsNumeric(data))
		return printf("TFASAMP ERROR: %s", data), 0;
	else 
	{
		TFASAMP_PLAYER[index][LAST_CHECK] = gettime();
		TFASAMP_PLAYER[index][TOKEN_CHECK] = strval(data);
	}
	return 1;
}

stock TFASAMP_IsNumeric(const string[])
{
    for (new i = 0, j = strlen(string); i < j; i++)
    {
        if (string[i] > '9' || string[i] < '0')
        {
            return 0;
        }
    }
    return 1;
}