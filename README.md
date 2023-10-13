# HSD

hsd provides part of the eibd (www.auto.tuwien.ac.at/~mkoegler/index.php/eibd) TCP/IP interface to communicate with EIB/KNX bus. eibd uses the EIBNet/IP protocol, while hsd uses the KO-Gateway provided by GIRA Homeserver.
For a using program, like fhem (www.fhem.de), it is transparent if hsd or eibd is running in the background. Because of that, fhem can be configured with the well known eibd settings while using hsd.

## Installation
The program is installed by copy installation. Create a folder of your choice at a place of your choice, e.g. `c:\hsd` or `/opt/hsd` and extract the zip-file there.
At last, there should be a folder `/bin` containing the executable hsd (linux) or hsd.exe (windows).
After the first start of the program addition folder are created:

| Location | Content |
| --- | --- |
| ./bin | Executable |
| ./etc | Configuration file, cp. Configuration |
| ./doc| Documentation |
| ./var| Log files, cp. Usage |

Dependencies:

| OS | Dependencies |
| --- | --- |
| Windows | Qt6 |
| | MinGw |
| Linux | Qt6 |

## Useage
hsd does not provide a graphical user interface. It is configured via configuration file or command line arguments:

|Argument|Functionality|
| --- | --- |
| -? |Showing help page |
| -l{x} |Setting the log level to {x }|
| | 0 (~TraceLevel) |
| | 1 (~DebugLevel) |
| | 2 (~InfoLevel, default) |
| | 3 (~WarnLevel) |
| | 4 (~ErrorLevel) |
| | 5 (~FatalLevel) |
| -v | Printing program version. |
| -c {addr} | Printing convertion of address {addr}, e.g. hsd -c 4200 returns: "KNX: 8/2/0, HEX: 4200, HS: 16896" |
| -E | Exit a running hsd instances resp. server. |

E.g. Simply starting the hsd program:
```
./hsd
```

Setting the log level to debug level:
```
./hsd -l1
```

## Configuration

### Config file
| Parameter | Description |
| --- | --- |
| HsdPort | Port, where hsd listens for eibd messages.|
| HSGwPort | GIRA Homeserver ~KO-Gateway port. |
| HSIP | IP address of GIRA Homeserver. |
| HSWebPort | Port of GIRA Homeserver web server. |
| LogLevel | Hsd log level (see Usage) |
| HSGwPass | Password of the Homeserver ~KO-Gateway.|

A valid file looks like:
```
[General]
LogLevel=4
HSIP=192.168.1.2
HsdPort=6720
HSWebPort=80
HSGwPort=7003
HSGwPass=123456
```

### fhem
Add to `fhem.cfg`
```
define KNX TUL eibd:localhost 1.1.249
```
where 1.1.249 should be replaced by the EIB/KNX adress of your choice. This address is used for the protocol but has no influence on the EIB/KNX bus. To avoid possible conflicts, the address should be unique.

## Changelog 
v0.6.0
- Upgrade to Qt6
- Removed dependency on QsLog

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
