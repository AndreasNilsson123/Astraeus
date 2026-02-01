plugins {
    java
    application
}

application {
    // Change to your generator entrypoint
    mainClass.set("com.astraeus.codegen.Main")
}

val generatedDir = layout.buildDirectory.dir("generated/sources/ffm/main")

tasks.register<JavaExec>("generateBindings") {
    group = "codegen"
    description = "Generates FFM bindings / other sources for the frontend."

    dependsOn(tasks.named("classes"))

    // Run the generator from this module
    classpath = sourceSets["main"].runtimeClasspath
    mainClass.set(application.mainClass)

    // Give your generator an output directory (adjust args to your generator contract)
    args(generatedDir.get().asFile.absolutePath)

    // Make Gradle understand inputs/outputs for up-to-date checks
    outputs.dir(generatedDir)
}
