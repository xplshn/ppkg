#include <time.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/sysinfo.h"
#include "core/tar.h"

#include "sha256sum.h"
#include "uppm.h"
#include "ppkg.h"

static int uppm_record_installed_files_r(const char * dirPath, const size_t offset, const int outputFD) {
    if (dirPath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (dirPath[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    size_t dirPathLength = strlen(dirPath);

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return PPKG_ERROR;
    }

    int ret;

    struct stat st;

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return PPKG_OK;
            } else {
                perror(dirPath);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathLength = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char   filePath[filePathLength];

        ret = snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (stat(filePath, &st) != 0) {
            perror(filePath);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (S_ISDIR(st.st_mode)) {
            ret = dprintf(outputFD, "d|%s/\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return PPKG_ERROR;
            }

            ret = uppm_record_installed_files_r(filePath, offset, outputFD);

            if (ret != PPKG_OK) {
                closedir(dir);
                return ret;
            }
        } else {
            ret = dprintf(outputFD, "f|%s\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return PPKG_ERROR;
            }
        }
    }
}

static int uppm_record_installed_files(const char * installedDIRPath) {
    size_t installedDIRLength = strlen(installedDIRPath);

    size_t installedManifestFilePathLength = installedDIRLength + 20U;
    char   installedManifestFilePath[installedManifestFilePathLength];

    int ret = snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s/.uppm/manifest.txt", installedDIRPath);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int fd = open(installedManifestFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(installedManifestFilePath);
        return PPKG_ERROR;
    }

    ret = uppm_record_installed_files_r(installedDIRPath, installedDIRLength + 1, fd);

    close(fd);

    return ret;
}

int uppm_install(const char * packageName, const bool verbose, const bool force) {
    UPPMFormula * formula = NULL;

    int ret = uppm_formula_lookup(packageName, &formula);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    if (formula->dep_pkg != NULL) {
        size_t depPackageNamesLength = strlen(formula->dep_pkg);

        size_t depPackageNamesCopyLength = depPackageNamesLength + 1U;
        char   depPackageNamesCopy[depPackageNamesCopyLength];
        strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

        char * depPackageNameArrayList[10];
        size_t depPackageNameArrayListSize = 0;

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            depPackageNameArrayList[depPackageNameArrayListSize] = depPackageName;
            depPackageNameArrayListSize++;
            depPackageName = strtok(NULL, " ");
        }

        for (size_t i = 0; i < depPackageNameArrayListSize; i++) {
            ret = uppm_install(depPackageNameArrayList[i], verbose, force);

            if (ret != PPKG_OK) {
                uppm_formula_free(formula);
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    char   uppmHomeDIR[PATH_MAX];
    size_t uppmHomeDIRLength;

    ret = uppm_home_dir(uppmHomeDIR, PATH_MAX, &uppmHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (!force) {
        ret = uppm_check_if_the_given_package_is_installed(packageName);

        if (ret == PPKG_OK) {
            uppm_formula_free(formula);
            fprintf(stderr, "uppm package '%s' already has been installed.\n", packageName);
            return PPKG_OK;
        }

        if (ret != PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            uppm_formula_free(formula);
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    if (verbose) {
        fprintf(stderr, "prepare to install package [%s].\n", packageName);
    }

    //////////////////////////////////////////////////////////////////////////

    pid_t pid = getpid();

    time_t ts = time(NULL);

    size_t tmpStrCapacity = strlen(formula->bin_url) + 30U;
    char   tmpStr[tmpStrCapacity];

    ret = snprintf(tmpStr, tmpStrCapacity, "%s|%ld|%d", formula->bin_url, ts, pid);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    char sessionID[65] = {0};

    ret = sha256sum_of_string(sessionID, tmpStr);

    if (ret != 0) {
        uppm_formula_free(formula);
        return PPKG_ERROR;
    }

    if (verbose) {
        printf("%d: ts = %lu\n", pid, ts);
        printf("%d: sessionID = %s\n", pid, sessionID);
        printf("%d: bin_sha   = %s\n", pid, formula->bin_sha);
    }

    //////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t downloadDIRCapacity = uppmHomeDIRLength + 11U;
    char   downloadDIR[downloadDIRCapacity];

    ret = snprintf(downloadDIR, downloadDIRCapacity, "%s/downloads", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(downloadDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(downloadDIR) != 0) {
                perror(downloadDIR);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }

            if (mkdir(downloadDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(downloadDIR);
                    uppm_formula_free(formula);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(downloadDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(downloadDIR);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    char binFileNameExtension[21] = {0};

    ret = ppkg_examine_filetype_from_url(formula->bin_url, binFileNameExtension, 20);

    if (ret != PPKG_OK) {
        uppm_formula_free(formula);
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    size_t binFileNameLength = strlen(binFileNameExtension) + 65U;
    char   binFileName[binFileNameLength];

    ret = snprintf(binFileName, binFileNameLength, "%s%s", formula->bin_sha, binFileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t binFilePathLength = downloadDIRCapacity + binFileNameLength + 1U;
    char   binFilePath[binFilePathLength];

    ret = snprintf(binFilePath, binFilePathLength, "%s/%s", downloadDIR, binFileName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    bool needFetch = true;

    if (lstat(binFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char actualSHA256SUM[65] = {0};

            if (sha256sum_of_file(actualSHA256SUM, binFilePath) != 0) {
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }

            if (strcmp(actualSHA256SUM, formula->bin_sha) == 0) {
                needFetch = false;
            }
        }
    }

    if (needFetch) {
        char * tmpFileName = sessionID;

        size_t tmpFilePathCapacity = downloadDIRCapacity + 65U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = ppkg_http_fetch_to_file(formula->bin_url, tmpFilePath, verbose, verbose);

        if (ret != PPKG_OK) {
            uppm_formula_free(formula);
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        if (sha256sum_of_file(actualSHA256SUM, tmpFilePath) != 0) {
            uppm_formula_free(formula);
            return PPKG_ERROR;
        }

        if (strcmp(actualSHA256SUM, formula->bin_sha) == 0) {
            if (rename(tmpFilePath, binFilePath) == 0) {
                printf("%s\n", binFilePath);
            } else {
                perror(binFilePath);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->bin_sha, actualSHA256SUM);
            uppm_formula_free(formula);
            return PPKG_ERROR_SHA256_MISMATCH;
        }
    } else {
        fprintf(stderr, "%s already have been fetched.\n", binFilePath);
    }

    //////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRootDIRCapacity = uppmHomeDIRLength + 21U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(packageInstalledRootDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(packageInstalledRootDIR) != 0) {
                perror(packageInstalledRootDIR);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }

            if (mkdir(packageInstalledRootDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(packageInstalledRootDIR);
                    uppm_formula_free(formula);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(packageInstalledRootDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(packageInstalledRootDIR);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRealDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char   packageInstalledRealDIR[packageInstalledRealDIRCapacity];

    ret = snprintf(packageInstalledRealDIR, packageInstalledRealDIRCapacity, "%s/%s", packageInstalledRootDIR, sessionID);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(packageInstalledRealDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ppkg_rm_rf(packageInstalledRealDIR, false, verbose);

            if (ret != PPKG_OK) {
                uppm_formula_free(formula);
                return ret;
            }

            if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
                perror(packageInstalledRealDIR);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }
        } else {
            if (unlink(packageInstalledRealDIR) != 0) {
                perror(packageInstalledRealDIR);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }

            if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
                perror(packageInstalledRealDIR);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }
        }
    }

    if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
        perror(packageInstalledRealDIR);
        uppm_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    if (strcmp(binFileNameExtension, ".zip") == 0 ||
        strcmp(binFileNameExtension, ".tgz") == 0 ||
        strcmp(binFileNameExtension, ".txz") == 0 ||
        strcmp(binFileNameExtension, ".tlz") == 0 ||
        strcmp(binFileNameExtension, ".tbz2") == 0) {

        if (formula->unpackd == NULL) {
            ret = tar_extract(packageInstalledRealDIR, binFilePath, ARCHIVE_EXTRACT_TIME, verbose, 1);
        } else {
            size_t extractDIRCapacity = packageInstalledRealDIRCapacity + strlen(formula->unpackd) + 1U;
            char   extractDIR[extractDIRCapacity];

            ret = snprintf(extractDIR, extractDIRCapacity, "%s/%s", packageInstalledRealDIR, formula->unpackd);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = tar_extract(extractDIR, binFilePath, ARCHIVE_EXTRACT_TIME, verbose, 1);
        }

        if (ret != 0) {
            uppm_formula_free(formula);
            return abs(ret) + PPKG_ERROR_ARCHIVE_BASE;
        }
    } else {
        size_t toFilePathCapacity = packageInstalledRealDIRCapacity + 66U;
        char   toFilePath[toFilePathCapacity];

        ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", packageInstalledRealDIR, sessionID);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = ppkg_copy_file(binFilePath, toFilePath);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    if (chdir(packageInstalledRealDIR) != 0) {
        perror(packageInstalledRealDIR);
        uppm_formula_free(formula);
        return PPKG_ERROR;
    }

    if (formula->install != NULL) {
        SysInfo sysinfo = {0};

        if (sysinfo_make(&sysinfo) != 0) {
            perror(NULL);
            uppm_formula_free(formula);
            return PPKG_ERROR;
        }

        const char * libcName;

        switch(sysinfo.libc) {
            case 1:  libcName = (char*)"glibc"; break;
            case 2:  libcName = (char*)"musl";  break;
            default: libcName = (char*)"";
        }

        size_t packageInstalledLinkDIRCapacity = packageInstalledRootDIRCapacity + strlen(packageName) + 2U;
        char   packageInstalledLinkDIR[packageInstalledLinkDIRCapacity];

        ret = snprintf(packageInstalledLinkDIR, packageInstalledLinkDIRCapacity, "%s/%s", packageInstalledRootDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        size_t shellCodeCapacity = strlen(formula->install) + 2048U;
        char   shellCode[shellCodeCapacity];

        ret = snprintf(shellCode, shellCodeCapacity,
                "set -ex\n\n"
                "NATIVE_OS_KIND='%s'\n"
                "NATIVE_OS_TYPE='%s'\n"
                "NATIVE_OS_NAME='%s'\n"
                "NATIVE_OS_VERS='%s'\n"
                "NATIVE_OS_LIBC='%s'\n"
                "NATIVE_OS_ARCH='%s'\n"
                "NATIVE_OS_NCPU='%u'\n\n"
                "PPKG_VERSION='%s'\n"
                "PPKG_VERSION_MAJOR='%d'\n"
                "PPKG_VERSION_MINOR='%d'\n"
                "PPKG_VERSION_PATCH='%d'\n"
                "PPKG_HOME='%s'\n\n"
                "PPKG_EXECUTABLE='%s'\n\n"
                "PKG_SUMMARY='%s'\n"
                "PKG_WEBPAGE='%s'\n"
                "PKG_VERSION='%s'\n"
                "PKG_BIN_URL='%s'\n"
                "PKG_BIN_SHA='%s'\n"
                "PKG_DEP_PKG='%s'\n"
                "PKG_BIN_FILENAME='%s'\n"
                "PKG_BIN_FILETYPE='%s'\n"
                "PKG_BIN_FILEPATH='%s'\n"
                "PKG_INSTALL_DIR='%s'\n\n"
                "for item in $PKG_DEP_PKG\n"
                "do\n"
                "if [ -d \"$PPKG_HOME/installed/$item/bin\" ] ; then\n"
                "PATH=\"$PPKG_HOME/installed/$item/bin:$PATH\"\n"
                "fi\n"
                "done\n\n"
                "pwd\n"
                "%s",
                sysinfo.kind,
                sysinfo.type,
                sysinfo.name,
                sysinfo.vers,
                libcName,
                sysinfo.arch,
                sysinfo.ncpu,
                PPKG_VERSION,
                PPKG_VERSION_MAJOR,
                PPKG_VERSION_MINOR,
                PPKG_VERSION_PATCH,
                uppmHomeDIR,
                "",
                formula->summary == NULL ? "" : formula->summary,
                formula->webpage == NULL ? "" : formula->webpage,
                formula->version == NULL ? "" : formula->version,
                formula->bin_url == NULL ? "" : formula->bin_url,
                formula->bin_sha == NULL ? "" : formula->bin_sha,
                formula->dep_pkg == NULL ? "" : formula->dep_pkg,
                binFileName,
                binFileNameExtension,
                binFilePath,
                packageInstalledLinkDIR,
                formula->install);

        if (ret < 0) {
            perror(NULL);
            sysinfo_free(&sysinfo);
            return PPKG_ERROR;
        }

        sysinfo_free(&sysinfo);

        printf("run shell code:\n%s\n", shellCode);

        pid_t pid = fork();

        if (pid == -1) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (pid == 0) {
            execl("/bin/sh", "/bin/sh", "-c", shellCode);
            perror("/bin/sh");
            exit(255);
        } else {
            int childProcessExitStatusCode;

            if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                perror(NULL);
                uppm_formula_free(formula);
                return PPKG_ERROR;
            }

            if (childProcessExitStatusCode != 0) {
                if (WIFEXITED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running shell code exit with status code: %d\n", WEXITSTATUS(childProcessExitStatusCode));
                } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running shell code killed by signal: %d\n", WTERMSIG(childProcessExitStatusCode));
                } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running shell code stopped by signal: %d\n", WSTOPSIG(childProcessExitStatusCode));
                }

                uppm_formula_free(formula);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    size_t packageInstalledMetaInfoDIRCapacity = packageInstalledRealDIRCapacity + 6U;
    char   packageInstalledMetaInfoDIR[packageInstalledMetaInfoDIRCapacity];

    ret = snprintf(packageInstalledMetaInfoDIR, packageInstalledMetaInfoDIRCapacity, "%s/.uppm", packageInstalledRealDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (mkdir(packageInstalledMetaInfoDIR, S_IRWXU) != 0) {
        perror(packageInstalledMetaInfoDIR);
        uppm_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    ret = uppm_record_installed_files(packageInstalledRealDIR);

    if (ret != PPKG_OK) {
        uppm_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    size_t receiptFilePathCapacity = packageInstalledMetaInfoDIRCapacity + 12U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/receipt.yml", packageInstalledMetaInfoDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int receiptFD = open(receiptFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (receiptFD == -1) {
        perror(receiptFilePath);
        uppm_formula_free(formula);
        return PPKG_ERROR;
    }

    int formulaFD = open(formula->path, O_RDONLY);

    if (formulaFD == -1) {
        perror(formula->path);
        uppm_formula_free(formula);
        close(receiptFD);
        return PPKG_ERROR;
    }

    uppm_formula_free(formula);
    formula = NULL;

    dprintf(receiptFD, "pkgname: %s\n", packageName);

    char buf[1024];

    for (;;) {
        ssize_t readSize = read(formulaFD, buf, 1024);

        if (readSize == -1) {
            perror(NULL);
            close(formulaFD);
            close(receiptFD);
            return PPKG_ERROR;
        }

        if (readSize == 0) {
            close(formulaFD);
            break;
        }

        ssize_t writeSize = write(receiptFD, buf, readSize);

        if (writeSize == -1) {
            perror(receiptFilePath);
            close(formulaFD);
            close(receiptFD);
            return PPKG_ERROR;
        }

        if (writeSize != readSize) {
            fprintf(stderr, "not fully written to %s\n", receiptFilePath);
            close(formulaFD);
            close(receiptFD);
            return PPKG_ERROR;
        }
    }

    dprintf(receiptFD, "\nsignature: %s\ntimestamp: %lu\n", PPKG_VERSION, ts);

    close(receiptFD);

    if (chdir (packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
        return PPKG_ERROR;
    }

    for (;;) {
        if (symlink(sessionID, packageName) == 0) {
            fprintf(stderr, "%s package was successfully installed.\n", packageName);
            return PPKG_OK;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ppkg_rm_rf(packageName, false, verbose);

                        if (ret != PPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return PPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageInstalledRealDIR);
                return PPKG_ERROR;
            }
        }
    }
}

int uppm_install_the_given_packages(const char * packageNames[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        const char * packageName = packageNames[i];

        UPPMFormula * formula = NULL;

        int ret = uppm_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }
    }
    return 0;
}