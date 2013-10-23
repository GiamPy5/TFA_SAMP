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

if(!isset($_GET['email']))
	die("'email' is missing.");
	
if(!isset($_GET['cellphone']))
	die("'cellphone' is missing.");
	
if(!isset($_GET['area_code']))
	die("'area_code' is missing.");
	
$newUser = $TFA_SAMP['authy_api']->registerUser($_GET['email'], $_GET['cellphone'], $_GET['area_code']);

if($newUser->ok()) 
{
	echo $newUser->id();
	exit;
}
else
{
	foreach($verification->errors() as $field => $message) {
		die("$field = $message");
	}
}

?>