<?php

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

if(isset($_GET['api_key'])) 
	$TFA_SAMP['authy_api'] = (isset($_GET['development']) && $_GET['development'] == 'true') ? new Authy_Api(htmlspecialchars($_GET['api_key']), 'http://sandbox-api.authy.com') : new Authy_Api(htmlspecialchars($_GET['api_key']), 'https://api.authy.com');
else 
	die("'api_key' is invalid.");
	
if(!isset($_GET['userid'])) 
	die("'userid' is missing.");
	
if(!isset($_GET['token'])) 
	die("'token' is missing.");

$verification = (!isset($_GET['force'])) ? $TFA_SAMP['authy_api']->verifyToken(htmlspecialchars($_GET['userid']), htmlspecialchars($_GET['token']), array('force' => 'true')) : $TFA_SAMP['authy_api']->verifyToken(htmlspecialchars($_GET['userid']), htmlspecialchars($_GET['token']), array('force' => 'false'));

if($verification->ok()) 
	die('1');
else
{
	foreach($verification->errors() as $field => $message) {
		die("$field = $message");
	}
}

?>