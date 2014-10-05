#! bin/sh

DataDir=/boot/home/config/non-packaged/data
AddonDir=/boot/home/config/non-packaged/add-ons/input_server/methods

jam -q -j4

mkdir -p ${DataDir}/Canna
cp -a data/default ${DataDir}/Canna
cp -a data/dic ${DataDir}/Canna
cp generated/distro/canna ${AddonDir}

