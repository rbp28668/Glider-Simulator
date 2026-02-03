# CondorInstrumentsJFX

This module builds and runs the JavaFX-based Condor instruments UI.

Prerequisites
- JDK 17 or later installed (set `JAVA_HOME` is recommended).
- Apache Maven installed and `mvn` available on PATH.
- JavaFX SDK installed when running with the system JDK (see `FX_LIB` below).

Build
1. From the workspace root run:

```powershell
mvn -f CondorInstrumentsJFX/pom.xml clean package
```

2. This produces the shaded JAR:

```
CondorInstrumentsJFX/target/condor-instruments-jfx-1.0.0-SNAPSHOT-shaded.jar
```

Run (wrapper script)
- The project includes a wrapper script `run.cmd` in the `CondorInstrumentsJFX` folder. That script requires `FX_LIB` to be set externally (it will not set `FX_LIB` for you).

FX_LIB (JavaFX runtime)
- Set `FX_LIB` externally to point to the JavaFX SDK `lib` folder before launching `run.cmd`. Examples below show how to set the environment variable for the current shell only.

PowerShell example:

```powershell
$env:FX_LIB='C:\Program Files\Java\javafx-sdk-17.0.17\lib'
Set-Location CondorInstrumentsJFX
.\run.cmd
```

CMD example:

```cmd
cd /d CondorInstrumentsJFX
set "FX_LIB=C:\Program Files\Java\javafx-sdk-17.0.17\lib"
run.cmd
```

Notes
- `run.cmd` will use `%JAVA_HOME%\bin\java` if `JAVA_HOME` is set; otherwise it falls back to `java` on PATH.
- The script passes `--module-path "%FX_LIB%" --add-modules=javafx.controls,javafx.graphics,javafx.swing` to the JVM. Use the examples above to avoid issues with spaces in paths.
- The shaded JAR produced by the POM intentionally excludes JavaFX modules so JavaFX is loaded from the module path at runtime.

Troubleshooting
- If you see errors about missing JavaFX runtime, confirm `FX_LIB` points at the SDK `lib` directory for a JavaFX SDK matching your Java version.
- If `mvn` is not found, install Maven and add it to PATH.

Contact
- If you want a change to how `run.cmd` behaves (for example detecting common FX locations automatically), open an issue or request the change.
# CondorInstrumentsJFX

Minimal README: build, run, and notes for the JavaFX-based Condor instruments UI.

Prerequisites
- JDK 17 or later installed and `JAVA_HOME` set.
- Apache Maven installed and `mvn` on PATH.

Build
1. From the workspace root run:

```powershell
mvn -f CondorInstrumentsJFX/pom.xml clean package
```

2. This produces the shaded executable JAR:

```
CondorInstrumentsJFX/target/condor-instruments-jfx-1.0.0-SNAPSHOT-shaded.jar
```

Run
- Run the jar directly (recommended):

```bat
%JAVA_HOME%\bin\java -jar CondorInstrumentsJFX\target\condor-instruments-jfx-1.0.0-SNAPSHOT-shaded.jar --panel=combined.xml --dialog=true --diagnostics=true
```

- Or use the provided wrapper script `run.cmd` in `CondorInstrumentsJFX`. The script will attempt to build the project with Maven if the shaded jar is missing, then run it.

FX_LIB (JavaFX runtime)
- If you installed a JavaFX SDK, set `FX_LIB` to the SDK `lib` folder and run the wrapper so Java launches with the JavaFX module path:

```powershell
$env:FX_LIB='C:\path\to\javafx-sdk-17\lib'
& CondorInstrumentsJFX\run.cmd
```

Or in cmd.exe:

```bat
set FX_LIB=C:\path\to\javafx-sdk-17\lib
CondorInstrumentsJFX\run.cmd
```

When `FX_LIB` is set the script launches Java with `--module-path` and `--add-modules=javafx.controls,javafx.graphics,javafx.swing`. If `FX_LIB` is not set the script will attempt to run the jar normally and may fail with "JavaFX runtime components are missing" if JavaFX modules are not available on the runtime.

What I changed
- Added `pom.xml` with JavaFX and JAXB dependencies, `javafx-maven-plugin` configured with `uk.co.alvagem.condorjfx.Main`, and `maven-shade-plugin` to produce an executable JAR.
- Updated `run.cmd` to build (if needed) and run the shaded jar.
- Added `BUILD_AND_CHANGES.md` summarizing edits.

Platform notes
- The POM uses a JavaFX classifier property `<javafx.platform>` set to `win` by default. Change it to `mac` or `linux` before building on those OSes.

Troubleshooting
- If `mvn` is not found: install Maven and add it to PATH.
- If JavaFX runtime errors occur, ensure you are using a matching Java version (JDK 17+) and the correct `javafx.platform` classifier.
- If `JAVA_HOME` is unset, `run.cmd` will fall back to `java` on PATH.

Next steps (optional)
- Create a native installer with `jpackage` or `jlink` for easier distribution.
