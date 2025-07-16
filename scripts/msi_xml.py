#!/usr/bin/python3

import os
import argparse
import hashlib
import uuid

from xml.etree import ElementTree
from xml.dom import minidom

def _escape_cdata(text, encoding = "utf-8"):
    try:
        if "&" in text:
            text = text.replace("&", "&amp;")
        # if "<" in text:
            # text = text.replace("<", "&lt;")
        # if ">" in text:
            # text = text.replace(">", "&gt;")
        return text
    except TypeError:
        raise TypeError("cannot serialize %r (type %s)" % (text, type(text).__name__))
        
def custom_install(Product):
    # Кстомизация стандартного набора диалогов
    UI = ElementTree.SubElement(Product, "UI", {"Id": "WixUI_InstallDir"})
    ElementTree.SubElement(UI, "TextStyle", {"Id": "WixUI_Font_Normal", "FaceName": "Tahoma", "Size": "8"})
    ElementTree.SubElement(UI, "TextStyle", {"Id": "WixUI_Font_Bigger", "FaceName": "Tahoma", "Size": "12"})
    ElementTree.SubElement(UI, "TextStyle", {"Id": "WixUI_Font_Title", "FaceName": "Tahoma", "Size": "9", "Bold": "yes"})

    ElementTree.SubElement(UI, "Property", {"Id": "DefaultUIFont", "Value": "WixUI_Font_Normal"})
    ElementTree.SubElement(UI, "Property", {"Id": "WixUI_Mode", "Value": "InstallDir"})

    ElementTree.SubElement(UI, "DialogRef", {"Id": "BrowseDlg"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "DiskCostDlg"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "ErrorDlg"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "FatalError"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "FilesInUse"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "MsiRMFilesInUse"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "PrepareDlg"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "ProgressDlg"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "ResumeDlg"})
    ElementTree.SubElement(UI, "DialogRef", {"Id": "UserExit"})

    # CDATA запихать получилось перегрузкой _escape_cdata (смотри выше)
    ElementTree.SubElement(UI, "Publish", {"Dialog": "BrowseDlg", "Control": "OK", "Event": "DoAction", "Value": "WixUIValidatePath", "Order": "3"}).text = "1"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "BrowseDlg", "Control": "OK", "Event": "SpawnDialog", "Value": "InvalidDirDlg", "Order": "4"}).text = "<![CDATA[NOT WIXUI_DONTVALIDATEPATH AND WIXUI_INSTALLDIR_VALID<>\"1\"]]>"

    ElementTree.SubElement(UI, "Publish", {"Dialog": "ExitDialog", "Control": "Finish", "Event": "EndDialog", "Value": "Return", "Order": "999"}).text = "1"

    ElementTree.SubElement(UI, "Publish", {"Dialog": "WelcomeDlg", "Control": "Next", "Event": "NewDialog", "Value": "InstallDirDlg"}).text = "NOT Installed"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "WelcomeDlg", "Control": "Next", "Event": "NewDialog", "Value": "VerifyReadyDlg"}).text = "Installed AND PATCH"

    # ElementTree.SubElement(UI, "Publish", {"Dialog": "LicenseAgreementDlg", "Control": "Back", "Event": "NewDialog", "Value": "WelcomeDlg"}).text = "1"
    # ElementTree.SubElement(UI, "Publish", {"Dialog": "LicenseAgreementDlg", "Control": "Next", "Event": "NewDialog", "Value": "InstallDirDlg"}).text = "LicenseAccepted = \"1\""

    ElementTree.SubElement(UI, "Publish", {"Dialog": "InstallDirDlg", "Control": "Back", "Event": "NewDialog", "Value": "WelcomeDlg"}).text = "1"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "InstallDirDlg", "Control": "Next", "Event": "SetTargetPath", "Value": "[WIXUI_INSTALLDIR]", "Order": "1"}).text = "1"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "InstallDirDlg", "Control": "Next", "Event": "DoAction", "Value": "WixUIValidatePath", "Order": "2"}).text = "NOT WIXUI_DONTVALIDATEPATH"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "InstallDirDlg", "Control": "Next", "Event": "SpawnDialog", "Value": "InvalidDirDlg", "Order": "3"}).text = "<![CDATA[NOT WIXUI_DONTVALIDATEPATH AND WIXUI_INSTALLDIR_VALID<>\"1\"]]>"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "InstallDirDlg", "Control": "Next", "Event": "NewDialog", "Value": "VerifyReadyDlg", "Order": "4"}).text = "WIXUI_DONTVALIDATEPATH OR WIXUI_INSTALLDIR_VALID=\"1\""
    ElementTree.SubElement(UI, "Publish", {"Dialog": "InstallDirDlg", "Control": "ChangeFolder", "Property": "_BrowseProperty", "Value": "[WIXUI_INSTALLDIR]", "Order": "1"}).text = "1"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "InstallDirDlg", "Control": "ChangeFolder", "Event": "SpawnDialog", "Value": "BrowseDlg", "Order": "2"}).text = "1"

    ElementTree.SubElement(UI, "Publish", {"Dialog": "VerifyReadyDlg", "Control": "Back", "Event": "NewDialog", "Value": "InstallDirDlg", "Order": "1"}).text = "NOT Installed"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "VerifyReadyDlg", "Control": "Back", "Event": "NewDialog", "Value": "MaintenanceTypeDlg", "Order": "2"}).text = "Installed AND NOT PATCH"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "VerifyReadyDlg", "Control": "Back", "Event": "NewDialog", "Value": "WelcomeDlg", "Order": "2"}).text = "Installed AND PATCH"

    ElementTree.SubElement(UI, "Publish", {"Dialog": "MaintenanceWelcomeDlg", "Control": "Next", "Event": "NewDialog", "Value": "MaintenanceTypeDlg"}).text = "1"

    ElementTree.SubElement(UI, "Publish", {"Dialog": "MaintenanceTypeDlg", "Control": "RepairButton", "Event": "NewDialog", "Value": "VerifyReadyDlg"}).text = "1"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "MaintenanceTypeDlg", "Control": "RemoveButton", "Event": "NewDialog", "Value": "VerifyReadyDlg"}).text = "1"
    ElementTree.SubElement(UI, "Publish", {"Dialog": "MaintenanceTypeDlg", "Control": "Back", "Event": "NewDialog", "Value": "MaintenanceWelcomeDlg"}).text = "1"

    ElementTree.SubElement(UI, "Property", {"Id": "ARPNOMODIFY", "Value": "1"})

    ElementTree.SubElement(Product, "Property", {"Id": "WIXUI_INSTALLDIR", "Value": "INSTALL_DIRECTORY"})
    ElementTree.SubElement(Product, "UIRef", {"Id": "WixUI_Common"})

    return
        
def prettify(element):
    rough_string = ElementTree.tostring(element, "utf-8")
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")
    
def form_file_guid(GUID, count):
    count_string = "000" + str(count)
    count_string = count_string[len(count_string) - 4:]
    return (GUID[0:19] + count_string + GUID[len(GUID) - 13:])
    
def correct_file_id(directory_id, file_id):
    file_id = file_id.replace(" ", "_").replace("-", "_").replace("+", "_")
    return (directory_id + "_" + file_id)
    
def create_component(product_name, directory, component_id, component_guid, deploy_path, dir_name, files):
    Component = ElementTree.SubElement(directory, "Component", {"Id": component_id, "Guid": component_guid})

    for file_name in files:        
        full_file_name = os.path.join(dir_name, file_name)
        file_path = full_file_name.replace("/", "\\") # винда же все таки

        file_id = product_name + "_" + full_file_name.replace(deploy_path + os.sep, "")
        file_id = file_id.replace("/", "_").replace("\\", "_").replace(" ", "_").replace("-", "_").replace("+", "_") # убрать "левые" символы
        
        ElementTree.SubElement(Component, "File", {"Id": file_id, "Name": file_name, "Source": file_path, "DiskId": "1", "Checksum": "yes"})
    return

def main():
    ElementTree._escape_cdata = _escape_cdata
    
    parser = argparse.ArgumentParser()
    parser.add_argument("product_name", help = "Product name")
    parser.add_argument("variant", help = "Build variant {x86, x64}")
    parser.add_argument("version", help = "Product version")
    parser.add_argument("root_path", help = "Path with msi_xml.py")
    parser.add_argument("install_path", help = "Root path to installation")
    args = parser.parse_args()
    
    # из основного скрипта летят: название продукта, вариант (x86, x64), версия и пути
    product_name = args.product_name
    product_md5 = hashlib.md5(product_name.encode("utf-8"))
    GUID = str(uuid.UUID(product_md5.hexdigest())).upper()
    
    variant = args.variant
    version = args.version
    
    # эта иконка только для отображения в Программы и компоненты.
    icon_path = args.root_path + "\\images\\" + product_name.lower() + ".ico"
    bmp_path = args.root_path + "\\images\\"
    
    install_path = args.install_path
    deploy_path=install_path.replace("\\", "/") + "/source/" + variant
    print("\nProduct: " + product_name + " " + version + " " + variant + "\nInstall path: " + install_path + "\nDeploy path: " + deploy_path + "\n")
   
    # мне из версии надо получить такую фигню 1.2.3.4 => 01020304, предполагается ограничение 99 в каждой части версии 
    version_parts = version.split(".")
    for i in range(0, len(version_parts)):
        if len(version_parts[i]) < 2:
            version_parts[i] = "0" + version_parts[i]
    
    while len(version_parts) < 4:
        version_parts.append("00")
        
    # Эта часть будет использоваться для поиска установленного product_name
    product_guid = GUID[0:28]
    # А это по сути полный GUID с учетом версии
    version_guid = product_guid + "".join(version_parts) 
    
    Wix = ElementTree.Element("Wix", {"xmlns": "http://schemas.microsoft.com/wix/2006/wi", "xmlns:util": "http://schemas.microsoft.com/wix/UtilExtension"})
    Product = ElementTree.SubElement(Wix, "Product", {"Id": version_guid, "Name": product_name, "Language": "1049", "Codepage": "65001", "Version": version, "Manufacturer": "Den P. Classen", "UpgradeCode": GUID})
    Package = ElementTree.SubElement(Product, "Package", {"Id": "*", "Description": product_name + " installer package", "Comments": "Comment for ...", "Manufacturer": "Den P. Classen", 
                                                          "Platform": variant, "InstallerVersion": "300", "Compressed": "yes", "InstallScope": "perMachine"})

    ElementTree.SubElement(Product, "Icon", {"Id": product_name.lower() + ".ico", "SourceFile": icon_path})
    ElementTree.SubElement(Product, "Property", {"Id": "ARPPRODUCTICON", "Value": product_name.lower() + ".ico"})
    ElementTree.SubElement(Product, "Media", {"Id": "1", "Cabinet": product_name + ".cab", "EmbedCab": "yes"})
    
    ElementTree.SubElement(Product, "MajorUpgrade", {"AllowDowngrades": "yes", "Schedule": "afterInstallInitialize"})
    # про запуск программы после установки
    
    ElementTree.SubElement(Product, "CustomAction", {"Id": "LunchProgram", "Directory": "INSTALL_DIRECTORY", "Impersonate": "yes", "Execute": "immediate", "Return": "asyncNoWait", "ExeCommand": "[INSTALL_DIRECTORY]" + product_name + ".exe"})
    Sequence = ElementTree.SubElement(Product,"InstallExecuteSequence")
    ElementTree.SubElement(Sequence, "Custom", {"Action": "LunchProgram", "After": "InstallFinalize"}).text = "UPGRADINGPRODUCTCODE" # запускать только при апгрейде
    '''
    Property Name  Installed  REINSTALL  UPGRADINGPRODUCTCODE  REMOVE

    Install        False      False      False                 False
    Uninstall      True       False      False                 True
    Change         True       False      False                 False
    Repair         True       True       False                 False
    Upgrade        True       False      True                  True
    '''
    
    element = ElementTree.SubElement(Product, "Property", {"Id": "PartiallyGuid"}).text = product_guid
    element = ElementTree.SubElement(Product, "Property",  {"Id": "FullGuid"}).text = version_guid
    
    ElementTree.SubElement(Product, "WixVariable", {"Id": "WixUIDialogBmp", "Value": bmp_path + "background.bmp"}) # 493 × 312
    ElementTree.SubElement(Product, "WixVariable", {"Id": "WixUIBannerBmp", "Value": bmp_path + "banner.bmp"}) # 493 × 58
    
    # Кастомная последовательсноть диалогов
    custom_install(Product)

    #ElementTree.SubElement(Product, "SetDirectory", {"Id": "WINDOWSVOLUME", "Value": "[WindowsVolume]"})
    if variant == 'x64':
        ElementTree.SubElement(Product, "SetDirectory", {"Id": "ProgramFiles", "Value": "[ProgramFiles64Folder]"})
    else:
        ElementTree.SubElement(Product, "SetDirectory", {"Id": "ProgramFiles", "Value": "[ProgramFilesFolder]"})
    
    TargetDir = ElementTree.SubElement(Product, "Directory", {"Id": "TARGETDIR", "Name": "SourceDir"})
    # со старта будет предлагаться поставить в Program Files/product_name
    ProgramsDir = ElementTree.SubElement(TargetDir, "Directory", {"Id": "ProgramFiles", "Name": "ProgramFiles"})
    directory = ElementTree.SubElement(ProgramsDir, "Directory", {"Id": "INSTALL_DIRECTORY", "Name": product_name})

    # Просто использование одного из стандартных набора диалогов
    '''
    ElementTree.SubElement(Product, "UIRef", {"Id": "WixUI_InstallDir"})
    ElementTree.SubElement(Product, "WixVariable", {"Id": "WixUILicenseRtf", "Value": install_path + "\\source\\license.rtf"})
    ElementTree.SubElement(Product, "WixVariable", {"Id": "WixUIBannerBmp", "Value": "..."}) # 493 × 58
    ElementTree.SubElement(Product, "WixVariable", {"Id": "WixUIDialogBmp", "Value": "..."}) # 493 × 312
    '''
    # Расположение просто в корне C
    '''
    ElementTree.SubElement(Product, "SetDirectory", {"Id": "WINDOWSVOLUME", "Value": "[WindowsVolume]"})

    TargetDir = ElementTree.SubElement(Product, "Directory", {"Id": "TARGETDIR", "Name": "SourceDir"})
    ProgramsDir = ElementTree.SubElement(TargetDir, "Directory", {"Id": "WINDOWSVOLUME", "Name": "ProgramFiles"})
    directory = ElementTree.SubElement(ProgramsDir, "Directory", {"Id": "INSTALL_DIRECTORY", "Name": product_name})
    '''
    components = [] # список компонентов для установки
    directory_path = [directory] # буду хранить ноды в порядке вложенности
    count = 0
    previous = 0
    
    # Дальше самое основное - перебор файлов и директорий
    file_tree = os.walk(deploy_path)
    for file_tuple in file_tree:
        dir_name=file_tuple[0]
        level = dir_name.count(os.sep)
        files = file_tuple[2]
        
        partially_name = dir_name.replace(deploy_path,"")
        directory_id = product_name + partially_name.replace(os.sep, "_")
        name = dir_name.split(os.sep)[-1]
        
        component_id = directory_id + "_" +str(count)
        components.append(component_id)
        component_guid = form_file_guid(GUID, count)
        count += 1
        
        if level == 0:
            create_component(product_name, directory, component_id, component_guid, deploy_path, dir_name, files)
            continue
                
        directory = ElementTree.SubElement(directory_path[level-1], "Directory", {"Id": directory_id, "Name": name})
        create_component(product_name, directory, component_id, component_guid, deploy_path, dir_name, files)
        
        if level > previous:
            directory_path.append(directory)
            
        if level < previous:
            directory_path.pop()
            
        previous = level
    
    # Сделать ярлыки в Programs и на Рабочий стол
    shortcut_guid = form_file_guid(GUID, count);
    component_id = "Programs_Shortcut_"+str(count)
    components.append(component_id)
    
    ProgramsMenu = ElementTree.SubElement(TargetDir, "Directory", {"Id": "ProgramMenuFolder"})
    ApplicationProgram = ElementTree.SubElement(ProgramsMenu, "Directory", {"Id": "ApplicationProgramsFolder", "Name": product_name})
    Component = ElementTree.SubElement(ApplicationProgram, "Component", {"Id": component_id, "Guid": shortcut_guid})
    ElementTree.SubElement(Component, "Shortcut", {"Id": "Programs_Shortcut", "Name": product_name, "Description": product_name, "Target": "[INSTALL_DIRECTORY]" + product_name + ".exe", "WorkingDirectory": "INSTALL_DIRECTORY"})
    ElementTree.SubElement(Component, "Shortcut", {"Id": "Uninstall_Shortcut", "Name": "Uninstall " + product_name, "Description": "Uninstall " + product_name, "Target": "[SystemFolder]msiexec.exe", "Arguments": "/uninstall [ProductCode]"})
    ElementTree.SubElement(Component, "RemoveFolder", {"Id": "ApplicationProgramsFolder", "On": "uninstall"})
    ElementTree.SubElement(Component, "RegistryValue", {"Root": "HKCU", "Key": "Software\\WebSuccess\\" + product_name, "Name": "installed", "Type": "integer", "Value": "1", "KeyPath": "yes"})
        
    count += 1
    shortcut_guid = form_file_guid(GUID, count);
    component_id = "Desktop_Shortcut_"+str(count)
    components.append(component_id)
    
    DesktopFolder = ElementTree.SubElement(TargetDir, "Directory", {"Id": "DesktopFolder", "Name": "Desktop"})
    Component = ElementTree.SubElement(DesktopFolder, "Component", {"Id": component_id, "Guid": shortcut_guid})
    ElementTree.SubElement(Component, "Shortcut", {"Id": "Desktop_Shortcut", "Name": product_name, "Description": product_name, "Target": "[INSTALL_DIRECTORY]" + product_name + ".exe", "WorkingDirectory": "INSTALL_DIRECTORY"})
    ElementTree.SubElement(Component, "RegistryValue", {"Root": "HKCU", "Key": "Software\\WebSuccess\\" + product_name, "Name": "installed", "Type": "integer", "Value": "1", "KeyPath": "yes"})
    
    # Все components добавить в установку
    Feature = ElementTree.SubElement(Product, "Feature", {"Id": product_name, "Title": product_name, "Level": "1"})
    for component in components:
        ElementTree.SubElement(Feature, "ComponentRef", {"Id": component})
    
    # Закрыть работающее приложение при установке
    ElementTree.SubElement(Product, "util:CloseApplication", {"Id": "Close" + product_name, "Target": product_name + ".exe", "CloseMessage": "yes", "RebootPrompt": "no", "Timeout": "30"})
    
    # для красоты и удобства "посмотреть", если что-то не так
    content = prettify(Wix)

    # сохранить все в файл
    xml_file_name = install_path + "/generated/" + product_name + "_install_" + variant + ".xml"
    generated = os.path.dirname(xml_file_name)
    if not os.path.exists(generated):
        os.mkdir(generated)
    
    file = open(xml_file_name, "wb")
    file.write(str.encode(content))

if __name__ == '__main__':
    main()