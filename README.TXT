c0ck bl0cker by qwikrazor87

Here is a simple plugin to prevent those pesky little kids from messing with your Vita.
This plugin starts with a specified game/app and doesn't start the game/app until
a new password is created or the password entered matches.
I had CMA in mind when making this plugin, to prevent kids from messing with the Vita's precious files.
But this plugin also works with other games/apps.
I tested it on the Settings app but it doesn't not show up on screen, but it does run in the background.
I still haven't figured out how to fix it, so I'll leave it for now.
Also, it doesn't work on all games, certain ones will prevent the password file from being opened, not sure why.

Thanks to everyone involved in the community.
I hope you enjoy this more than I did making it. :D

Instructions:
	copy c0ck_bl0cker.suprx here:
		ux0:tai/c0ck_bl0cker.suprx

	update ux0:tai/config.txt, for example (repeat for each game/app):
		*NPXS10026 #CMA ID
		ux0:tai/c0ck_bl0cker.suprx

		*NPXS10015 #Settings ID
		ux0:tai/c0ck_bl0cker.suprx

	Start molecularShell, press Start and update taihen config.txt in menu.
	now launch game/app and get c0ck bl0cked. :P

Changelog:
v1.1 (2017-01-06):
	Password matched even when only using the first password key.
	Fixed bad programming on password check, fixed.
v1 (2017-01-06):
	Initial release.
