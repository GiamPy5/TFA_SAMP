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

if(!isset($_GET['password']))
	die('Password must be sent.');

if(!isset($_GET['api']))
	die('API key must be sent.');
	
require_once 'class/tfasamp.php';	

try
{
	CTFA_SAMP::connect($_GET['password'], $_GET['api'], 'production');
}
catch(Exception $error)
{
	echo $error->getMessage();
}

switch($_GET['command'])
{
	case 'create': 
	{
		$TFA_SAMP->createUser($_GET['email'], $_GET['cellphone'], $_GET['area_code']);
		break;
	}
	case 'check':
	{
		$TFA_SAMP->verifyToken($_GET['userid'], $_GET['token'], $_GET['settings']);
		break;
	}
	
	default: die('Invalid command.');
}