#! bin/sh
# install.sh
# Install Canna without HaikuDepot
#
DataDir=/boot/home/config/non-packaged/data
AddonDir=/boot/home/config/non-packaged/add-ons/input_server/methods
SettingsDir=/boot/home/config/settings

if [ -e ${SettingsDir}/Canna ]
then
# Upgrade install. Canna don't copy data files
# if SettingsDir/Canna exists.
#	cp -a data/default ${SettingsDir}/Canna
#	cp -a data/dic ${SettingsDir}/Canna
alert upgrade
else
alert new
fi
