#! bin/sh
# install.sh
# Install Canna without HaikuDepot
#
DataDir=/boot/home/config/non-packaged/data
AddonDir=/boot/home/config/non-packaged/add-ons/input_server/methods
SettingsDir=/boot/home/config/settings
JOBS=$(nproc)

jam -q -j${JOBS}

mkdir -p ${DataDir}/Canna
cp -a data/default ${DataDir}/Canna
cp -a data/dic ${DataDir}/Canna

if [ -e ${SettingsDir}/Canna ]
then
# Upgrade install. Canna don't copy data files
# if SettingsDir/Canna exists.
	cp -a data/default ${SettingsDir}/Canna
	cp -a data/dic ${SettingsDir}/Canna
fi

cp generated/distro/canna ${AddonDir}

