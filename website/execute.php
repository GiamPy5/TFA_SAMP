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

require 'internal/settings.php';
require 'Authy/Authy.php';

if(!isset($_GET['password']))
	die("The password has not been set.");

if(htmlspecialchars($_GET['password']) != $TFA_SAMP['password']) 
	die("The password is invalid.");	
	
$aFiles = array_values(array_diff(scandir('commands'), array('..', '.')));

for($i = 0; $i < count($aFiles); $i++)
	$aFiles[$i] = substr($aFiles[$i], 0, strrpos($aFiles[$i], "."));

if(!isset($_GET['command'])) 
{
	echo("No command has been selected.<br/>");		
	echo("The following commands are available:");		
	echo('<ul type="disc">');
	
	for($i = 0; $i < count($aFiles); $i++)
		echo('<li>' . $aFiles[$i] . '</li>');

	echo('</ul>');
	exit();
}

if(!in_array(htmlspecialchars($_GET['command']), $aFiles)) 
{
	echo("The selected command does not exist.<br/>");
	echo("The following commands are available:");		
	echo('<ul type="disc">');
	
	for($i = 0; $i < count($aFiles); $i++)
		echo('<li>' . $aFiles[$i] . '</li>');

	echo('</ul>');	
}
else 
	include('commands/' . htmlspecialchars($_GET['command']) . '.php');