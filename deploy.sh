#/bin/bash

cd ./src
#git pull
qmake
make
echo Stopping hsd service...
sudo service hsd stop
sudo cp ./../bin/hsd /opt/hsd/bin/
echo File copied.
sudo service hsd start
echo Service restarted
