<?php

require 'settings.php';
require 'Authy/Authy.php';

if(!isset($_GET['password']) || $_GET['password'] != $TFA_SAMP['password'])
	die("The password is wrong or not valid.");
	
if(isset($_GET['api_key']))
	$TFA_SAMP['authy_api'] = (isset($_GET['development']) && $_GET['development'] == 'true') 
	? new Authy_Api($_GET['api_key'], 'http://sandbox-api.authy.com') 
	: new Authy_Api($_GET['api_key'], 'https://api.authy.com');
else
	die("'api_key' is invalid.");
if(!isset($_GET['userid']))
	die("'userid' is missing.");
	
if(!isset($_GET['token']))
	die("'token' is missing.");

$verification = (!isset($_GET['force'])) 
? $TFA_SAMP['authy_api']->verifyToken($_GET['userid'], $_GET['token'], array('force' => 'true'))
: $TFA_SAMP['authy_api']->verifyToken($_GET['userid'], $_GET['token'], array('force' => 'false'));

if($verification->ok())
	die('1');
else
{
	foreach($verification->errors() as $field => $message) {
		die("$field = $message");
	}
}

?>