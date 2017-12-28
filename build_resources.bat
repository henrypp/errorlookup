@echo off

php "..\builder\make_resource.php" ".\src\resource.hpp"
php "..\builder\make_locale.php" "Error Lookup" "errorlookup" ".\bin\i18n" ".\src\resource.hpp" ".\bin\errorlookup.lng"

pause
