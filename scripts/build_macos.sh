#!/bin/bash
# Сразу переходим в UTF-8
export LANG=C.UTF-8

# Путь откуда начали и он же "путь проекта/scripts" запоминаем сразу
root_path=$(pwd)
cd ..

# Путь самого проекта
petcore_path=$(pwd)
cd scripts

export TERM=xterm-256color
# Очистка терминала
clear

# Сначала то, что в системе
# export QTDIR=$Qt5_DIR

# set custom QTDIR if needed (QTDIR=/opt/Qt/5.15.2/gcc_6)
export QTDIR=$QT_ROOT_DIR
echo $QTDIR

# set custom compilere_path if needed (compiler_path=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin)
compiler_path=/usr/bin

openssl_path=/usr/local/ssl
mac_deploy_qt=$QTDIR/bin/macdeployqt

qmake_command=$QTDIR/bin/qmake
make_command=/usr/bin/make
cmake_command=/usr/local/bin/cmake

# Пути, где сборка и деплой самого проекта будет
export petcore_build_path=$petcore_path/build
export petcore_deploy_path=$petcore_path/deploy
export petcore_install_path=$petcore_path/install

# Пути, где либы будут собираться и инсталлиться, пока только LimeReport
export lr_path=$petcore_path/3rdparty/LimeReport
export lr_build_path=$lr_path/build
export lr_deploy_path=$lr_path/install

# Куда LimeReport пихает сами либы и хедеры
export lr_install_path=$lr_deploy_path/build/5.15.2/macx/release/lib

# Прописываем path, lib и include чтобы система знала сразу где искать библиотеки, заголовки и т.д. Чтобы каждый раз в параметрах не указывать
export PATH=$PATH:$QTDIR/bin:$compiler_path:$openssl_path/bin:$lr_install_path
export CPLUS_INCLUDE_PATH=$QTDIR/include:$lr_install_path/include
export LIBRARY_PATH=$QTDIR/lib:$lr_install_path

# пропатчить Qt https://github.com/crystalidea/qt-build-tools/commit/134b1cc523bf266c7a2cd00e8902268537406f1c
cp -R -f macos/toolchain.prf $QTDIR/mkspecs/features/

# собрать LimeReport
#mkdir -p $lr_build_path
cd $lr_path
$qmake_command CONFIG+=release BINARY_RESULT_DIR=$lr_deploy_path limereport.pro -o $lr_build_path/Makefile
[ $? -eq 0 ] || exit $?

cd $lr_build_path
$make_command -j4
# $make_command install -j4
[ $? -eq 0 ] || exit $?

# Собрать PetCore
cd $petcore_path
$qmake_command CONFIG+=release PetCore.pro -o $petcore_build_path/Makefile
[ $? -eq 0 ] || exit $?

cd $petcore_build_path
$make_command -j4
# $make_command install -j4
[ $? -eq 0 ] || exit $?

#export APPIMAGETOOL_APP_NAME=PetCore-x86_64.dmg
export VERSION=$(cat $petcore_path/petcore_version)

# структура petcore_deploy_path, которую должны сделать мы, остальное сделает macdeployqt
#
# $petcore_deploy_path 
# -- PetCore.App
# -- -- Contents
# -- -- -- MacOs
# -- -- -- -- PetCore
# -- -- -- Frameworks
# -- -- -- Resources
# -- -- -- -- translations
# -- -- -- Info.plist

# Создать директории для либ, которые ручками кидать буду
mkdir -p $petcore_deploy_path/PetCore.app/Contents/MacOs
mkdir -p $petcore_deploy_path/PetCore.app/Contents/Frameworks
mkdir -p $petcore_deploy_path/PetCore.app/Contents/Resources/translations

# Скоипровать весь .app поближе, в $petcore_deploy_path
cp -R -f $petcore_build_path/bin/PetCore.app $petcore_deploy_path/
cp -f $petcore_path/translations/*.qm $petcore_deploy_path/PetCore.app/Contents/Resources/translations/

# скопировать LimeReport
cp -f $lr_install_path/liblimereport.dylib $petcore_deploy_path/PetCore.app/Contents/Frameworks/

# Корректировать путь libQtZint.dylib для liblimereport.dylib
install_name_tool -change libQtZint.dylib @executable_path/../Frameworks/libQtZint.dylib $petcore_deploy_path/PetCore.app/Contents/Frameworks/liblimereport.dylib

# скопировать QtZint
cp -f $lr_install_path/libQtZint.dylib $petcore_deploy_path/PetCore.app/Contents/Frameworks/

# Корректировать пути либ для самого PetCore
#echo "correct libs path"
install_name_tool -change liblimereport.dylib @executable_path/../Frameworks/liblimereport.dylib $petcore_deploy_path/PetCore.app/Contents/MacOs/PetCore
install_name_tool -change libQtZint.dylib @executable_path/../Frameworks/libQtZint.dylib $petcore_deploy_path/PetCore.app/Contents/MacOs/PetCore
                          
# Сам macdeployqt
cd $petcore_deploy_path
$mac_deploy_qt PetCore.app -qmldir=$petcore_deploy_path/PetCore.app/Contents/Resources/translations -libpath=$petcore_deploy_path/PetCore.app/Contents/Frameworks -always-overwrite #-dmg

# Заменить Info.plist на свой, необязательно в принципе
cp -f $root_path/macos/Info.plist $petcore_deploy_path/PetCore.app/Contents/

# Инсталлировать create-img
brew install create-dmg
[ $? -eq 0 ] || exit $?

# Переменные для отправки параметрами в create-dmg
export app_name=PetCore
export dmg_name=$app_name-$VERSION.dmg

# Создать сам .dmg

# Вариант с использованием make_dmg - при необходимости раскомментировать
#chmod +x $root_path/macos/make_dmg.sh
#$root_path/macos/make_dmg.sh -b $petcore_path/res/background.png
#                             -I 80 -i $petcore_path/res/petcore.icns
#                              -s "640:360" -c 400:155:130:160 
#                              -d $petcore_deploy_path/$app_name.app -n $dmg_name -N $app_name

# Вариант с использованием create_dmg - при необходимости закомментировать
create-dmg --volname $app_name --volicon $root_path/images/petcore.icns \
           --filesystem APFS \
           --background $root_path/images/arrow.png \
           --window-pos 200 120 --window-size 520 360 \
           --icon-size 80 --icon $app_name.app 130 160 \
           --hide-extension $app_name.app \
           --app-drop-link 400 155 $dmg_name $petcore_deploy_path/$app_name.app
[ $? -eq 0 ] || exit $?

# Дебужная инфа, нужна для понимания, что не так с либами - раскомментировать при необходимости
#echo "libs for PetCore"
#otool -L $petcore_deploy_path/PetCore.app/Contents/MacOS/PetCore

#echo "libs for Lime Report"
#otool -L $petcore_deploy_path/PetCore.app/Contents/Frameworks/limereport.dylib

# Создать директорию, куда сложим .dmg
mkdir -p $petcore_install_path
mv -f $petcore_deploy_path/$dmg_name $petcore_install_path/
#cp -f $petcore_path/petcore_version $petcore_install_path/macos-$petcore_arch-version

cd $petcore_install_path

# exit 0
# Подчистить за сборкой
rm -rf $lr_build_path
rm -rf $lr_deploy_path

rm -rf $petcore_build_path
rm -rf $petcorer_deploy_path

echo "Done."