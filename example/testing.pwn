#include <a_samp>
#include <tfasamp>

main() {}

public OnGameModeInit()
{
	TFASAMP_prepareConnection("localhost/authy/", "testing", "yourapikey", "development");	
	TFASAMP_verifyToken(0, "user_id", "token");
	return 1;
}