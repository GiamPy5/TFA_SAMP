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
 * - TFASAMP_verifyTokenTime(playerid);
*/

/*
 * Includes
*/

#include 		<a_samp>

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
};

new TFASAMP_INTERNAL[TFASAMP_internalInformation];
new TFASAMP_PLAYER[MAX_PLAYERS][TFASAMP_playerInformation];
 
/*
 * Functions
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

stock TFASAMP_addUser(playerid, email[], cellphone[], area_code[] = "1") 
{
	if(!TFASAMP_INTERNAL[CONNECTION_PREPARED])
		return print("TFASAMP: The connection is not prepared yet."), 0;
		
	new _string[1024];
	
	format(_string, sizeof(_string), "%screate.php?password=%s&api_key=%s&email=%s&cellphone=%s&area_code=%s", TFASAMP_INTERNAL[HOSTNAME], TFASAMP_INTERNAL[PASSWORD], TFASAMP_INTERNAL[API_KEY], email, cellphone, area_code);
	
	if(!strcmp(TFASAMP_INTERNAL[TYPE], "development"))
		format(_string, sizeof(_string), "%s&development=true", _string);
		
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: '_string' = '%s'", _string);
	#endif		
	
	HTTP(playerid, HTTP_GET, _string, "", "TFASAMP_addUser_response");
	return 1;
}

stock TFASAMP_verifyToken(playerid, user_id[], token[], bool: force = true)
{
	if(!TFASAMP_INTERNAL[CONNECTION_PREPARED])
		return print("TFASAMP: The connection is not prepared yet."), 0;
		
	new _string[1024];
	
	format(_string, sizeof(_string), "%scheck.php?password=%s&api_key=%s&userid=%s&token=%s", TFASAMP_INTERNAL[HOSTNAME], TFASAMP_INTERNAL[PASSWORD], TFASAMP_INTERNAL[API_KEY], user_id, token);
	
	if(!strcmp(TFASAMP_INTERNAL[TYPE], "development"))
		format(_string, sizeof(_string), "%s&development=true", _string);
		
	if(!force)
		format(_string, sizeof(_string), "%s&force=false", _string);
		
	#if TFASAMP_DEBUG == 1
		printf("TFASAMP_DEBUG: '_string' = '%s'", _string);
	#endif		
		
	HTTP(playerid, HTTP_GET, _string, "", "TFASAMP_verifyToken_response");
	return 1;
}

stock TFASAMP_verifyTokenTime(playerid)
{
	if(!TFASAMP_INTERNAL[CONNECTION_PREPARED])
		return print("TFASAMP: The connection is not prepared yet."), 0;
	
	if((TFASAMP_PLAYER[index][LAST_CHECK]+20) > time())
		return false;
	else
		return true;
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
	
    if(!TFASAMP_IsNumeric(data))
		return printf("TFASAMP ERROR: %s", data), 0;
	else 
	{
		TFASAMP_PLAYER[index][LAST_CHECK] = time();
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