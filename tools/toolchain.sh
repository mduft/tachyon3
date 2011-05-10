#/bin/env bash

# .------------------------------------------------------.
# | tachyon toolchain bootstrap and environment scripts  |
# '------------------------------------------------------'

help="$0 usage:

   -v | --verbose          verbose output.
   -b | --build            build the tools (as opposed to setting environment only).
   -s <s> | --source=s     use 's' as source directory containing the tools sources.
   --version=<t>:<v>       use specified version 'v' for tool 't'. for example to use
                           gcc-4.5.2, give --version=gcc:4.5.2
   --configure=<t>:<f>     use configure flags 'f' for tool 't'
   -p <p> | --prefix=<p>   define the prefix where to install all the tools. defaults
                           to '\$(dirname $0)/../.package'."

# the tools array implicitly defines the build order, so be sure to keep those aligned.

tools=(
    "binutils:2.21:--target=x86_64-pc-elf"
    "gmp:5.0.2:"
    "mpfr:3.0.1:"
    "mpc:0.9:--with-gmp=\${_tt_prefix} --with-mpfr=\${_tt_prefix}"
    "gcc:4.6.0:--with-gnu-ld --with-gnu-as --with-mpfr=\${_tt_prefix} --with-gmp=\${_tt_prefix} --with-mpc=\${_tt_prefix}"
    "gdb:7.2:--target=x86_64-pc-linux-gnu --disable-werror"
    "cgdb:0.6.5:"
    "grub:1.99~rc2:"
    "xorriso:1.0.8:"
    "qemu:0.14.0:--disable-user --enable-system --enable-curses --enable-sdl --target-list=i386-softmmu,x86_64-softmmu --enable-debug"
    "bochs:2.4.6:--with-sdl --with-x11 --with-x --enable-x86-64 --enable-smp --enable-long-phy-address --enable-cpu-level=6 --enable-x2apic --enable-debugger --enable-logging --enable-vbe --enable-pci"
)

function info()  { echo ">>> " "$@"; }
function warn()  { echo "!!! " "$@"; }
function error() { echo "*** " "$@"; }
function fatal() { error "$@"; exit 1; }

function tt_exit_help() {
    echo "${help}"

    exit $1
}

function tt_split() {
    local packed="$1"

    save_IFS=$IFS
    IFS=':'

    local unpacked=( ${packed} )
    echo "${unpacked[@]}"
}

_tt_home="$(cd "$(dirname "$0")"; pwd)"
_tt_verbose=false
_tt_build=false
_tt_source="${_tt_home}/.source"
_tt_bdir="${_tt_home}/.build"
_tt_prefix="$(cd "${_tt_home}/.."; pwd)/.package"

# .------------------------------------------.
# | parses the command line parameters given |
# | to the script.                           |
# '------------------------------------------'
function tt_parse_cl() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
        "-v"|"--verbose")   _tt_verbose=true                ;;
        "-b"|"--build")     _tt_build=true                  ;;
        "-s")               shift; _tt_source="$1"          ;;
        "--source="*)       _tt_source="${1#--source=}"     ;;
        "-p")               shift; _tt_prefix="$1"          ;;
        "--prefix="*)       _tt_prefix="${1#--prefix=}"     ;;
        *)                  error "unknown argument: '$1'";
                            tt_exit_help 1                  ;;
        esac
        shift
    done
}

# .--------------------------------------------.
# | creates and changes to the build directory |
# | for the given package.                     |
# '--------------------------------------------'
function tt_build_dir() {
    :
}

# .--------------------------------------------.
# | finds the source package for the given tool|
# '--------------------------------------------'
function tt_find_source() {
    
}

_tt_cur_name=
_tt_cur_version=
_tt_cur_flags=
_tt_cur_bdir=
_tt_cur_src=

# .--------------------------------------------.
# | assures that all required information for  |
# | the current package is available.          |
# '--------------------------------------------'
function tt_assure_complete() {
    [[ -n "${_tt_cur_name}" ]] || fatal "package not complete: name missing"
    [[ -n "${_tt_cur_version}" ]] || fatal "package not complete: version missing"
    [[ -n "${_tt_cur_src}" ]] || fatal "package not complete: source missing"
}

# .--------------------------------------------.
# | parses a packages information and sets the |
# | global package information variables to    |
# | according values.                          |
# '--------------------------------------------'

function tt_use_package() {
    local pkginfo="$1"
    local sIFS=${IFS}
    IFS=':'
    local pkg=( "${pkginfo}" )
    IFS=${sIFS}

    _tt_cur_name="${pkg[0]}"
    _tt_cur_version="${pkg[1]}"
    _tt_cur_flags="${pkg[2]}"
    _tt_cur_src="$(tt_find_source)"
    _tt_cur_bdir="${_tt_bdir}/${_tt_cur_name}"

    tt_assure_complete
}

tt_parse_cl "$@"

for package in "${tools[@]}"; do
    tt_use_package "${package}"
done

