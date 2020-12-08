@echo off

php "..\builder\make_resource.php" ".\src\resource.h"
php "..\builder\make_locale.php" "Error Lookup" "errorlookup" ".\bin\i18n" ".\src\resource.h" ".\src\resource.rc" ".\bin\errorlookup.lng"
copy /y ".\bin\errorlookup.lng" ".\bin\32\errorlookup.lng"
copy /y ".\bin\errorlookup.lng" ".\bin\64\errorlookup.lng"

pause
