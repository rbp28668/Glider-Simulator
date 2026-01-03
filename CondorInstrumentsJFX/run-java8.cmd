@echo off
setlocal

set "JAVA_HOME"="C:\Program Files\Java\openlogic-openjdk-8u472-b08-windows-x64"

REM Resolve script directory
set "SCRIPT_DIR=%~dp0"
REM Default jar produced by pom-java8 (shaded classifier 'java8')
set "JAR=%SCRIPT_DIR%target\condor-instruments-jfx-java8-1.0.0-SNAPSHOT-java8.jar"
 
REM Fallback to non-shaded artifact name if present
if not exist "%JAR%" (
  set "JAR=%SCRIPT_DIR%target\condor-instruments-jfx-java8-1.0.0-SNAPSHOT.jar"
)

if not exist "%JAR%" (
  echo Shaded jar not found: "%JAR%"
  echo Build it with:
  echo    mvn -f "%SCRIPT_DIR%pom-java8.xml" clean package
  exit /b 1
)

REM Determine Java executable (prefer JAVA_HOME)
if defined JAVA_HOME (
  echo Using JAVA_HOME=%JAVA_HOME%
  set "JAVA=%JAVA_HOME%\bin\java"
) else (
  echo JAVA_HOME not set; using java on PATH.
  set "JAVA=java"
)

echo Launching Condor Instruments JFX (Java 8)...
"%JAVA%" -jar "%JAR%" %*
