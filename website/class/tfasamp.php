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

require_once 'Authy/Authy.php';

class CTFA_SAMP 
{
	private static $allowedPassword = 'testing';
	private static $allowedAddress = array('127.0.0.1');
	
	private $password;
	private $API;
	private $connectionURL;
	
	private $connectionAllowed;
	
	private function __construct($password, $API, $connectionURL)
	{
		$this->password = $password;
		$this->api = $api;
		$this->connectionURL = $connectionURL;
	}
	
	public static function connect($password, $API, $connectionType = 'production')
	{
		if(!isset($password))
			throw new BadMethodCallException('Password is missing');
	
		if($password != self::$password)
			throw new BadMethodCallException('Password is invalid');
			
		if(isset($_SERVER['HTTP_X_FORWARDED_FOR']) && $_SERVER['HTTP_X_FORWARTDED_FOR'] != '') 
		{
			if(!in_array($_SERVER['HTTP_X_FORWARDED_FOR'], $this->allowedAddress))
				throw new InvalidArgumentException('Address not allowed');
		} 
		else 
		{
			if(!in_array($_SERVER['REMOTE_ADDR'], $this->allowedAddress))
				throw new InvalidArgumentException('Address not allowed');
		}
		
		switch($connectionType)
		{
			case 'production': 
				return new CTFA_SAMP($password, $API, 'https://api.authy.com');
			
			case 'development':
				return new CTFA_SAMP($password, $API, 'http://sandbox-api.authy.com');
			
			default: 
				throw new BadMethodCallException('Connection type is invalid: must be "production" or "development".');
		}		
	}
	
	function createUser($email, $cellphone, $areaCode = 1)
	{	
		if(!isset($email))
			return false;
			
		if(!isset($cellphone))
			return false;

		$authyLibrary = new Authy_Api($this->API, $this->connectionURL);
		$requestResult = $authyLibrary->registerUser(htmlspecialchars($email), htmlspecialchars($cellphone), htmlspecialchars($areaCode));
		
		if($requestResult->ok()) 
		{
			echo $requestResult->id();
		}
		else
		{
			foreach($requestResult->errors() as $field => $message) {
				echo("$field = $message");
			}
		}
	}	
	
	function verifyToken($userID, $token, $settings = array())
	{		
		if(!isset($userID))
			return false;
			
		if(!isset($token))
			return false;
			
        if(!array_key_exists("force", $settings))
            $settings["force"] = 'true';

		$authyLibrary = new Authy_Api($this->API, $this->connectionURL);
		$requestResult = $authyLibrary->verifyToken(htmlspecialchars($userID), htmlspecialchars($token), $settings);
		
		if($requestResult->ok())
			echo '1';
		else
		{
			foreach($requestResult->errors() as $field => $message) {
				echo("$field = $message");
			}
		}
	}
}

?>