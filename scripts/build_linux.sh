#!/bin/bash
# Сразу переходим в UTF-8
export LANG=C.UTF-8

# Путь откуда начали и он же "путь проекта/scripts" запоминаем сразу
root_path=$(pwd)
cd ..

# Путь самого проекта
petcore_path=$(pwd)
cd scripts
echo $petcore_path

export TERM=xterm-256color
# Очистка терминала
clear

# Сначала то, что в системе
# export QTDIR=$Qt5_DIR

# set custom QTDIR if needed (QTDIR=/opt/Qt/5.15.2/gcc_6)
export QTDIR=$QT_ROOT_DIR
echo $QTDIR

# set custom compilere_path if needed
compiler_path=/usr/bin

# openssl_path=/usr/lib/x86_64-linux-gnu
linux_deploy_qt=$petcore_path/scripts/linux/linuxdeployqt/linuxdeployqt-continuous-x86_64.AppImage
app_image_tool=$petcore_path/scripts/linux/appimagetool/appimagetool-940-x86_64.AppImage

qmake_command=/home/runner/work/petcore_src/Qt/5.15.2/gcc_64/bin/qmake #/usr/bin/qmake 
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
export lr_install_path=$lr_deploy_path/build/5.15.2/linux64/release/lib

# Прописываем path, lib и include чтобы система знала сразу где искать библиотеки, заголовки и т.д. Чтобы каждый раз в параметрах не указывать
export PATH=$PATH:$QTDIR/bin:$compiler_path::$lr_install_path
export CPLUS_INCLUDE_PATH=$QTDIR/include:$lr_install_path/include
export LIBRARY_PATH=$QTDIR/lib:$lr_install_path

# собрать LimeReport
# mkdir -p $lr_build_path
cd $lr_path
$qmake_command CONFIG+=release BINARY_RESULT_DIR=$lr_deploy_path limereport.pro  -o $lr_build_path/Makefile
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

#export APPIMAGETOOL_APP_NAME=PetCore-x86_64.AppImage
export VERSION=$(cat $petcore_path/petcore_version)

# структура petcore_deploy_path, коотрую должны создать мы, остальное сделает Linuxdeployqt
#
# $petcore_deploy_path
# -- usr
# -- -- bin
# -- -- -- PetCore
# -- -- share
# -- -- -- applications
# -- -- -- -- PetCore.desktop 
# -- -- -- icons
# -- -- -- -- hicolor
# -- -- -- -- -- scalable
# -- -- -- -- -- -- apps
# -- -- -- -- -- -- -- petcore.png
# -- -- -- translations

# Создать директории для деплоя
mkdir -p $petcore_deploy_path/usr/bin/
mkdir -p $petcore_deploy_path/usr/lib/
mkdir -p $petcore_deploy_path/usr/share/translations/
mkdir -p $petcore_deploy_path/usr/share/applications/
mkdir -p $petcore_deploy_path/usr/share/icons/hicolor/scalable/apps/

mkdir -p $petcore_deploy_path/usr/share/doc/libc6/
echo > $petcore_deploy_path/usr/share/doc/libc6/copyright

cp $petcore_build_path/bin/PetCore $petcore_deploy_path/usr/bin/

cp /usr/lib/x86_64-linux-gnu/libssl.so.1.1 $petcore_deploy_path/usr/lib/
cp /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1 $petcore_deploy_path/usr/lib/

cp $root_path/linux/PetCore.desktop $petcore_deploy_path/usr/share/applications/
cp $petcore_path/res/images/petcore.png $petcore_deploy_path/usr/share/icons/hicolor/scalable/apps/
cp $petcore_path/translations/*.qm $petcore_deploy_path/usr/share/translations/

cp $petcore_path/res/images/petcore.png $petcore_deploy_path/

# Корректировать пути либ
#patchelf --force-rpath --set-rpath ./lib $petcore_deploy_path/usr/lib/libssl.so.1.1
#patchelf --force-rpath --set-rpath ./lib $petcore_deploy_path/usr/lib/libcrypto.so.1.1

# Подсказать, где искать либы
# echo $lr_install_path
# ls $lr_install_path
export LD_LIBRARY_PATH=$lr_install_path

# Создать директорию, куда сложим appImage
mkdir -p $petcore_install_path
cd $petcore_install_path

# Запустить в дире деплоя appimagetool или linuxdeployqt, для линя нет стандартного, у меня на сервере и где-то тут рядом есть
chmod +x $app_image_tool
chmod +x $linux_deploy_qt

# Версия glibc в системе
# или прямо либу посмотреть /lib/x86_64-linux-gnu/libc.so.6
# libc_string=$(ldd --version | grep 'GLIBC [0-9]\.[0-9][0-9]')
# libc_version=${libc_string: -4}
# echo libc version: $libc_version

# TODO: выцепить версию glibc в системе, и в зависимости от нее запустить ту или иную утилю для создания деплоя и AppImage
# -unsupported-allow-new-glibc -unsupported-bundle-everything -no-strip
# $linux_deploy_qt $petcore_deploy_path/usr/share/applications/PetCore.desktop -qmake=$qmake_command -always-overwrite -no-translations -no-copy-copyright-files -unsupported-allow-new-glibc -appimage

$app_image_tool deploy $petcore_deploy_path/usr/share/applications/PetCore.desktop
ARCH=x86_64 $app_image_tool $petcore_deploy_path

# TODO: сделать .AppDir с помощью $linux_deploy_qt или $app_image_tool, а упаковать $petcore_deploy_path в AppImage уже с помощью низкоуровневым appimagetool ?
[ $? -eq 0 ] || exit $?

# Скомировать файл с версией поближе к инсталлятору
cp $petcore_path/petcore_version $petcore_install_path/petcore_version

# exit 0
# Подчистить за сборкой
rm -rf $lr_build_path
rm -rf $lr_deploy_path

rm -rf $petcore_build_path
rm -rf $petcore_deploy_path

echo "Done."