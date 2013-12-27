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

if(!isset(htmlspecialchars($_GET['password']) || htmlspecialchars($_GET['password']) != $TFA_SAMP['password']) 
	die("The password is wrong or not valid.");	
	
if(!isset($_GET['command'])) 
	die("No command has been selected.");
	
if(isset($_GET['command']) && !file_exists('commands/' . htmlspecialchars($_GET['command']) . '.php'))
	die("The selected command does not exist.");	
else 
	include('commands/' . htmlspecialchars($_GET['command']) . '.php');
