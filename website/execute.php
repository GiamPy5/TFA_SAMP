<?php

/**
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
 * 
 * @copyright     Copyright (C) 2014 Giampaolo Falqui (https://github.com/GiampaoloFalqui/TFA_SAMP)
 * @link          https://github.com/GiampaoloFalqui/TFA_SAMP TFA_SAMP
 * @version       1.1.0
 * @license       http://opensource.org/licenses/GPL-2.0 GNU License
 */

if(! isset($_GET['password']))
  die('Password must be sent.');

if(! isset($_GET['api']))
  die('API key must be sent.');

if(! isset($_GET['command']))
  die('A command must be sent.');	
	
require_once 'class/tfasamp.php';	

$TFA_SAMP = null;

try {
  $args = $_GET + array('type' => null);
  $TFA_SAMP = CTFA_SAMP::connect($_GET['password'], $_GET['api'], $args['type']);
} catch(Exception $connectError) {
  echo $connectError->getMessage();
  exit;
}

switch($_GET['command']) {
  case 'create': {
    try {
      $args = $_GET + array('email' => null, 'cellphone' => null, 'area_code' => null);
      
      /** 
       * This is a demonstration of how it should work with the 'raw' return type.
       */      
      $result = $TFA_SAMP->createUser($args['email'], $args['cellphone'], $args['area_code'], 'raw');
      if ($result) {
        foreach($result as $key => $value) {
          echo "{$key} => {$value}";
        }
      }
      
      /** 
       * This is a demonstration of how it should work with the 'json' return type.
       */ 
      $result = $TFA_SAMP->createUser($args['email'], $args['cellphone'], $args['area_code'], 'json');
      if ($result) {
        echo $result;
      }              
      
    } catch(Exception $createError) {
      echo $createError->getMessage();
    }	
    break;
  }
  
  case 'check': {
    try {
      $args = $_GET + array('userid' => null, 'token' => null, 'settings' => null);
      
      /** 
       * This is a demonstration of how it should work with the 'raw' return type.
       */
      $result = $TFA_SAMP->verifyToken($args['userid'], $args['token'], $args['settings'], 'raw');
      if ($result) {
        foreach($result as $key => $value) {
          echo "{$key} => {$value}";
        }
      }
      
      /** 
       * This is a demonstration of how it should work with the 'json' return type.
       */      
      $result = $TFA_SAMP->verifyToken($args['userid'], $args['token'], $args['settings'], 'json');
      if ($result) {
        echo $result;
      }      
      
    } catch(Exception $checkError) {
      echo $checkError->getMessage();
    }		
    break;
  }
	
  default: die('Invalid command.');
}