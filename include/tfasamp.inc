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
 * - TFASAMP_prepareConnection(host[], password[], api_key[], type[] = "development", bool:debug = false)
 * - TFASAMP_addUser(playerid, email[], cellphone[], area_code[] = 1)
 * - TFASAMP_verifyToken(playerid, user_id, token, bool: force = true)
 * - TFASAMP_isTokenVerifiedValid(playerid)
 * - TFASAMP_setPlayerUserID(playerid, userid)
 * - TFASAMP_getPlayerUserID(playerid)
*/

/*
 * Versioning
*/

#define 	TFA_SAMP_VERSION		"0.2.0"

/*
 * Includes
*/

#tryinclude			<a_samp>
#if !defined _inc_a_samp
	#error			You need a_samp.inc to run tfasamp.inc.
#endif

#tryinclude			<a_http>
#if !defined _inc_a_http
	#error			You need a_http.inc to run tfasamp.inc.
#endif

#tryinclude			<YSI\y_stringhash>
#if !defined _inc_y_stringhash
	#error			You need YSI\y_stringhash to run tfasamp.inc.
#endif

#tryinclude			<YSI\y_hooks>
#if !defined _inc_y_hooks
	#error			You need YSI\y_hooks to run tfasamp.inc.
#endif

/*
 * Macros
*/

#if !defined isnull
	#define isnull(%1) ((%1[0] == 0) || (%1[0] == 1 && %1[1] == 0))		
#endif

/*
 * Variables Declaration
*/

// Backward-compatibility define
#define 	TFASAMP_verifyTokenTime(%0) TFASAMP_isTokenVerifiedValid(%0)

enum TFASAMP_internalInformation {
	HOSTNAME[1024],
	PASSWORD[128],
	API_KEY[128],
	TYPE[128],
	bool: DEBUG_STATUS,
	bool: CONNECTION_PREPARED,
};

enum TFASAMP_playerInformation {
	USER_ID,
	TOKEN_CHECK,
	LAST_CHECK,
	bool: REQUESTING_API,
};

static TFASAMP_INTERNAL[TFASAMP_internalInformation];
static TFASAMP_PLAYER[MAX_PLAYERS][TFASAMP_playerInformation];
 
/*
 * Hooks
*/ 
 
hook OnPlayerConnect(playerid)
{
	TFASAMP_PLAYER[playerid][USER_ID] = 0;
	TFASAMP_PLAYER[playerid][TOKEN_CHECK] = 0;
	TFASAMP_PLAYER[playerid][LAST_CHECK] = 0;
	TFASAMP_PLAYER[playerid][REQUESTING_API] = false;
	return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
	TFASAMP_PLAYER[playerid][USER_ID] = 0;
	TFASAMP_PLAYER[playerid][TOKEN_CHECK] = 0;
	TFASAMP_PLAYER[playerid][LAST_CHECK] = 0;
	TFASAMP_PLAYER[playerid][REQUESTING_API] = false;
	return 1;
}
 
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
stock TFASAMP_prepareConnection(hostname[], password[], api_key[], type[] = "development", bool: tfa_debug = false) 
{
	if(isnull(hostname))
		return printf("TFASAMP: 'hostname' (%s) is invalid.", hostname), 0;
		
	if(isnull(password))
		return printf("TFASAMP: 'password' (%s) is invalid.", password), 0;
		
	if(isnull(api_key))
		return printf("TFASAMP: 'api_key' (%s) is invalid.", api_key), 0;
		
	switch(YHash(type))
	{
		case _H<production>: strcat(TFASAMP_INTERNAL[TYPE], "production", 128);
		case _H<development>: strcat(TFASAMP_INTERNAL[TYPE], "development", 128);
		
		default: 
		{
			printf("TFASAMP: 'type' (%s) is invalid.", type);
			return 0;
		}
	}
	
	TFASAMP_INTERNAL[DEBUG_STATUS] = (tfa_debug) ? (true) : (false);
	
	strcat(TFASAMP_INTERNAL[HOSTNAME], hostname, 1024);
	strcat(TFASAMP_INTERNAL[PASSWORD], password, 128);
	strcat(TFASAMP_INTERNAL[API_KEY], api_key, 128);
	
	if(TFASAMP_INTERNAL[DEBUG_STATUS])
	{
		
		print("(debug) TFASAMP_prepareConnection: Debug is active!");
		print("(debug) TFASAMP_prepareConnection: Version "TFA_SAMP_VERSION" started.");
		printf("(debug) TFASAMP_prepareConnection: 'hostname' = '%s'", TFASAMP_INTERNAL[HOSTNAME]);
		printf("(debug) TFASAMP_prepareConnection: 'password' = '%s'", TFASAMP_INTERNAL[PASSWORD]);
		printf("(debug) TFASAMP_prepareConnection: 'api_key' = '%s'", TFASAMP_INTERNAL[API_KEY]);
		printf("(debug) TFASAMP_prepareConnection: 'type' = '%s'", TFASAMP_INTERNAL[TYPE]);
	}
	
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
	
	format(_string, sizeof(_string), "%sexecute.php?password=%s&command=create&api_key=%s&email=%s&cellphone=%s&area_code=%s", TFASAMP_INTERNAL[HOSTNAME], TFASAMP_INTERNAL[PASSWORD], TFASAMP_INTERNAL[API_KEY], email, cellphone, area_code);
	
	if(!strcmp(TFASAMP_INTERNAL[TYPE], "development"))
		format(_string, sizeof(_string), "%s&development=true", _string);
		
	if(TFASAMP_INTERNAL[DEBUG_STATUS])
		printf("(debug) TFASAMP_addUser: '_string' = '%s'", _string);
	
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
			
	if(TFASAMP_isHTTPProcessing(playerid))
		return printf("TFASAMP: playerid %d has already an API request in progress, wait please.", playerid), 0;		
		
	new _string[1024];
	
	format(_string, sizeof(_string), "%sexecute.php?password=%s&command=check&api_key=%s&userid=%d&token=%s", TFASAMP_INTERNAL[HOSTNAME], TFASAMP_INTERNAL[PASSWORD], TFASAMP_INTERNAL[API_KEY], user_id, token);
	
	if(!strcmp(TFASAMP_INTERNAL[TYPE], "development"))
		format(_string, sizeof(_string), "%s&development=true", _string);
		
	if(!force)
		format(_string, sizeof(_string), "%s&force=false", _string);
		
	if(TFASAMP_INTERNAL[DEBUG_STATUS])
		printf("(debug) TFASAMP_debugStatus: '_string' = '%s'", _string);
	
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
	
	if(TFASAMP_INTERNAL[DEBUG_STATUS])
	{
		printf("(debug) TFASAMP_isTokenVerifiedValid: 'TOKEN_CHECK' = '%d'", TFASAMP_PLAYER[playerid][TOKEN_CHECK]);
		printf("(debug) TFASAMP_isTokenVerifiedValid: 'LAST_CHECK' = '%d'", TFASAMP_PLAYER[playerid][LAST_CHECK]);
		printf("(debug) TFASAMP_isTokenVerifiedValid: 'gettime()-20' = '%d'", gettime()-20);
	}
	
	if(TFASAMP_PLAYER[playerid][TOKEN_CHECK] == 1) 
		if(TFASAMP_PLAYER[playerid][LAST_CHECK] > (gettime()-20)) 
			return true;
		else
		{
			TFASAMP_PLAYER[playerid][TOKEN_CHECK] = 0;
			TFASAMP_PLAYER[playerid][LAST_CHECK] = 0;
			return false;
		}
		
	return false;
}

/*
 * TFASAMP_isHTTPProcessing
 * This function checks if the HTTP server is still processing the request.
 *
 * playerid = playerid of the player you wish to check.
 *
 * @returns true if it's still processing, otherwise false.
*/
stock TFASAMP_isHTTPProcessing(playerid)
	return TFASAMP_PLAYER[playerid][REQUESTING_API];
	
/*
 * TFASAMP_setPlayerUserID
 * This function sets the Authy's userid to a playerid.
 *
 * playerid = playerid of the player you wish to set.
 * user_id[] = authy userid you wish to set.
*/
stock TFASAMP_setPlayerUserID(playerid, userid) 
{
	TFASAMP_PLAYER[playerid][USER_ID] = userid;
}

/*
 * TFASAMP_getPlayerUserID
 * This function sets the Authy's userid to a playerid.
 *
 * playerid = playerid of the player you wish to know the authy userid.
 *
 * @returns the authy userid of the playerid.
*/
stock TFASAMP_getPlayerUserID(playerid) 
	return TFASAMP_PLAYER[playerid][USER_ID];

/*
 * Internal Functions
*/

/*
stock TFASAMP_get(parameter[])
{
	switch(YHash(parameter))
	{
		case _H<hostname>: return TFASAMP_INTERNAL[HOSTNAME];
		case _H<password>: return TFASAMP_INTERNAL[PASSWORD];
		case _H<apikey>: return TFASAMP_INTERNAL[API_KEY];
		case _H<connectiontype>: return TFASAMP_INTERNAL[TYPE];
	}
	return false;
}
*/

/*
 * Callbacks
*/

forward TFASAMP_addUser_response(index, response_code, data[]);
public TFASAMP_addUser_response(index, response_code, data[])
{
	if(TFASAMP_INTERNAL[DEBUG_STATUS])
	{
		printf("(debug) TFASAMP_addUser_response: 'index' = '%d'", index);
		printf("(debug) TFASAMP_addUser_response: 'response_code' = '%d'", response_code);
		printf("(debug) TFASAMP_addUser_response: 'data' = '%s'", data);
	}
	
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
	if(TFASAMP_INTERNAL[DEBUG_STATUS])
	{
		printf("(debug) TFASAMP_verifyToken_response: 'index' = '%d'", index);
		printf("(debug) TFASAMP_verifyToken_response: 'response_code' = '%d'", response_code);
		printf("(debug) TFASAMP_verifyToken_response: 'data' = '%s'", data);
	}
	
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