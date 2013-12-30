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

/*
 * NONE - Logs nothing.
 * NOTICE - Logs notices and errors.
 * ERROR - Logs only errors.
*/
abstract class reportingLevel
{
	const NONE = 0;
    const NOTICE = 1;
    const ERROR = 2;
}

class CTFA_SAMP 
{
	# This is the password required to communicate with TFA_SAMP.
	private static $allowedPassword = 'testing';
	
	# These are the addresses allowed to communicate with TFA_SAMP.
	private static $allowedAddress = array('127.0.0.2');
	
	# Here you may choose the reportingLevel between: $NONE, $NOTICE and $ERROR. If undefined or invalid, $ERROR will be set as default.
	private static $fileDebugging = reportingLevel::ERROR;
	
	private $password;
	private $API;
	private $connectionURL;
	
	private $connectionAllowed;
	
	private function __construct($password, $API, $connectionURL)
	{
		$this->password = $password;
		$this->API = $API;
		$this->connectionURL = $connectionURL;
	}
	
	public static function connect($password, $API, $connectionType)
	{
		if(self::$fileDebugging > 3 || self::$fileDebugging < 0)
			self::$fileDebugging = reportingLevel::ERROR;
			
		if(isset($_SERVER['HTTP_X_FORWARDED_FOR']) && $_SERVER['HTTP_X_FORWARTDED_FOR'] != '') 
		{
			if(!in_array($_SERVER['HTTP_X_FORWARDED_FOR'], self::$allowedAddress)) 
			{
				self::logAction(reportingLevel::ERROR, '(CTFA_SAMP::connect) Address ('. $_SERVER["HTTP_X_FORWARDED_FOR"] .') not allowed.');
				throw new InvalidArgumentException('(CTFA_SAMP::connect) Address ('. $_SERVER["HTTP_X_FORWARDED_FOR"] .') not allowed.');
			}
		} 
		else 
		{
			if(!in_array($_SERVER['REMOTE_ADDR'], self::$allowedAddress))
			{
				self::logAction(reportingLevel::ERROR, '(CTFA_SAMP::connect) Address ('. $_SERVER["REMOTE_ADDR"] .') not allowed.');
				throw new InvalidArgumentException('(CTFA_SAMP::connect) Address ('. $_SERVER["REMOTE_ADDR"] .') not allowed.');
			}
		}
		
		if(!isset($password)) 
		{
			self::logAction(reportingLevel::ERROR, '(CTFA_SAMP::connect) password is missing.');
			throw new BadMethodCallException('(CTFA_SAMP::connect) password is missing.');
		}
	
		if($password != self::$allowedPassword) 
		{
			self::logAction(reportingLevel::ERROR, '(CTFA_SAMP::connect) password ('. $password .') is invalid.');
			throw new BadMethodCallException('(CTFA_SAMP::connect) password ('. $password .') is invalid.');
		}
		
		if(!isset($connectionType))
			$connectionType = 'production';
		
		switch($connectionType)
		{
			case 'production': 
			{
				self::logAction(reportingLevel::NOTICE, '(TFASAMP::connect) Connected through production connection type.');
				return new CTFA_SAMP($password, $API, 'https://api.authy.com');
			}	
				
			case 'development':
			{
				self::logAction(reportingLevel::NOTICE, '(TFASAMP::connect) Connected through development connection type.');
				return new CTFA_SAMP($password, $API, 'http://sandbox-api.authy.com');
			}
			
			default: 
			{
				self::logAction(reportingLevel::ERROR, '(TFASAMP::connect) Connection type ('. $connectionType .') is invalid: must be "production" or "development".');
				throw new BadMethodCallException('(TFASAMP::connect) Connection type ('. $connectionType .') is invalid: must be "production" or "development".');
			}
		}		
	}
	
	public function createUser($email, $cellphone, $areaCode = 1)
	{	
		if(!isset($email))
		{
			self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->createUser) email is missing.');
			throw new BadMethodCallException('(CTFA_SAMP->createUser) email is missing.');
		}
		
		if(!isset($cellphone))
		{
			self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->createUser) cellphone is missing.');
			throw new BadMethodCallException('(CTFA_SAMP->createUser) cellphone is missing.');
		}

		$authyLibrary = new Authy_Api($this->API, $this->connectionURL);
		$requestResult = $authyLibrary->registerUser($email, $cellphone, intval($areaCode));
		
		if($requestResult->ok())
		{
			self::logAction(reportingLevel::NOTICE, '(CTFA_SAMP->createUser) user created [mail: '. $email .' - cellphone: '. $cellphone .' - areacode: '. $areaCode .' - id: '. $requestResult->id() .']');
			echo $requestResult->id();
		}
		else
		{
			foreach($requestResult->errors() as $field => $message)
			{
				self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->createUser) '. $field .' = ' . $message .'');
				echo "$field = $message";
			}
		}
	}	
	
	public function verifyToken($userID, $token, $settings = array('force' => true))
	{		
		if(!isset($userID))
		{
			self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->verifyToken) email is missing.');
			throw new BadMethodCallException('(CTFA_SAMP->verifyToken) userID is missing.');
		}
			
		if(!isset($token))
		{
			self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->verifyToken) email is missing.');
			throw new BadMethodCallException('(CTFA_SAMP->verifyToken) token is missing.');
		}
		
		if(!isset($settings) || !array_key_exists("force", $settings))
            $settings["force"] = 'true';
			
		$authyLibrary = new Authy_Api($this->API, $this->connectionURL);
		$requestResult = $authyLibrary->verifyToken(intval($userID), intval($token), $settings);
		
		if($requestResult->ok())
		{
			self::logAction(reportingLevel::NOTICE, '(CTFA_SAMP->verifyToken) token verified [userid: '. $userID .' - token: '. $token .']');
			echo '1';
		}
		else
		{
			foreach($requestResult->errors() as $field => $message)
			{
				self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->verifyToken) '. $field .' = ' . $message .'');
				echo "$field = $message";
			}
		}
	}
	
	private static function logAction($reportingLevel, $text)
	{
		if($reportingLevel > self::$fileDebugging)
			return false;
		
		if(isset($_SERVER['HTTP_X_FORWARDED_FOR']) && $_SERVER['HTTP_X_FORWARTDED_FOR'] != '') 
			$IP = $_SERVER['HTTP_X_FORWARDED_FOR'];
		else
			$IP = $_SERVER['REMOTE_ADDR'];			
			
		switch($reportingLevel)
		{
			case reportingLevel::NOTICE: 
			{
				file_put_contents('tfasamp_logs.txt', "[NOTICE - ". date('r', time()) ." - ". $IP ."] ". $text ."\r\n", FILE_APPEND);
				break;
			}				
			case reportingLevel::ERROR:
			{
				file_put_contents('tfasamp_logs.txt', "[ERROR - ". date('r', time()) ." - ". $IP ."] ". $text ."\r\n", FILE_APPEND);
				break;
			}				
		}
	}	
}

?>