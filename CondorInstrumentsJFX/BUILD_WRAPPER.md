# Maven Wrapper (CondorInstrumentsJFX)

I added `mvnw`, `mvnw.cmd` and `.mvn/wrapper/maven-wrapper.properties` to the project. The actual bootstrap jar `.mvn/wrapper/maven-wrapper.jar` is not included (binary) and must be provided.

Options to obtain `maven-wrapper.jar`:

- If you have Maven installed locally, run from the `CondorInstrumentsJFX` folder:

```powershell
mvn -N io.takari:maven:wrapper
```

This will generate `.mvn/wrapper/maven-wrapper.jar` and update wrapper files.

- Or download the wrapper jar directly and place it at `.mvn/wrapper/maven-wrapper.jar`:

```
https://repo.maven.apache.org/maven2/io/takari/maven-wrapper/0.5.6/maven-wrapper-0.5.6.jar
```

After `maven-wrapper.jar` is present you can build with the wrapper (no system Maven required):

```powershell
CondorInstrumentsJFX\mvnw -f CondorInstrumentsJFX\pom.xml clean package
```
