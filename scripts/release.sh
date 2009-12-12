#! /bin/sh

#If can't bulid menu without errors, then stop
if ! scripts/build.sh; then
    echo "Error in build process, please check before building"
    exit
fi


REL_ROOT=release
REL_FULL=$REL_ROOT/local/dmenu

#Clean up host files, only leave dingoo
make -f Makefile.host clean

#copy over basic file
mkdir -p $REL_FULL
for x in changelog.txt readme.txt; do
  cp $x $REL_ROOT
done

#Copy over global assets, executables
for x in resources wallpapers dmenu*; do
    cp -r $x $REL_FULL
done

#Add default theme
mkdir -p $REL_FULL/themes/default
cp -r themes/default/* $REL_FULL/themes/default/

rm -f release-$1.zip
cd $REL_ROOT
zip -r -x"*.svn*" -x"*.thumb*" -x"*-host*" ../release-$1.zip .
cd -

