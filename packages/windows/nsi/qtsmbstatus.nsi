; qtsmbstatus.nsi
; by Daniel ROCHER
;

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------

Name "QtSmbstatus"
OutFile "qtsmbstatus-client.2.1.3-1.exe"

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
;Installer Sections

InstType "full"
InstType "minimal"

; installe le programme
Section "!Programs and libraries (required)" SecProgram
SectionIn 1 2 RO
SetOutPath "$INSTDIR\bin"
File "bin\qtsmbstatus.exe"

File "bin\*.dll"


SectionEnd 

; installe la documentation
Section "Documentation" SecDoc
SectionIn 1
SetOutPath "$INSTDIR\doc"
File "doc\license.txt"
File "doc\readme.txt"
File "doc\readme-*.txt"
File "doc\changelog.txt"

SectionEnd 

; installe les langues disponibles
Section "Languages" SecLang
SectionIn 1
SetOutPath "$INSTDIR\tr"
File "tr\qtsmbstatus_*.qm"
SectionEnd 

; installe les racourcis
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

Delete "$INSTDIR\bin\*"

Delete "$INSTDIR\doc\*"

Delete "$INSTDIR\tr\*"

RMDir $INSTDIR\doc
RMDir $INSTDIR\bin
RMDir $INSTDIR\tr
RMDir $INSTDIR
Delete "$DESKTOP\qtsmbstatus.lnk"
Delete "$SMPROGRAMS\qtsmbstatus\*.*"
RMDir "$SMPROGRAMS\qtsmbstatus"
SectionEnd
