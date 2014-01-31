<?php 

/**
 * The first Two-Factor Authentication Method for San Andreas Multiplayer.
 * Copyright (C) 2014 Giampaolo Falqui
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
 * @version       1.0.0
 * @license       http://www.opensource.org/licenses/mit-license.php MIT License
 */

require_once 'Authy/Authy.php';

/**
 * The abstract class reportingLevel contains the error constants that define the error levels.
 */
abstract class reportingLevel
{
  /**
   * It reports nothing from the system.
   * 
   * @var integer
   */
  const NONE = 0;
    
  /**
   * It reports only errors of the system.
   * 
   * @var integer
   */    
  const ERROR = 1;
  
  /**
   * It reports errors and notices of the system.
   * 
   * @var integer
   */
  const NOTICE = 2;
}

class CTFA_SAMP 
{ 
  /** 
   * This is the password required to communicate with TFA_SAMP.
   * 
   * @var      string
   * @access   protected
   */
  private static $__allowedPassword = 'testing';
	
  /**
   * These are the addresses allowed to communicate with TFA_SAMP.
   * 
   * @var      string
   * @access   private
   */
  private static $__allowedAddress = array('127.0.0.1');
	
  /**
   * Here you may choose the reportingLevel between: $NONE, $NOTICE and $ERROR. If undefined or invalid, $ERROR will be set as default.
   * 
   * @var      string
   * @access   private
   */
  private static $__fileDebugging = reportingLevel::ERROR;
  
  /**
   * The directory / file name for logging.
   * 
   * @var      string
   * @access   private
   */
  private static $__logFile = 'tfasamp_logs.txt';  
  
  /**
   * The IP address of the user that is connected.
   * 
   * @var      string
   * @access   private
   */
  private static $__userIP = null;  
	
  /**
   * Password selected by the user.
   * 
   * @var      string
   * @access   private
   */
  private $__password = null;
  
  /**
   * API selected by the user.
   * 
   * @var      string
   * @access   private
   */
  private $__API = null;
  
  /**
   * Connection URL selected by the system from user's API selection.
   * 
   * @var      string
   * @access   private
   */
  private $__connectionURL = null;
	
  /**
   * The constructor is private because we use the static method "connect" to initialize the object and make the internal checks.
   * 
   * @param string $password The password sent from the static connect method.
   * @param string $API The API sent from the static connect method.
   * @param string $connectionURL The connection URL chosen by the system depending on the API sent from the static connect method.
   * 
   * @access private
   * @return void
   */
  private function __construct($password, $API, $connectionURL)
  {
    $this->__password = $password;
    $this->__API = $API;
    $this->__connectionURL = $connectionURL;
  }
	
  /**
   * The static method "connect" works as 'class constructor' even if technically it is not. 
   * It checks if the security parameters and function arguments are correct before returning the class istance.
   * 
   * @param string $password The password selected by the user.
   * @param string $API The API selected by the user.
   * @param string $connectionType (optional) The connection type selected by the user, default to production.
   * 
   * @throws InvalidArgumentException If connection is not allowed.
   * @throws BadFunctionCallException If arguments are invalid.
   * 
   * @access public
   * @return mixed If successful, an instance of the class is returned, otherwise an exception is thrown.
   */
  public static function connect($password, $API, $connectionType = "production")
  {
    if(self::$__fileDebugging > 3 || self::$__fileDebugging < 0) {
      self::$__fileDebugging = reportingLevel::ERROR;
    }
      
    if(isset($_SERVER['HTTP_X_FORWARDED_FOR']) && $_SERVER['HTTP_X_FORWARTDED_FOR'] != '') {
      self::$__userIP = $_SERVER['HTTP_X_FORWARDED_FOR'];
      if(! in_array(self::$__userIP, self::$__allowedAddress)) {
        self::logAction(reportingLevel::ERROR, "(CTFA_SAMP::connect) Address ({$_SERVER["HTTP_X_FORWARDED_FOR"]}) not allowed.");
        throw new InvalidArgumentException("(CTFA_SAMP::connect) Address ({$_SERVER["HTTP_X_FORWARDED_FOR"]}) not allowed.");
      }
    } else {
      self::$__userIP = $_SERVER['REMOTE_ADDR'];
      if(! in_array(self::$__userIP, self::$__allowedAddress)) {	  		          
        self::logAction(reportingLevel::ERROR, "(CTFA_SAMP::connect) Address ({$_SERVER["REMOTE_ADDR"]}) not allowed.");
        throw new InvalidArgumentException("(CTFA_SAMP::connect) Address ({$_SERVER["REMOTE_ADDR"]}) not allowed.");
      }
    }
    
    if(! $password) {
      self::logAction(reportingLevel::ERROR, '(CTFA_SAMP::connect) password is missing.');
      throw new BadFunctionCallException('(CTFA_SAMP::connect) password is missing.');
    }
    
    if($password != self::$__allowedPassword) {
      self::logAction(reportingLevel::ERROR, "(CTFA_SAMP::connect) password ({$password}) is invalid.");
      throw new InvalidArgumentException("(CTFA_SAMP::connect) password ({$password}) is invalid.");
    }
    
    if($connectionType === null) {
      $connectionType = "production";
    }    
    
    switch($connectionType) {
      case 'production': {
        self::logAction(reportingLevel::NOTICE, '(TFASAMP::connect) Connected through production connection type.');
        return new CTFA_SAMP($password, $API, 'https://api.authy.com');
      }					
      case 'development': {
        self::logAction(reportingLevel::NOTICE, '(TFASAMP::connect) Connected through development connection type.');
        return new CTFA_SAMP($password, $API, 'http://sandbox-api.authy.com');
      }			
      default: {
        self::logAction(reportingLevel::ERROR, "(TFASAMP::connect) Connection type ({$connectionType}) is invalid: must be 'production' or 'development'.");
        throw new BadFunctionCallException("(TFASAMP::connect) Connection type ({$connectionType}) is invalid: must be 'production' or 'development'.");
      }
    }		
  }
	
  /**
   * The createUser method creates a user in the Authy's database.
   * 
   * @param string  $email      The email of the user.
   * @param string  $cellphone  The cellphone of the user.
   * @param integer $areacode   The areacode of the user's cellphone.
   * @param string  $returnType For convenience a return type has been added. You can decide between 'json' and 'raw'.
   * 
   * @throws InvalidArgumentException If arguments are invalid.
   * 
   * @access public
   * @return mixed If successful, the new user ID is returned, otherwise an array containing the errors is returned.
   * 
   * @link http://www.bennetyee.org/ucsd-pages/area.html For more information on area codes.
   */
  public function createUser($email, $cellphone, $areaCode = 1, $returnType = 'raw') {			
    if(! $email) {
      self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->createUser) email is missing.');
      throw new InvalidArgumentException('(CTFA_SAMP->createUser) email is missing.');
    }
    
    if(! $cellphone) {
      self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->createUser) cellphone is missing.');
      throw new InvalidArgumentException('(CTFA_SAMP->createUser) cellphone is missing.');
    }
    
    $authyLibrary = new Authy_Api($this->__API, $this->__connectionURL);
    $requestResult = $authyLibrary->registerUser($email, $cellphone, intval($areaCode));

    $result = null;   
    if($requestResult->ok()) {
      self::logAction(reportingLevel::NOTICE, "(CTFA_SAMP->createUser) user created [mail: '{$email}' - cellphone: '{$cellphone}' - areacode: '{$areaCode}' - id: '{$requestResult->id()}']");
      $result['userid'] = $requestResult->id();
    } else {
      foreach($requestResult->errors() as $field => $message) {
        self::logAction(reportingLevel::ERROR, "(CTFA_SAMP->createUser) {$field} = {$message}");
      }
      $result = $requestResult->errors();
    }
    
    if ($returnType === 'json') {
      if (version_compare(PHP_VERSION, '5.4.0', '>=')) {
        return json_encode($result, JSON_PRETTY_PRINT);
      }
      return json_encode($result);
    } else {
      return $result['userid'];
    }
  }
	
  /**
   * The verifyToken methond verifies the user token through an Authy API call.
   * 
   * @param integer $userID The ID of the user that needs to be verified.
   * @param integer $token The token that needs to be verified.
   * @param array   $settings (optional) An array of optional settings; 'force' can be assigned to also verify users that do not have an account verified yet.
   * @param string  $returnType For convenience a return type has been added. You can decide between 'json' and 'raw'.
   * 
   * @throws InvalidArgumentException If arguments are invalid.
   * 
   * @access public
   * @return mixed If successful, true is returned, otherwise an array of errors is returned.
   */
  public function verifyToken($userID, $token, $settings = array(), $returnType = 'raw')
  {		
    if(! $userID) {
      self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->verifyToken) email is missing.');
      throw new InvalidArgumentException('(CTFA_SAMP->verifyToken) userID is missing.');
    }
    	
    if(! $token) {
      self::logAction(reportingLevel::ERROR, '(CTFA_SAMP->verifyToken) email is missing.');
      throw new InvalidArgumentException('(CTFA_SAMP->verifyToken) token is missing.');
    }
    
    if(! $settings || ! array_key_exists("force", $settings)) {
      $settings["force"] = 'true';
    }
    	
    $authyLibrary = new Authy_Api($this->__API, $this->__connectionURL);
    $requestResult = $authyLibrary->verifyToken(intval($userID), intval($token), $settings);

    $result = null;
    if($requestResult->ok()) {
      self::logAction(reportingLevel::NOTICE, "(CTFA_SAMP->verifyToken) token verified successfully [userid: {$userID} - token: {$token}]");
      $result['result'] = 'success'; 
    } else {
      foreach($requestResult->errors() as $field => $message) {
        self::logAction(reportingLevel::ERROR, "(CTFA_SAMP->verifyToken) {$field} = {$message}");
      }
      $result = $requestResult->errors();
    }
    
    if ($returnType === 'json') {
      if (version_compare(PHP_VERSION, '5.4.0', '>=')) {
        return json_encode($result, JSON_PRETTY_PRINT);
      }
      return json_encode($result);
    } else {
      return $result;
    }
  }

  /**
   * The logAction method is an internal function that logs errors and notices in a file.
   * 
   * @param integer $reportingLevel The reporting level required to log an action.
   * @param string $text The text that needs to be logged.
   * 
   * @return boolean If successful, true is returned, otherwise false.
   */
  private static function logAction($reportingLevel, $text) {
    if (! $text) {
      return false;
    }	  
    
    if($reportingLevel > self::$__fileDebugging) {
      return false;
    }
    
    $time = date('r', time());
    $IP = self::$__userIP;

    switch($reportingLevel) {
      case reportingLevel::NOTICE: {
        file_put_contents(self::$__logFile, "[NOTICE - {$time} - {$IP}] {$text}\r\n", FILE_APPEND);
        return true;
      }				
      case reportingLevel::ERROR: {
        file_put_contents(self::$__logFile, "[ERROR - {$time} - {$IP}] {$text}\r\n", FILE_APPEND);
        return true;
      }				
    }
    
    return false;
  }
}