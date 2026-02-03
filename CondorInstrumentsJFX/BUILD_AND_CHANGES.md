# Build & Changes (CondorInstrumentsJFX)

Date: 2025-12-15

Summary
- Added Maven support and created an executable (shaded) JAR without modifying existing source files.

What I changed
- Added `pom.xml` with:
  - JavaFX dependencies (`javafx-controls`, `javafx-graphics`) using a platform classifier (default `win`).
  - JAXB dependencies (`jakarta.xml.bind-api`, `org.glassfish.jaxb:jaxb-runtime`).
  - `javafx-maven-plugin` configured with the detected main class `uk.co.alvagem.condorjfx.Main`.
  - `maven-shade-plugin` configured to produce a shaded executable jar with `Main-Class` set.
- Updated `run.cmd` to:
  - Build the project with Maven if the shaded jar is missing.
  - Run the shaded jar (uses `%JAVA_HOME%` if defined).

Files added/modified
- Added: `pom.xml` ([CondorInstrumentsJFX/pom.xml](CondorInstrumentsJFX/pom.xml#L1))
- Modified: `run.cmd` ([CondorInstrumentsJFX/run.cmd](CondorInstrumentsJFX/run.cmd#L1))
- Added: `BUILD_AND_CHANGES.md` (this file)

Build instructions (Windows)
1. Ensure Java 17+ and Maven are installed and on PATH. Set `JAVA_HOME` to your JDK installation.
2. From the workspace root run:

```powershell
mvn -f CondorInstrumentsJFX/pom.xml clean package
```

3. The shaded executable jar will be created at:

```
CondorInstrumentsJFX/target/condor-instruments-jfx-1.0.0-SNAPSHOT-shaded.jar
```

Run (Windows)
```bat
%JAVA_HOME%\\bin\\java -jar CondorInstrumentsJFX\\target\\condor-instruments-jfx-1.0.0-SNAPSHOT-shaded.jar --panel=combined.xml --dialog=true --diagnostics=true
```
Or simply run the included script (it will build if needed):

```bat
CondorInstrumentsJFX\\run.cmd
```

Notes / Caveats
- The `javafx.platform` property in the POM is set to `win` by default; change it to `mac` or `linux` when building on those OSes.
- I attempted to run `mvn` in this environment while creating the artifact, but `mvn` was not available on PATH here, so the shaded jar was not generated in-repo by me. You will need to run the build locally where Maven is installed.
- No source files were modified.

Next steps (optional)
- Produce a self-contained native image using `jpackage` or `jlink` for distribution.
- Add an assembly/installer if you want a single desktop installer.

