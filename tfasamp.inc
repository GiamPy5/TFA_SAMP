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
 * - TFASAMP_prepareConnection(host[], password[], api_key[], type[] = "development", bool:tfa_debug = false)
 * - TFASAMP_createUser(playerid, email[], cellphone[], area_code[] = 1)
 * - TFASAMP_verifyToken(playerid, user_id, token, bool: force = true)
 * - TFASAMP_setPlayerUserID(playerid, userid)
 * - TFASAMP_getPlayerUserID(playerid)
 * - TFASAMP_getUUID(playerid)
 * - TFASAMP_setUUID(playerid, TFA_UUID:uuid[])
 * - TFASAMP_getNotification(playerid, TFA_UUID:uuid[])
 *
 * Available Callbacks:
 * - TFASAMP_OnTokenVerify(playerid, result)
 * - TFASAMP_OnPushNotification(playerid, TFA_UUID:uuid[])
 * - TFASAMP_OnCheckNotification(playerid, status)
*/

/*
 * Includes
*/

#include			<a_samp>
#include			<a_http>
#include			<YSI\y_stringhash>
#include			<YSI\y_hooks>
#include            <a_json>

forward public 		TFASAMP_OnTokenVerify(playerid, result);
forward public      TFASAMP_OnPushNotification(playerid, TFA_UUID:uuid[]);
forward public      TFASAMP_OnCheckNotification(playerid, status);
/*
 * Versioning
*/

#define 			TFASAMP_VERSION				"1.1.0"

/*
 * Macros
*/

#if !defined isnull
	#define 		isnull(%1) 					((%1[0] == 0) || (%1[0] == 1 && %1[1] == 0))		
#endif

#define 			TFASAMP_getHostname()		TFASAMP_INTERNAL[HOSTNAME]
#define 			TFASAMP_getPassword()		TFASAMP_INTERNAL[PASSWORD]
#define 			TFASAMP_getAPIKey()			TFASAMP_INTERNAL[API_KEY]
#define 			TFASAMP_getConnectionType()	TFASAMP_INTERNAL[TYPE]
#define 			TFASAMP_isDebugActive()		TFASAMP_INTERNAL[DEBUG_STATUS]

/*
	UUID's size 
*/
#define  MAX_UUID_SIZE 60

/*
 * Variables Declaration
*/

enum TFASAMP_E_INTERNAL
{
	HOSTNAME[1024],
	PASSWORD[128],
	API_KEY[128],
	TYPE[128],
	bool: DEBUG_STATUS,
	bool: CONNECTION_PREPARED,
};

enum TFASAMP_E_PLAYER
{
	AUTHY_USER_ID,
	TOKEN_CHECK_STATUS,
	LAST_CHECK_UNIX,
	AUTHY_UUID[MAX_UUID_SIZE],
	bool: IS_HTTP_PROCESSING,
};

static stock 
	TFASAMP_INTERNAL[TFASAMP_E_INTERNAL], 
	TFASAMP_PLAYER[MAX_PLAYERS][TFASAMP_E_PLAYER];
 
hook OnPlayerConnect(playerid)
{
	TFASAMP_PLAYER[playerid][AUTHY_USER_ID] = 0;
	TFASAMP_PLAYER[playerid][TOKEN_CHECK_STATUS] = 0;
	TFASAMP_PLAYER[playerid][LAST_CHECK_UNIX] = 0;
	format(TFASAMP_PLAYER[playerid][AUTHY_UUID], MAX_UUID_SIZE, "");
	TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING] = false;
	return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
	TFASAMP_PLAYER[playerid][AUTHY_USER_ID] = 0;
	TFASAMP_PLAYER[playerid][TOKEN_CHECK_STATUS] = 0;
	TFASAMP_PLAYER[playerid][LAST_CHECK_UNIX] = 0;
	format(TFASAMP_PLAYER[playerid][AUTHY_UUID], MAX_UUID_SIZE, "");
	TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING] = false;
	return 1;
}

/*
 * TFASAMP_prepareConnection
 * This function prepares the connection to your hosting in order to request the APIs.
 *
 * host[]     = Your webhosting link and the directory, if any (without http://).
 * password[] = The password to use the PHP files to communicate with the APIs.
 * api_key[]  = You may find it in your Authy's dashboard.
 * type[]     = Choose your connection type between 'development' and 'production'.
 *
 * @returns false if failed, true if success.
*/
stock TFASAMP_prepareConnection(hostname[], password[], api_key[], type[] = "development", bool: tfa_debug = false) 
{
	if (isnull(hostname)) {
		return printf("TFASAMP: 'hostname' (%s) is invalid.", hostname), false;
	}
		
	if (isnull(password)) {
		return printf("TFASAMP: 'password' (%s) is invalid.", password), false;
	}
		
	if (isnull(api_key)) {
		return printf("TFASAMP: 'api_key' (%s) is invalid.", api_key), false;
	}
		
	switch (YHash(type)) {
		case _H<production>:  strcat(TFASAMP_INTERNAL[TYPE], "production", 128);
		case _H<development>: strcat(TFASAMP_INTERNAL[TYPE], "development", 128);
		
		default: {
			printf("TFASAMP: 'type' (%s) is invalid.", type);
			return false;
		}
	}
	
	TFASAMP_INTERNAL[DEBUG_STATUS] = (tfa_debug) ? (true) : (false);
	
	strcat(TFASAMP_INTERNAL[HOSTNAME], hostname, 1024);
	strcat(TFASAMP_INTERNAL[PASSWORD], password, 128);
	strcat(TFASAMP_INTERNAL[API_KEY], api_key, 128);
	
	if(TFASAMP_isDebugActive()) {	
		print("(debug) TFASAMP_prepareConnection: Debug is active!");
		print("(debug) TFASAMP_prepareConnection: Version "TFASAMP_VERSION" started.");
		printf("(debug) TFASAMP_prepareConnection: 'hostname' = '%s'", TFASAMP_getHostname());
		printf("(debug) TFASAMP_prepareConnection: 'password' = '%s'", TFASAMP_getPassword());
		printf("(debug) TFASAMP_prepareConnection: 'api_key' = '%s'", TFASAMP_getAPIKey());
		printf("(debug) TFASAMP_prepareConnection: 'type' = '%s'", TFASAMP_getConnectionType());
	}
	
	TFASAMP_INTERNAL[CONNECTION_PREPARED] = true;
	
	print("TFASAMP: Connection prepared, awaiting for commands.");	
	return true;
}

/*
 * TFASAMP_createUser
 * This function adds a new user to your Authy application.
 *
 * playerid    = playerid of the player you wish to add in your Authy application.
 * email[]     = email of the player you wish to add in your Authy application.
 * cellphone[] = Cellphone number of the player you wish to add in your Authy application.
 * area_code[] = International cellphone number prefix - you may find them at www.countrycode.org under the 'Country Code' column.
 *
 * @returns true if the function has been properly executed, false if not.
 * Notes: the callback TFASAMP_createUser_response will give you the userid of the player to be used for the token verification.
*/
stock TFASAMP_createUser(playerid, email[], cellphone[], area_code[] = "1") 
{
	if (!TFASAMP_INTERNAL[CONNECTION_PREPARED]) {
		return print("TFASAMP: The connection is not prepared yet."), false;
	}
		
	if (TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING]) {
		return print("TFASAMP: playerid %d has always an API request in progress, wait please."), false;
	}
		
	new _string[1024];
	
	format(_string, sizeof(_string), "%sexecute.php?password=%s&command=create&api=%s&email=%s&cellphone=%s&area_code=%s", TFASAMP_getHostname(), TFASAMP_getPassword(), TFASAMP_getAPIKey(), email, cellphone, area_code);
	
	if (!strcmp(TFASAMP_INTERNAL[TYPE], "development")) {
		format(_string, sizeof(_string), "%s&type=development", _string);
	}
		
	if (TFASAMP_isDebugActive()) {
		printf("(debug) TFASAMP_createUser: '_string' = '%s'", _string);
	}
	
	TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING] = true;
	
	HTTP(playerid, HTTP_GET, _string, "", "TFASAMP_createUser_response");
	return true;
}

/*
 * TFASAMP_verifyToken
 * This function checks a token if valid or invalid.
 *
 * playerid             = playerid of the player you wish to check the token.
 * user_id[] 		    = userid of the player you wish to check the token (check TFASAMP_createUser_response or your Authy's dashboard for clarifications).
 * token[]              = token to be checked.
 * force[] (bool: true) = It's recommended to leave this true. If user has not finished registration any token always works.
 *
 * @returns true if the function has been properly executed, false if not.
 * Notes: the callback TFASAMP_verifyToken_response will give you the userid of the player to be used for the token verification.
 * Notes: also, 20 seconds after the confirmation the player has to verify the token again for security reasons.
 * Notes: check TFASAMP_verifyTokenTime for clarifications.
*/
stock TFASAMP_verifyToken(playerid, user_id, token[], bool: force = true)
{
	if (!TFASAMP_INTERNAL[CONNECTION_PREPARED]) {
		return print("TFASAMP: The connection is not prepared yet."), false;
	}
			
	if (TFASAMP_isHTTPProcessing(playerid)) {
		return printf("TFASAMP: playerid %d has already an API request in progress, wait please.", playerid), false;	
	}
		
	new _string[1024];
	
	format(_string, sizeof(_string), "%sexecute.php?password=%s&command=check&api=%s&userid=%d&token=%s", TFASAMP_getHostname(), TFASAMP_getPassword(), TFASAMP_getAPIKey(), user_id, token);
	
	if (!strcmp(TFASAMP_INTERNAL[TYPE], "development")) {
		format(_string, sizeof(_string), "%s&type=development", _string);
	}
		
	if (!force) {
		format(_string, sizeof(_string), "%s&settings[force]=false", _string);
	}
		
	if (TFASAMP_isDebugActive()) {
		printf("(debug) TFASAMP_debugStatus: '_string' = '%s'", _string);
	}
	
	TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING] = true;
		
	HTTP(playerid, HTTP_GET, _string, "", "TFASAMP_verifyToken_response");
	return true;
}

/*
 * TFASAMP_pushNotification
 * This function sends a notification to the client's device for authorization
 *
 * playerid             = playerid of the player you wish to check the token.
 * user_id 			    = userid of the player you wish to check the token (check TFASAMP_createUser_response or your Authy's dashboard for clarifications).
 *
 * @returns true if the function has been properly executed, false if not. 
 * Notes: This function will trigger the callback TFASAMP_OnPushNotification.
 * Notes: the callback TFASAMP_pushNotification will give you the UUID of the notification which can be used to check the status of the approval later via TFASAMP_getNotification()
 * Notes: check function TFASAMP_getNotification for clarification as to where the UUID needs to be used
*/

stock TFASAMP_pushNotification(playerid, user_id){

    if (!TFASAMP_INTERNAL[CONNECTION_PREPARED]) {
		return print("TFASAMP: The connection is not prepared yet."), false;
	}

	if (TFASAMP_isHTTPProcessing(playerid)) {
		return printf("TFASAMP: playerid %d has already an API request in progress, wait please.", playerid), false;
	}

	new	string[1024];
	format(string, sizeof string, "%sexecute.php?password=%s&command=pushNoti&api=%s&userid=%d", TFASAMP_getHostname(), TFASAMP_getPassword(), TFASAMP_getAPIKey(), user_id);

    TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING] = true;
   	HTTP(playerid, HTTP_GET, string, "", "TFASAMP_push_response");
   	return true;
}

/*
 * TFASAMP_getNotification
 * This function gets the status of the notification which was sent to the client's device
 *
 * playerid             = playerid of the player you wish to check the token.
 * uuid 			    = uuid of the player (check TFASAMP_pushNotification, when used the callback TFASAMP_OnPushNotification will return TFA_UUID tagged value)
 *
 * @returns true if the function has been properly executed, false if not.
 * Notes: the callback TFASAMP_OnGetNotification will @return true if the authorization has been approved otherwise false if not or failed.
*/

stock TFASAMP_getNotification(playerid, TFA_UUID:uuid[]){

    if (!TFASAMP_INTERNAL[CONNECTION_PREPARED]) {
		return print("TFASAMP: The connection is not prepared yet."), false;
	}

	if (TFASAMP_isHTTPProcessing(playerid)) {
		return printf("TFASAMP: playerid %d has already an API request in progress, wait please.", playerid), false;
	}

	new	string[1024];
	format(string, sizeof string, "%sexecute.php?password=%s&command=checkNoti&api=%s&uuid=%s", TFASAMP_getHostname(), TFASAMP_getPassword(), TFASAMP_getAPIKey(), _:uuid);

    TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING] = true;
   	HTTP(playerid, HTTP_GET, string, "", "TFASAMP_check_response");
   	return 1;
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
{
	if (!TFASAMP_INTERNAL[CONNECTION_PREPARED]) {
		return print("TFASAMP: The connection is not prepared yet."), false;
	}
		
	return TFASAMP_PLAYER[playerid][IS_HTTP_PROCESSING];
}
	
/*
 * TFASAMP_setPlayerUserID
 * This function sets the Authy's userid to a playerid.
 *
 * playerid = playerid of the player you wish to set.
 * user_id[] = authy userid you wish to set.
 *
 * @returns true if the connection is prepared, otherwise false.
*/
stock TFASAMP_setPlayerUserID(playerid, userid) 
{
	if (!TFASAMP_INTERNAL[CONNECTION_PREPARED]) {
		return print("TFASAMP: The connection is not prepared yet."), false;
	}
		
	TFASAMP_PLAYER[playerid][AUTHY_USER_ID] = userid;
	return true;
}

/*
 * TFASAMP_getPlayerUserID
 * This function retrieves the Authy's userid of a playerid.
 *
 * playerid = playerid of the player you wish to know the authy userid.
 *
 * @returns the authy userid of the playerid if the connection is prepared, otherwise false.
*/
stock TFASAMP_getPlayerUserID(playerid) 
{
	if (!TFASAMP_INTERNAL[CONNECTION_PREPARED]) {
		return print("TFASAMP: The connection is not prepared yet."), false;
	}
		
	return TFASAMP_PLAYER[playerid][AUTHY_USER_ID];
}

stock TFASAMP_setUUID(playerid, TFA_UUID:_uuid[]){
	format(TFASAMP_PLAYER[playerid][AUTHY_UUID], MAX_UUID_SIZE, "%s", _:_uuid);
	return 1;
}

stock TFASAMP_getUUID(playerid, TFA_UUID:uuid_[], size = MAX_UUID_SIZE){
	strmid(_:uuid_, TFASAMP_PLAYER[playerid][AUTHY_UUID], 0, size, size);
	return 1;
}

forward TFASAMP_createUser_response(index, response_code, data[]);
public TFASAMP_createUser_response(index, response_code, data[])
{
	if(TFASAMP_isDebugActive()) {
		printf("(debug) TFASAMP_createUser_response: 'index' = '%d'", index);
		printf("(debug) TFASAMP_createUser_response: 'response_code' = '%d'", response_code);
		printf("(debug) TFASAMP_createUser_response: 'data' = '%s'", data);
	}
	
	TFASAMP_PLAYER[index][IS_HTTP_PROCESSING] = false;

	new JSONNode:jsonData;
	jsonData = json_parse_string(data);
	new result;
	result = json_get_int(jsonData, "userid");
	
	if(!result){
		printf("(ERROR) TFASAMP_createUser_response: User not created!");
	}
	return result;
}

forward TFASAMP_verifyToken_response(index, response_code, data[]);
public TFASAMP_verifyToken_response(index, response_code, data[])
{
	if(TFASAMP_isDebugActive()) {
		printf("(debug) TFASAMP_verifyToken_response: 'index' = '%d'", index);
		printf("(debug) TFASAMP_verifyToken_response: 'response_code' = '%d'", response_code);
		printf("(debug) TFASAMP_verifyToken_response: 'data' = '%s'", data);
	}
	
	TFASAMP_PLAYER[index][IS_HTTP_PROCESSING] = false;
	
	new JSONNode:jsonData;
	jsonData = json_parse_string(data);
	new result[15];
	json_get_string(jsonData, result, sizeof result, "success");

	if(!strcmp(result, "true", true) && !isnull(result)){
   		CallLocalFunction("TFASAMP_OnTokenVerify", "dd", index, 1);
		TFASAMP_PLAYER[index][LAST_CHECK_UNIX] = gettime();
		TFASAMP_PLAYER[index][TOKEN_CHECK_STATUS] = 1;
	}
	else{
		CallLocalFunction("TFASAMP_OnTokenVerify", "dd", index, 0);
	    printf("TFASAMP ERROR: %s", data);
	    TFASAMP_PLAYER[index][TOKEN_CHECK_STATUS] = 0;
	    return 0;
	}

	return 1;
}

forward TFASAMP_push_response(index, response_code, data[]);
public TFASAMP_push_response(index, response_code, data[]){

	if(TFASAMP_isDebugActive()) {
		printf("(debug) TFA_push_response: 'index' = '%d'", index);
		printf("(debug) TFA_push_response: 'response_code' = '%d'", response_code);
		printf("(debug) TFA_push_response: 'data' = '%s'", data);
	}

	TFASAMP_PLAYER[index][IS_HTTP_PROCESSING] = false;

	new JSONNode:jsonData;
	jsonData = json_parse_string(data);
	new result[128];
	json_get_string(jsonData, result, sizeof result, "success");

	if(!strcmp(result, "true", true)){
       	json_get_string(jsonData, result, sizeof result, "uuid");
		CallLocalFunction("TFASAMP_OnPushNotification", "ds", index, result);
	}
	else{
		CallLocalFunction("TFASAMP_OnPushNotification", "ds", index, "NULL");
	}
}

forward TFASAMP_check_response(index, response_code, data[]);
public TFASAMP_check_response(index, response_code, data[]){

	if(TFASAMP_isDebugActive()) {
		printf("(debug) TFA_check_response: 'index' = '%d'", index);
		printf("(debug) TFA_check_response: 'response_code' = '%d'", response_code);
		printf("(debug) TFA_check_response: 'data' = '%s'", data);
	}

	TFASAMP_PLAYER[index][IS_HTTP_PROCESSING] = false;

	new JSONNode:jsonData;
	jsonData = json_parse_string(data);
	new result[128];
	json_get_string(jsonData, result, sizeof result, "status");

	if(!strcmp(result, "approved", true)){
		CallLocalFunction("TFASAMP_OnCheckNotification", "dd", index, 1);
	}
	else{
		CallLocalFunction("TFASAMP_OnCheckNotification", "dd", index, 0);
	}
}