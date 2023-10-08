=== HSD ===

hsd provides part of the eibd (www.auto.tuwien.ac.at/~mkoegler/index.php/eibd) TCP/IP interface to communicate with EIB/KNX bus. eibd uses the EIBNet/IP protocol, while hsd uses the KO-Gateway provided by GIRA Homeserver.
For a using program, like fhem (www.fhem.de), it is transparent if hsd or eibd is running in the background. Because of that, fhem can be configured with the well known eibd settings while using hsd.

=== Changelog ===
v0.5.5
- Fixed Bug in reading GA-XML from HS
- Using EIS info from HS to identify required DPT

v0.5.4
- Fixed bug: incoming msg. not detected correctly.
- 1st imolementation of APDU Response (small)

v0.5.3
- Fixed incomming EIB_APDU_PACKET handling

v0.5.2
- Removed length info from outgoing eibd messages
- Solved bug in loosing pointer information leading to wrong messages (e.g. 00 02 22 02 instead of 00 02 00 22 )


v0.5.1
- Redesign cont.


v0.5.0
- Redesign

v0.4.12
- Added EIB_APDU_PACKET after EIB_OPEN_T_GROUP message
- Introduced different incomming eibd connections

v0.4.11
- Added EIB_OPEN_T_GROUP message

v0.4.10
- Fixed bug in address conversion from hex to GA

v0.4.9
- Fixed bug in address conversion from HS to eibd

v0.4.8
- Fixed Bug in KNX address conversion
- Added support for temperature values (DTP 9.1)
- Improved Logging messages
- Improved translations for German
- Other bug fixes