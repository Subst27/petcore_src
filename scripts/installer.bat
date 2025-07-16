@echo off
rem сразу в UTF8 уйдем
chcp 65001
Setlocal EnableDelayedExpansion

set root_path=%cd%
set wix_path=d:\programs\wix
set install_path=e:\qtdistr\petcore_install
rem set distrib_path
set project_path=d:\qtprojects\petcore

set product_name=PetCore
set /p product_version= <!project_path!\!product_name!_version

rem для x64 и x86 вариантов два раза одно и тоже с разными исходниками и разными результатами. Потому цикл
for %%v in (x64) do ( rem x86) do (
  set variant=%%v
	echo Current toolchain for Win_!variant!
	set source_path=!project_path!\qt_win_!variant!\release\bin
  set deploy_path=!install_path!\source\!variant!
	if ERRORLEVEL 1 goto :eof
	
	copy /y !source_path!\!product_name!.exe !deploy_path!
  copy /y !project_path!\translations\!product_name!_ru.qm !deploy_path!\translations
  rem copy /y !source_path!\!product_name!_version !install_path!
  if ERRORLEVEL 1 goto :eof

	echo !product_name! !product_name!_version - !product_version!_!variant!
  if ERRORLEVEL 1 goto :eof
	
	if not exist !install_path!\generated (
    md !install_path!\generated
		)
	if ERRORLEVEL 1 goto :eof
		
	rem в параметрах: вариант, версия, пути самого скрипта и где install будет (там же рядом и deploy)
	call python3 !root_path!\msi_xml.py !product_name! !variant! !product_version! !root_path! !install_path!
  rem (call msi_xml.bat) > !install_path!\generated\!product_name!_install_!variant!.xml
	if ERRORLEVEL 1 goto :eof
  !wix_path!\candle.exe !install_path!\generated\!product_name!_install_!variant!.xml -arch !variant! -ext !wix_path!\WixUtilExtension.dll -o !install_path!\generated\!product_name!_install_!variant!.wixobj
  if ERRORLEVEL 1 goto :eof
	!wix_path!\light.exe !install_path!\generated\!product_name!_install_!variant!.wixobj -ext WixUIExtension -cultures:ru-ru -ext !wix_path!\WixUtilExtension.dll -sw1076 -pdbout !install_path!\generated\!product_name!_install_!variant!.wixpdb -o !install_path!\!product_name!_!product_version!_!variant!.msi
	if ERRORLEVEL 1 goto :eof
		
	echo Done: !install_path!\!product_name!_!product_version!_!variant!.msi
	echo.
  rem Подчистить за собой
  rem rmdir /s /q %install_path%\generated\
  cd !root_path!
	)
	
copy /y !project_path!\!product_name!_version !install_path!\!product_name!_version