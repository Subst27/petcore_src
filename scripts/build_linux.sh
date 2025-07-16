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
# Очистка терминала
clear

# Нам нужны пути к компилятору, Qt, LibQ7Zip и т.д.
# Сначала то, что в системе
qt_path=$Qt5_DIR
export QTDIR=$Qt5_DIR

echo $Qt5_DIR
# set custom qt_path if needed
#qt_path=/opt/Qt/5.15.2/gcc_64
compiler_path=/usr/bin
# set custom compilere_path if needed
# compiler_path=
#openssl_path=/usr/lib/x86_64-linux-gnu

linux_deploy_qt=$petcore_path/scripts/linux/linuxdeployqt/linuxdeployqt-continuous-x86_64.AppImage
app_image_tool=$petcore_path/scripts/linux/appimagetool/appimagetool-911-x86_64.AppImage

qmake_command=/home/runner/work/petcore_src/Qt/5.15.2/gcc_64/bin/qmake #/usr/bin/qmake 
make_command=/usr/bin/make
cmake_command=/usr/local/bin/cmake

# Пути, где сборка и деплой самого проекта будет
export petcore_build_path=$petcore_path/build
export petcore_deploy_path=$petcore_path/deploy
export petcore_install_path=$petcore_path/install

# Пути, где либы будут собираться и инсталлиться, пока только LimeReport
export lr_path=$petcore_path/3rdparty/limereport
export lr_build_path=$lr_path/build
export lr_deploy_path=$lr_path/install

# Прописываем path, lib и include чтобы система знала сразу где искать библиотеки, заголовки и т.д. Чтобы каждый раз в параметрах не указывать
export PATH=$PATH:$qt_path/bin:$compiler_path
export CPLUS_INCLUDE_PATH=$qt_path/include
export LIBRARY_PATH=$qt_path/lib

export VERSION=$(cat $petcore_path/petcore_version)

# TODO: возможно, надо собрать LimeReport предварительно

# Собрать petcore
cd $petcore_path
$qmake_command CONFIG+=release PetCore.pro -o $petcore_build_path/Makefile
[ $? -eq 0 ] || exit $?

cd $petcore_build_path
$make_command -j4
# $make_command install -j4
[ $? -eq 0 ] || exit $?

# Версию забираем из файла vers, что лежит в корне проекта
#export petcore_version=$(cat $petcore_path/petcore_version)
#export petcore_arch=$(cat $petcore_path/arch)

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

# Создать директории для деплоя
mkdir -p $petcore_deploy_path/usr/bin/
mkdir -p $petcore_deploy_path/usr/lib/
mkdir -p $petcore_deploy_path/usr/translations/
mkdir -p $petcore_deploy_path/usr/share/applications/
mkdir -p $petcore_deploy_path/usr/share/icons/hicolor/scalable/apps/

cp $petcore_build_path/bin/PetCore $petcore_deploy_path/usr/bin/

cp /usr/lib/x86_64-linux-gnu/libssl.so.1.1 $petcore_deploy_path/usr/lib/
cp /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1 $petcore_deploy_path/usr/lib/

#cp $petcore_path/petcore_version $petcore_deploy_path/usr/bin/
cp $root_path/linux/PetCore.desktop $petcore_deploy_path/usr/share/applications/
cp $petcore_path/res/images/petcore.png $petcore_deploy_path/usr/share/icons/hicolor/scalable/apps/
cp $petcore_path/translations/*.qm $petcore_deploy_path/usr/translations

# Корректировать пути либ
#patchelf --force-rpath --set-rpath ./lib $petcore_deploy_path/usr/lib/libssl.so.1.1
#patchelf --force-rpath --set-rpath ./lib $petcore_deploy_path/usr/lib/libcrypto.so.1.1

# Подсказать, где искать либы
# export LD_LIBRARY_PATH=$lr_deploy_path/lib

# Создать директорию, куда сложим appImage
mkdir -p $petcore_install_path
cd $petcore_install_path

# Запустить в дире деплоя appimagetool или linuxdeployqt, для линя нет стандартного, у меня на сервере и где-то тут рядом есть
chmod +x $app_image_tool
# chmod +x $linux_deploy_qt

# Версия glibc в системе
#$linux_deploy_qt $petcore_deploy_path/usr/share/applications/PetCore.desktop -qmake=$qmake_command -always-overwrite -no-copy-copyright-files -no-translations -appimage
$app_image_tool deploy $petcore_deploy_path/usr/share/applications/PetCore.desktop
ARCH=x86_64 $app_image_tool $petcore_deploy_path
[ $? -eq 0 ] || exit $?
ls $petcore_install_path

# Скомировать файл с версией поближе к инсталлятору
#cp $petcore_path/petcore_version $petcore_install_path/linux-$petcore_arch-version

# exit 0
# Подчистить за сборкой
# rm -rf $lr_build_path
# rm -rf $lr_deploy_path

rm -rf $petcore_build_path
rm -rf $petcore_deploy_path

echo "Done."