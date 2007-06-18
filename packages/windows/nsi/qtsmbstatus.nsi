; qtsmbstatus.nsi
; by Daniel Rocher
;

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------

Name "QtSmbstatus"
OutFile "qtsmbstatus-client.1.2.1-1.exe"

InstallDir "$PROGRAMFILES\qtsmbstatus"
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qtsmbstatus""UninstallString"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------

;Pages

  !insertmacro MUI_PAGE_LICENSE "doc\license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Sections

InstType "full"
InstType "minimal"

; install program
Section "!Programs and libraries (required)" SecProgram
SectionIn 1 2 RO
SetOutPath "$INSTDIR\bin"
File "bin\qtsmbstatus.exe"

File "bin\mingwm10.dll"
File "bin\libeay32.dll"
File "bin\libssl32.dll"
File "bin\Qt3Support4.dll"
File "bin\QtCore4.dll"
File "bin\QtGui4.dll"
File "bin\QtNetwork4.dll"
File "bin\QtSql4.dll"
File "bin\QtXml4.dll"


SectionEnd 

; install documentation
Section "Documentation" SecDoc
SectionIn 1
SetOutPath "$INSTDIR\doc"
File "doc\license.txt"
File "doc\readme.txt"
File "doc\readme-fr.txt"
File "doc\versions.txt"

SectionEnd 

; install languages
Section "Languages" SecLang
SectionIn 1
SetOutPath "$INSTDIR\tr"
File "tr\qtsmbstatus_*.qm"
SectionEnd 

; install shortcuts
Section "shortcuts" SecShortcuts
SectionIn 1
SetOutPath "$SMPROGRAMS\qtsmbstatus"
CreateShortCut "$SMPROGRAMS\qtsmbstatus\qtsmbstatus client.lnk""$INSTDIR\bin\qtsmbstatus.exe"
CreateShortCut "$SMPROGRAMS\qtsmbstatus\readme.lnk""$INSTDIR\doc\readme.txt"
CreateShortCut "$SMPROGRAMS\qtsmbstatus\license.lnk""$INSTDIR\doc\license.txt"
CreateShortCut "$SMPROGRAMS\qtsmbstatus\uninstall.lnk""$INSTDIR\uninst-qtsmbstatus.exe"
CreateShortCut "$DESKTOP\qtsmbstatus.lnk""$INSTDIR\bin\qtsmbstatus.exe"
SectionEnd 


Section -PostInstall 
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qtsmbstatus""DisplayName""qtsmbstatus (uninstall)"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qtsmbstatus""UninstallString"'"$INSTDIR\uninst-qtsmbstatus.exe"' 
WriteUninstaller "uninst-qtsmbstatus.exe"
WriteRegStr HKLM "Software\qtsmbstatus\settings\""path""$INSTDIR" 
SectionEnd 

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecProgram ${LANG_ENGLISH} "Install QtSmbstatus for windows in your computer. This section is required."
  LangString DESC_SecDoc ${LANG_ENGLISH} "Documentation (optionnal)."
  LangString DESC_SecLang ${LANG_ENGLISH} "to install other Languages (optionnal)."
  LangString DESC_SecShortcuts ${LANG_ENGLISH} "Install shortcuts (optionnal)."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SecProgram} $(DESC_SecProgram)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecDoc} $(DESC_SecDoc)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecLang} $(DESC_SecLang)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} $(DESC_SecShortcuts)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

Section "Uninstall"
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qtsmbstatus"
DeleteRegKey HKLM "Software\qtsmbstatus"

Delete "$INSTDIR\uninst-qtsmbstatus.exe"

Delete "$INSTDIR\bin\qtsmbstatus.exe"

Delete "$INSTDIR\bin\mingwm10.dll"
Delete "$INSTDIR\bin\libeay32.dll"
Delete "$INSTDIR\bin\libssl32.dll"
Delete "$INSTDIR\bin\Qt3Support4.dll"
Delete "$INSTDIR\bin\QtCore4.dll"
Delete "$INSTDIR\bin\QtGui4.dll"
Delete "$INSTDIR\bin\QtNetwork4.dll"
Delete "$INSTDIR\bin\QtSql4.dll"
Delete "$INSTDIR\bin\QtXml4.dll"

Delete "$INSTDIR\doc\readme.txt"
Delete "$INSTDIR\doc\readme-fr.txt"
Delete "$INSTDIR\doc\versions.txt"
Delete "$INSTDIR\doc\license.txt"

Delete "$INSTDIR\tr\qtsmbstatus_*.qm"

RMDir $INSTDIR\doc
RMDir $INSTDIR\bin
RMDir $INSTDIR\tr
RMDir $INSTDIR
Delete "$DESKTOP\qtsmbstatus.lnk"
Delete "$SMPROGRAMS\qtsmbstatus\*.*"
RMDir "$SMPROGRAMS\qtsmbstatus"
SectionEnd
