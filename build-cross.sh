#!/bin/sh

source ./env.sh

DEBUG=${DEBUG:="false"}
CORES=${CORES:="4"}
#BUILD_DIR="$PWD/build"
#BINUTIL_VER="2.43"
#GCC_VER="14.2.0"
#GDB_VER="15.1"

TARGET=${TARGET:="x86_64-elf"}
PREFIX=${PREFIX:="${BUILD_DIR}/cc"}

BINUTIL_TAR_P=${BUILD_DIR}/cc/binutils-${BINUTIL_VER}.tar.gz
GCC_TAR_P=${BUILD_DIR}/cc/gcc-${GCC_VER}.tar.gz
GDB_TAR_P=${BUILD_DIR}/cc/gdb-${GDB_VER}.tar.gz
BINUTIL_D=${BUILD_DIR}/cc/binutils-${BINUTIL_VER}
GCC_D=${BUILD_DIR}/cc/gcc-${GCC_VER}
GDB_D=${BUILD_DIR}/cc/gdb-${GDB_VER}

function run() {
    if [[ $DEBUG = "yes" ]] || \
        [[ $DEBUG = "y" ]] || \
        [[ $DEBUG = "true" ]] || \
        [[ $DEBUG = "1" ]]; then
        $@
    else
        $@ &> /dev/null
    fi
}

function download() {
    mkdir -p build/cc
    if [ ! -e "$BINUTIL_TAR_P" ]; then
        echo "INFO: Downloading ${BINUTIL_TAR_P}"
        curl https://ftp.gnu.org/gnu/binutils/binutils-${BINUTIL_VER}.tar.gz -o ${BINUTIL_TAR_P} ${SILENCE}
    fi
    if [ ! -e "$GCC_TAR_P" ]; then
        echo "INFO: Downloading ${GCC_TAR_P}"
        curl https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-${GCC_VER}.tar.gz -o ${GCC_TAR_P} ${SILENCE}
    fi
    if [ ! -e "$GDB_TAR_P" ]; then
        echo "INFO: Downloading ${GDB_TAR_P}"
        curl https://ftp.gnu.org/gnu/gdb/gdb-${GDB_VER}.tar.gz -o ${GDB_TAR_P} ${SILENCE}
    fi
}

function extract() {
    if [ ! -e "$BINUTIL_D" ]; then
        echo "INFO: Extracting ${BINUTIL_TAR_P}"
        tar -xzf ${BINUTIL_TAR_P} -C ${BUILD_DIR}/cc
    fi
    if [ ! -e "$GCC_D" ]; then
        echo "INFO: Extracting ${GCC_TAR_P}"
        tar -xzf ${GCC_TAR_P} -C ${BUILD_DIR}/cc
    fi
    if [ ! -e "$GDB_D" ]; then
        echo "INFO: Extracting ${GDB_TAR_P}"
        tar -xzf ${GDB_TAR_P} -C ${BUILD_DIR}/cc
    fi
}

function configure() {
    if [ ! -e "${BINUTIL_D}/Makefile" ]; then
        pushd ${BINUTIL_D} &> /dev/null
            echo "INFO: Configuring binutils"
            run ./configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
        popd &> /dev/null
    fi
    if [ ! -e "${GCC_D}/Makefile" ]; then
        pushd ${GCC_D} &> /dev/null
            echo "INFO: Configuring gcc"
            mkdir -p config/i386/
            echo "MULTILIB_OPTIONS += mno-red-zone" > config/i386/t-x86_64-elf
            echo "MULTILIB_DIRNAMES += no-red-zone" >> config/i386/t-x86_64-elf
            run ./configure --target=$TARGET --prefix=$PREFIX --disable-werror
        popd &> /dev/null
    fi
    if [ ! -e "${GDB_D}/Makefile" ]; then
        pushd ${GDB_D} &> /dev/null
            echo "INFO: Configuring gdb"
            run ./configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
        popd &> /dev/null
    fi
}

function compile() {
    run make -j${CORES} -C ${BINUTIL_D}
    run make -j${CORES} -C ${GCC_D} all-gcc
    run make -j${CORES} -C ${GCC_D} all-target-libgcc
    run make -j${CORES} -C ${GDB_D} all-gdb
}

function install() {
    run make -C ${BINUTIL_D} install
    run make -C ${GCC_D} install-gcc install-target-libgcc
    run make -C ${GDB_D} install
}

function help() {
    echo "Usage: $1 [command] [-f,--force]"
    echo "Commands:"
    echo "    help                - show this text"
    echo "    help-long           - show this text, but longer"
    echo "    initial             - Initial setup of dev env, should only be ran once"
    echo "    build               - compile the cross-compiler"
    echo "    install             - install the compiled compiler"
    echo ""
    echo "Env: "
    echo "    CORES               - How many cores to use during compilation [default: $CORES]"
    echo "    DEBUG               - Set to one of: ['yes', 'y', '1', 'true'] to enable debug output [default: false]"
    echo "    PREFIX              - Set the install location of all the components [default: $PREFIX]"
    echo "    TARGET              - Set the target arch for components [default: $TARGET]"
    if [[ "$2" = "long" ]]; then
        echo ""
        echo "Component versions:"
        echo "    binutils  - $BINUTIL_VER"
        echo "    gcc       - $GCC_VER"
        echo "    gdb       - $GDB_VER"
        echo ""
        echo "Current Env:"
        echo "    DEBUG         - $DEBUG"
        echo "    CORES         - $CORES"
        echo "    BUILD_DIR     - $BUILD_DIR"
        echo "    BINUTIL_VER   - $BINUTIL_VER"
        echo "    GCC_VER       - $GCC_VER"
        echo "    GDB_VER       - $GDB_VER"
        echo "    TARGET        - $TARGET"
        echo "    PREFIX        - $PREFIX"
        echo "    BINUTIL_TAR_P - $BINUTIL_TAR_P"
        echo "    GCC_TAR_P     - $GCC_TAR_P"
        echo "    GDB_TAR_P     - $GDB_TAR_P"
        echo "    BINUTIL_D     - $BINUTIL_D"
        echo "    GCC_D         - $GCC_D"
        echo "    GDB_D         - $GDB_D"
        echo "    "
    fi
    exit 1
}

function initial_setup() {
    # TODO: Make distro agnostic
    sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev
    sudo usermod -a -G kvm ${USER}
}

function cli_build() {
    echo "INFO: starting compilation"
    if [ ! -e "${GCC_TAR_P}" ] || [ ! -e "${BINUTIL_TAR_P}" ] || [ ! -e "${GDB_TAR_P}" ] || [[ "$1" = "force" ]]; then
        download
    fi
    if [ ! -e "${GCC_D}" ] || [ ! -e "${BINUTIL_D}" ] || [ ! -e "${GDB_D}" ] || [[ "$1" = "force" ]]; then
        extract
    fi
    if [ ! -e "${GCC_D}/Makefile" ] || [ ! -e "${BINUTIL_D}/Makefile" ] || [ ! -e "${GDB_D}/Makefile" ] || [[ "$1" = "force" ]]; then
        configure
    fi
    compile
}
function main() {
    case "$2" in
        "help")
            help $1
            ;;
        "help-long")
            help $1 long
            ;;
        "initial")
            initial_setup
            ;;
        "build")
            if [[ "$3" = "-f" ]] || [[ "$2" = "-f" ]]; then
                time cli_build force
            else
                time cli_build
            fi
            ;;
        "install")
            install
            ;;
        *)
            help $0
            ;;
    esac
    echo "INFO: Done"
}; main $0 $@;
