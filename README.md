# ppkg
portable package manager for Unix-like system.

## Install ppkg via [HomeBrew](https://brew.sh/)
```bash
brew tap leleliu008/fpliu
brew install ppkg
```

## Install ppkg via [ppkg](https://github.com/leleliu008/ppkg)
```bash
ppkg install ppkg
```

## Build from source
|dependency|required?|purpose|
|----|---------|-------|
|[cmake](https://cmake.org/)|required |for generating `build.ninja`|
|[ninja](https://ninja-build.org/)|required |for doing jobs that read from `build.ninja`|
|[pkg-config>=0.18](https://www.freedesktop.org/wiki/Software/pkg-config/)|required|for finding libraries|
||||
|[jansson](https://github.com/akheron/jansson)|required|for parsing and creating JSON.|
|[libyaml](https://github.com/yaml/libyaml/)|required|for parsing formula files whose format is YAML.|
|[libgit2](https://libgit2.org/)|required|for updating formula repositories.|
|[libcurl](https://curl.se/)|required|for http requesting support.|
|[openssl](https://www.openssl.org/)|required|for https requesting support and SHA-256 sum checking support.|
|[libarchive](https://www.libarchive.org/)|required|for uncompressing .zip and .tar.* files.|
|[zlib](https://www.zlib.net/)|required|for compress and uncompress data.|
|[pcre2](https://www.pcre.org/)||for Regular Expressions support. only required on OpenBSD.|


**[vcpkg](https://github.com/microsoft/vcpkg)**
```bash
# install g++ curl zip unzip tar git

git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT="$PWD/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"

vcpkg install curl openssl libgit2 libarchive libyaml jansson

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build   build.d
cmake --install build.d
```

**[Ubuntu](https://ubuntu.com/)**

```bash
apt -y update
apt -y install git cmake ninja-build pkg-config gcc libcurl4 libcurl4-openssl-dev libgit2-dev libarchive-dev libyaml-dev libjansson-dev zlib1g-dev

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[Fedora](https://getfedora.org/)**

```bash
dnf -y update
dnf -y install git cmake ninja-build pkg-config gcc libcurl-devel libgit2-devel libarchive-devel libyaml-devel jansson-devel zlib-devel

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[ArchLinux](https://archlinux.org/)**

```bash
pacman -Syyuu --noconfirm
pacman -S     --noconfirm git cmake ninja pkg-config gcc curl openssl libgit2 libarchive libyaml jansson zlib

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[AlpineLinux](https://www.alpinelinux.org/)**

```bash
apk add git cmake ninja pkgconf gcc libc-dev curl-dev openssl-dev libgit2-dev libarchive-dev yaml-dev jansson-dev zlib-dev

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[VoidLinux](https://voidlinux.org/)**

```bash
xbps-install -Suy xbps
xbps-install -Suy cmake ninja gcc pkg-config libcurl-devel libgit2-devel libarchive-devel libyaml-devel jansson-devel zlib-devel

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[Gentoo Linux](https://www.gentoo.org/)**

```bash
emerge dev-vcs/git cmake dev-util/ninja gcc pkg-config net-misc/curl dev-libs/libgit2 libarchive dev-libs/libyaml dev-libs/jansson dev-libs/zlib

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[openSUSE](https://www.opensuse.org/)**

```bash
zypper update  -y  
zypper install -y git cmake ninja gcc pkg-config libcurl-devel libgit2-devel libarchive-devel libyaml-devel libjansson-devel zlib-devel

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[macOS](https://www.apple.com/macos/)**

```bash
brew update
brew install git cmake pkg-config ninja curl jansson libyaml libgit2 libarchive zlib

git clone https://github.com/leleliu008/ppkg
cd ppkg

export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/opt/openssl@1.1/lib/pkgconfig:/usr/local/opt/curl/lib/pkgconfig:/usr/local/opt/libarchive/lib/pkgconfig"

CMAKE_EXE_LINKER_FLAGS='-L/usr/local/lib -L/usr/local/opt/openssl@1.1/lib -lssl -liconv -framework CoreFoundation -framework Security'
CMAKE_FIND_ROOT_PATH="$(brew --prefix openssl@1.1);$(brew --prefix curl);$(brew --prefix libarchive)"

cmake \
    -S . \
    -B build.d \
    -G Ninja \
    -DCMAKE_INSTALL_PREFIX=./output \
    -DCMAKE_EXE_LINKER_FLAGS="$CMAKE_EXE_LINKER_FLAGS" \
    -DCMAKE_FIND_ROOT_PATH="$CMAKE_FIND_ROOT_PATH"

cmake --build   build.d
cmake --install build.d
```

**[FreeBSD](https://www.freebsd.org/)**

```bash
pkg install -y git cmake ninja pkgconf gcc curl openssl libgit2 libarchive libyaml jansson zlib

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[OpenBSD](https://www.openbsd.org/)**

```bash
pkg_add git cmake ninja pkgconf llvm curl libgit2 libarchive libyaml jansson zlib

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[NetBSD](https://www.netbsd.org/)**

```bash
pkgin -y install git mozilla-rootcerts cmake ninja-build pkg-config clang curl openssl libgit2 libarchive libyaml jansson zlib

mozilla-rootcerts install

git clone https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```


## ~/.ppkg
all relevant dirs and files are located in `~/.ppkg` directory.


## ppkg command usage
*   **show help of this command**
        
        ppkg -h
        ppkg --help
        
*   **show version of this command**

        ppkg -V
        ppkg --version
        
*   **show current machine os info**

        ppkg env
        
*   **integrate `zsh-completion` script**

        ppkg integrate zsh
        ppkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
        ppkg integrate zsh -v
        
    I have provide a zsh-completion script for `ppkg`. when you've typed `ppkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`

*   **update formula repositories**

        ppkg update
        
*   **search packages**
        
        ppkg search curl
        ppkg search lib
        
*   **show infomation of the given package**
        
        ppkg info curl
        ppkg info curl version
        ppkg info curl summary
        ppkg info curl web-url
        ppkg info curl git-url
        ppkg info curl git-sha
        ppkg info curl git-ref
        ppkg info curl src-url
        ppkg info curl src-sha

        ppkg info curl formula-path

        ppkg info curl formula-json
        ppkg info curl formula-yaml

        ppkg info curl formula-json | jq .
        ppkg info curl formula-yaml | yq .

        ppkg info curl receipt-path

        ppkg info curl receipt-json
        ppkg info curl receipt-yaml

        ppkg info curl receipt-json | jq .
        ppkg info curl receipt-yaml | yq .

        ppkg info curl installed-dir
        ppkg info curl installed-files
        ppkg info curl installed-timestamp-unix
        ppkg info curl installed-timestamp-iso-8601
        ppkg info curl installed-timestamp-rfc-3339
        ppkg info curl installed-timestamp-iso-8601-utc
        ppkg info curl installed-timestamp-rfc-3339-utc
        ppkg info curl installed-version

        ppkg info @all
        
*   **show packages that are depended by the given package**
        
        ppkg depends curl

        ppkg depends curl -t dot
        ppkg depends curl -t box
        ppkg depends curl -t png
        ppkg depends curl -t svg

        ppkg depends curl -t dot > xx.dot
        ppkg depends curl -t box > xx.txt
        ppkg depends curl -t png > xx.png
        ppkg depends curl -t svg > xx.svg
        
*   **download formula resources of the given package to the local cache**
        
        ppkg fetch curl
        ppkg fetch @all

        ppkg fetch curl -v
        ppkg fetch @all -v

*   **install packages**
        
        ppkg install curl
        ppkg install curl bzip2 -v
        
*   **reinstall packages**
        
        ppkg reinstall curl
        ppkg reinstall curl bzip2 -v
        
*   **uninstall packages**

        ppkg uninstall curl
        ppkg uninstall curl bzip2 -v
        
*   **upgrade the outdated packages**

        ppkg upgrade
        ppkg upgrade curl
        ppkg upgrade curl bzip2 -v
        
*   **upgrade this software**

        ppkg upgrade-self
        ppkg upgrade-self -v
        
*   **list the avaliable formula repositories**

        ppkg formula-repo-list

*   **add a new formula repository**

        ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo
        ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo master
        ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo main
        
    **Note:**
    - this software supports multiple formula repositories.
    - offical formula repository is https://github.com/leleliu008/ppkg-formula-repository-offical-core

*   **delete a existing formula repository**

        ppkg formula-repo-del my_repo

*   **list the available packages**
        
        ppkg ls-available
        
*   **list the installed packages**
        
        ppkg ls-installed
        
*   **list the outdated packages**
        
        ppkg ls-outdated
        
*   **is the given package available ?**
        
        ppkg is-available curl
        
*   **is the given package installed ?**
        
        ppkg is-installed curl
        
*   **is the given package outdated ?**
        
        ppkg is-outdated  curl
        
*   **list files of the given installed package in a tree-like format**
        
        ppkg tree curl
        ppkg tree curl -L 3
        
*   **show logs of the given installed package**
        
        ppkg logs curl
        
*   **pack the given installed package**
        
        ppkg pack curl
        ppkg pack curl --type=tar.xz
        ppkg pack curl --type=tar.gz
        ppkg pack curl --type=tar.lz
        ppkg pack curl --type=tar.bz2
        ppkg pack curl --type=zip
        
*   **cleanup the unused cached files**
        
        ppkg cleanup
        

## influential environment variables

*   **HOME**

    this environment variable must be set.

    this environment variable already have been set on most systems, if not set or set a empty string, you will receive an error message.

*   **PATH**

    some features rely on this environment variable.

    this environment variable already have been set on most systems, if not set or set a empty string, you will receive an error message.


*   **SSL_CERT_FILE**

    ```bash
    curl -LO https://curl.se/ca/cacert.pem
    export SSL_CERT_FILE="$PWD/cacert.pem"
    ```

    In general, you don't need to set this environment variable, but, if you encounter the reporting `the SSL certificate is invalid`, trying to run above commands in your terminal will do the trick.

*   **PPKG_URL_TRANSFORM**

    ```bash
    export PPKG_URL_TRANSFORM=/path/of/url-transform
    ```

    `/path/of/url-transform` command would be invoked as `/path/of/url-transform <URL>`

    `/path/of/url-transform` command must output a `<URL>`

    following is a example of `/path/of/url-transform` command implementation:

    ```bash
    #!/bin/sh

    case $1 in
        *githubusercontent.com/*)
            printf 'https://ghproxy.com/%s\n' "$1"
            ;;
        https://github.com/*)
            printf 'https://ghproxy.com/%s\n' "$1"
            ;;
        '') printf '%s\n' 'url-transform <URL>, <URL> is not given.' >&2 ;;
        *)  printf '%s\n' "$1"
    esac
    ```

    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.

*   **CMake relevant environment variables**

    [Reference](https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html)

*   **Cargo relevant environment variables**

    [Reference](https://doc.rust-lang.org/cargo/reference/environment-variables.html)

*   **Golang relevant environment variables**

    [Reference](https://golang.org/doc/install/source#environment)

    example:

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```

## ppkg formula repository
a ppkg formula repository is a git repository.

a ppkg formula repository's location is `~/.ppkg/repos.d/${PPKGFormulaRepoName}`

a ppkg formula repository should have a config file whose location is `~/.ppkg/repos.d/${PPKGFormulaRepoName}/.ppkg-formula-repo.dat`, this file is zlib deflated, and you're able to uncompress it via `zlib-flate -uncompress < ~/.ppkg/repos.d/${PPKGFormulaRepoName}/.ppkg-formula-repo.dat`.

follwoing is `offical-core` formula repository's uncompressed config:

```
url: https://github.com/leleliu008/ppkg-formula-repository-offical-core
branch: master
pinned: no
```

If a ppkg formula repository is pinned, which means this ppkg formula repository would not be updated.

If you find that a package is not in `offical-core` formula repo yet, PR is welcomed.

**Note:** 
 - please do not directly modify the formula file. your modifications may be lost after updaing.
 - ppkg supports multiple formula repositories.

