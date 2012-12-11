nwn2dev-public
==============

NWN2 related development work (public repository)

This repository contains supporting infrastructure for a server browser
replacement for NWN2 and NWN1.  A public web service API is provided that
enables clients (such as websites) to obtain a current list of participating
NWN2 and NWN1 player-run game servers.

The API endpoint resides at:
http://api.mst.valhallalegends.com/NWNMasterServerAPI/NWNMasterServerAPI.svc

The API WSDL resides at:
http://api.mst.valhallalegends.com/NWNMasterServerAPI/NWNMasterServerAPI.svc?wsdl

All API methods take a common "Product" string argument.  This argument is
intended to specify which game product the server list should be returned for.
Valid values include (case sensitive):

"NWN2" - Neverwinter Nights 2
"NWN1" - Neverwinter Nights 1

Servers must install xp\_bugfix 1.0.20 (or higher), for NWN2, to participate:
http://www.nwnx.org/phpBB2/viewtopic.php?t=2036

