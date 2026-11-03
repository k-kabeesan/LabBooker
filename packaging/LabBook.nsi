Unicode true
!include "MUI2.nsh"
!include "LogicLib.nsh"

!ifndef PACKAGE_DIR
!define PACKAGE_DIR "..\build-release\package\LabBook"
!endif
!ifndef OUTPUT_FILE
!define OUTPUT_FILE "..\release\LabBook-Setup.exe"
!endif

Name "LabBook"
OutFile "${OUTPUT_FILE}"
InstallDir "$PROGRAMFILES64\LabBook"
InstallDirRegKey HKLM "Software\LabBook" "InstallDir"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
Icon "..\resources\app.ico"
UninstallIcon "..\resources\app.ico"
BrandingText "LabBook University Laboratory Booking System"
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "LabBook application" SecMain
  SetRegView 64
  SetShellVarContext all
  SetOutPath "$INSTDIR"
  File /r "${PACKAGE_DIR}\*.*"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\LabBook"
  CreateShortcut "$SMPROGRAMS\LabBook\LabBook.lnk" "$INSTDIR\LabBook.exe" "" "$INSTDIR\LabBook.exe" 0
  CreateShortcut "$SMPROGRAMS\LabBook\Uninstall LabBook.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortcut "$DESKTOP\LabBook.lnk" "$INSTDIR\LabBook.exe" "" "$INSTDIR\LabBook.exe" 0
  WriteRegStr HKLM "Software\LabBook" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook" "DisplayName" "LabBook"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook" "DisplayIcon" "$INSTDIR\LabBook.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook" "Publisher" "Team 13"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook" "NoRepair" 1
SectionEnd

Section "Uninstall"
  SetRegView 64
  SetShellVarContext all
  Delete "$DESKTOP\LabBook.lnk"
  Delete "$SMPROGRAMS\LabBook\LabBook.lnk"
  Delete "$SMPROGRAMS\LabBook\Uninstall LabBook.lnk"
  RMDir "$SMPROGRAMS\LabBook"
  !include "uninstall-files.nsh"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LabBook"
  DeleteRegKey HKLM "Software\LabBook"
  ; The existing database in AppData is deliberately retained.
SectionEnd
