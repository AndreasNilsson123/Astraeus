import org.gradle.api.plugins.JavaPluginExtension
import org.gradle.jvm.toolchain.JavaLanguageVersion
import org.gradle.api.tasks.compile.JavaCompile
import org.gradle.api.tasks.testing.Test
import org.gradle.api.tasks.JavaExec

// (optional) you can remove the plugins{} block entirely in the root build.
// If you keep it, it must ONLY contain plugin declarations, nothing else.
plugins { }

allprojects {
    group = "com.astraeus"
    version = "0.1.0"

    repositories {
        mavenCentral()
    }
}

subprojects {
    // Only configure toolchain when the Java plugin is actually applied
    pluginManager.withPlugin("java") {
        extensions.configure<JavaPluginExtension> {
            toolchain {
                languageVersion.set(JavaLanguageVersion.of(25))
            }
        }
    }

    // Preview flags (safe even if a project doesn't have these tasks)
    tasks.withType<JavaCompile>().configureEach {
        options.encoding = "UTF-8"
        options.compilerArgs.add("--enable-preview")
    }

    tasks.withType<Test>().configureEach {
        jvmArgs("--enable-preview")
    }

    tasks.withType<JavaExec>().configureEach {
        jvmArgs("--enable-preview")
    }
}
