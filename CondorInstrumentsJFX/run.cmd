@echo off
setlocal
REM Resolve script directory
set SCRIPT_DIR=%~dp0
set JAR=%SCRIPT_DIR%target\condor-instruments-jfx-1.0.0-SNAPSHOT-shaded.jar

REM Determine Java executable
if defined JAVA_HOME (
	echo Using JAVA_HOME=%JAVA_HOME%
	set JAVA=%JAVA_HOME%\bin\java
) else (
	echo JAVA_HOME not set; using system Java.
	set JAVA=java
)

REM Check for JavaFX runtime location in FX_LIB environment variable
REM FX_LIB must be set externally. The script will not set FX_LIB.
if not defined FX_LIB (
    echo FX_LIB not set; ensure JavaFX runtime is available in your Java installation.
    echo Set FX_LIB to the JavaFX SDK lib folder, e.g. C:\"Program Files\"\Java\javafx-sdk-17.0.17\lib
    exit /b 1
)

echo Launching Condor Instruments JFX...
echo Using FX_LIB=%FX_LIB%

REM Use quoted values when passing module-path and jar to handle spaces in paths.
set "JAVA_CMD=%JAVA%"
set "MODULE_PATH=%FX_LIB%"
set "JAR_PATH=%JAR%"

REM --panel=front_panel.xml --dialog=true --diagnostics=true --drag=true
set OPTS=
set OPTS=%OPTS% --panel=front_panel.xml
REM set OPTS=%OPTS% --panel=rear_panel.xml
set OPTS=%OPTS% --dialog=true 
REM set OPTS=%OPTS% --diagnostics=true 
REM set OPTS=%OPTS% --drag=true
set OPTS=%OPTS% --use=1
"%JAVA_CMD%" --module-path "%MODULE_PATH%" --add-modules=javafx.controls,javafx.graphics,javafx.swing -jar "%JAR_PATH%" %OPTS%

