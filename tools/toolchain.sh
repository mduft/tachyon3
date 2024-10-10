#/bin/env bash

# .------------------------------------------------------.
# | tachyon toolchain bootstrap and environment scripts  |
# '------------------------------------------------------'

# the tools array implicitly defines the build order, so be sure to keep those aligned.
# each entry has this format: <name>:<version>:<configure flags>:<all target>:<install target>

tools=(
    "binutils:2.40:--build=\${_tt_host} --target=x86_64-pc-elf"
    "gmp:6.2.1:--build=\${_tt_host}"
    "mpfr:4.2.0:--build=\${_tt_host}"
    "mpc:1.3.1:--build=\${_tt_host} --with-gmp=\${_tt_prefix} --with-mpfr=\${_tt_prefix}"
    "gcc:12.2.0:--build=\${_tt_host} --with-gnu-ld --with-gnu-as --with-mpfr=\${_tt_prefix} --with-gmp=\${_tt_prefix} --with-mpc=\${_tt_prefix} --target=x86_64-pc-elf --enable-languages=c,c++:all-gcc:install-gcc"
    "gdb:12.1:--build=\${_tt_host} --target=x86_64-pc-linux-gnu --disable-werror"
    "cgdb:0.8.0:--build=\${_tt_host}"
    "grub:2.06:--build=\${_tt_host} --disable-werror"
    "xorriso:1.5.4:--build=\${_tt_host}"
    "qemu:7.2.0:--disable-docs --disable-user --enable-system --enable-curses --enable-gtk --target-list=i386-softmmu,x86_64-softmmu --enable-debug"
    "bochs:2.6.10:--with-x11 --with-x --with-term --disable-docbook --enable-cdrom --enable-pci --enable-usb --enable-usb-ohci --enable-a20-pin --enable-cpu-level=6 --enable-x86-64 --enable-fpu --enable-disasm --enable-idle-hack --enable-all-optimizations --enable-repeat-speedups --enable-plugins --enable-sb16=linux --enable-ne2000 --enable-pnic --enable-smp --enable-logging"
)

sites=(
    "binutils|http://ftp.gnu.org/gnu/binutils/\${P}-\${V}.tar.bz2"
    "gmp|https://gmplib.org/download/gmp/\${P}-\${V}.tar.bz2"
    "mpfr|http://www.mpfr.org/\${P}-\${V}/\${P}-\${V}.tar.bz2"
    "mpc|http://ftp.gnu.org/gnu/mpc/\${P}-\${V}.tar.gz"
    "gcc|http://ftp.gnu.org/gnu/gcc/\${P}-\${V}/\${P}-\${V}.tar.gz"
    "gdb|http://ftp.gnu.org/gnu/gdb/\${P}-\${V}.tar.gz"
    "cgdb|http://cgdb.me/files/\${P}-\${V}.tar.gz"
    "grub|http://ftp.gnu.org/gnu/grub/\${P}-\${V}.tar.gz"
    "xorriso|http://ftp.gnu.org/gnu/xorriso/\${P}-\${V}.tar.gz"
    "qemu|https://download.qemu.org/\${P}-\${V}.tar.bz2"
    "bochs|http://downloads.sourceforge.net/project/bochs/bochs/\${V}/\${P}-\${V}.tar.gz"
)

function info()  { [[ ${_tt_verbose} == true ]] && echo ">>> " "$@"; }
function warn()  { echo "!!! " "$@"; }
function error() { echo "*** " "$@"; }
function fatal() { error "$@"; exit 1; }

function tt_exit_help() {
    echo "${help}"

    exit $1
}

_tt_home="$(cd "$(dirname "$0")"; pwd)"
_tt_verbose=false
_tt_source="${_tt_home}/.source"
_tt_bdir="${_tt_home}/.build"
_tt_prefix="$(cd "${_tt_home}/.."; pwd)/.package"
_tt_preflight=false
_tt_clean=false
_tt_download=false
_tt_jobs=1
_tt_env="${_tt_prefix}/env.sh"
_tt_mkenv="${_tt_prefix}/env.mk"
_tt_pkg=
_tt_host="$(uname -m)-pc-linux-gnu"

help="$0 usage:

   --help                  print this message.
   -v | --verbose          verbose output.
   -j <j> | --jobs=<j>     use 'j' jobs during builds.
   --download              download sources
   --clean                 remove existing build directories before building.
   --preflight             check package completeness (source availability) before
   --package=<p>           only build a single package 'p'.
   -e <e> | --env=<e>      generate the environment script to 'e' instead of the
                           default '${_tt_env}'.
                           starting the builds.
   -s <s> | --source=s     use 's' as source directory containing the tools sources.
                           defaults to '${_tt_source}'.
   -t <t> | --temp=<t>     use 't' as temporary build directory.
                           defaults to '${_tt_bdir}'.
   -p <p> | --prefix=<p>   define the prefix where to install all the tools. defaults
                           to '${_tt_prefix}'."

# .------------------------------------------.
# | parses the command line parameters given |
# | to the script.                           |
# '------------------------------------------'
function tt_parse_cl() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
        "-v"|"--verbose")   _tt_verbose=true                ;;
        "-s")               shift; _tt_source="$1"          ;;
        "--source="*)       _tt_source="${1#--source=}"     ;;
        "-p")               shift; _tt_prefix="$1"          ;;
        "--prefix="*)       _tt_prefix="${1#--prefix=}"     ;;
        "-t")               shift; _tt_bdir="$1"            ;;
        "--temp="*)         _tt_bdir="${1#--temp=}"         ;;
        "-j")               shift; _tt_jobs="$1"            ;;
        "-j"[0-9]*)         _tt_jobs="${1#-j}"              ;;
        "--jobs=")          _tt_jobs="${1#--jobs=}"         ;;
        "--preflight")      _tt_preflight=true              ;;
        "--clean")          _tt_clean=true                  ;;
        "--download")       _tt_download=true               ;;
        "--debug")          set -xv                         ;;
        "--package="*)      _tt_pkg="${1#--package=}"       ;;
        "--help")           tt_exit_help 0                  ;;
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
    local dir="${_tt_cur_bdir}"

    [[ -d ${dir} && ${_tt_clean} == false ]] && { cd ${dir}; return; }
    [[ -d ${dir} ]] && rm -rf ${dir}

    mkdir -p "${dir}"
    cd "${dir}"
}

function tt_download() {
    [[ -d "${_tt_source}" ]] || mkdir -p "${_tt_source}"

    for pkg in "${tools[@]}"; do
        local sIFS=${IFS}
        IFS=':'
        local p=( ${pkg} )
        IFS=${sIFS}

        P="${p[0]}"
        V="${p[1]}"

        _tt_cur_name="${P}"
        _tt_cur_version="${V}"

        src="$(tt_find_source)"

        [[ -n "${src}" ]] && { info "source for ${P} already present, not downloading"; continue; }

        for site in "${sites[@]}"; do
            local sIFS=${IFS}
            IFS='|'
            local s=( ${site} )
            IFS=${sIFS}

            sP="${s[0]}"
            if [[ ${sP} == ${P} ]]; then
                url="$(eval echo "${s[1]}")"
                wget -O "${_tt_source}/${url##*/}" "${url}" || rm "${_tt_source}/${url##*/}"
            fi
        done
    done
}

# .--------------------------------------------.
# | finds the source package for the given tool|
# '--------------------------------------------'
function tt_find_source() {
    for file in ${_tt_source}/${_tt_cur_name}-${_tt_cur_version}.*; do
        if [[ ${file} == *.tar.gz || ${file} == *.tar.bz2 || ${file} == *.tgz || ${file} == *.tar ]]; then
            echo "${file}"
            return
        fi
    done
}

# .--------------------------------------------.
# | unpacks the source package into the build  |
# | directory.                                 |
# '--------------------------------------------'
function tt_unpack() {
    info "unpacking ${_tt_cur_name}-${_tt_cur_version}"

    local tflags=

    case "${_tt_cur_src}" in
    *.tgz|*.tar.gz) tflags="xfz";;
    *.tar.bz2)      tflags="xfj";;
    *.tar)          tflags="xf";;
    esac

    tar ${tflags} "${_tt_cur_src}"
    cd ${_tt_cur_name}* || fatal "missing expected directory"

    for patch in "${_tt_home}/patches/${_tt_cur_name}-${_tt_cur_version}"*.patch; do
        [[ ${patch} == *'*.patch' ]] && break

        local _p=undef
        for p in 0 1 2 3 4; do
            if patch --quiet -p${p} --dry-run < "${patch}" > /dev/null 2>&1; then
                _p=${p}
                break
            fi
        done

        if [[ ${_p} == undef ]]; then
            warn "patch ${patch} does not apply!"
            continue
        fi

        info "applying ${patch}"

        patch --quiet -p${_p} < "${patch}"
    done

    cd ..
}

# .--------------------------------------------.
# | configures the current package.            |
# '--------------------------------------------'
function tt_configure() {
    info "configuring ${_tt_cur_name}-${_tt_cur_version}"

    [[ $(pwd) == ${_tt_bdir}/* ]] \
        || fatal "oups. configuring outside the build dir?"

    [[ -x ./configure ]] \
        || fatal "oups. cannot find configure script"

    case "${_tt_cur_name}" in
    bochs|qemu)
        # debugable emulator/simulator comes handy when searching
        # the source of interrupts, etc.
        CFLAGS="-g"
        CXXFLAGS="-g"
        ;;
    esac

    ./configure --prefix=${_tt_prefix} $(eval echo ${_tt_cur_flags})
}

# .--------------------------------------------.
# | builds the current package.                |
# '--------------------------------------------'
function tt_build() {
    info "building ${_tt_cur_name}-${_tt_cur_version}"
    make -j${_tt_jobs} ${_tt_cur_build_target}
}

# .--------------------------------------------.
# | installs the current package.              |
# '--------------------------------------------'
function tt_install() {
    info "installing ${_tt_cur_name}-${_tt_cur_version}"
    make -j${_tt_jobs} ${_tt_cur_install_target}
}

_tt_cur_name=
_tt_cur_version=
_tt_cur_flags=
_tt_cur_bdir=
_tt_cur_src=
_tt_cur_build_target=
_tt_cur_install_target=

# .--------------------------------------------.
# | assures that all required information for  |
# | the current package is available.          |
# '--------------------------------------------'
function tt_assure_complete() {
    [[ -n "${_tt_cur_name}" ]] || fatal "package not complete: name missing"
    [[ -n "${_tt_cur_version}" ]] || fatal "${_tt_cur_name} not complete: version missing"
    [[ -n "${_tt_cur_src}" ]] || fatal "${_tt_cur_name} not complete: source missing"

    [[ -n "${_tt_cur_install_target}" ]] || _tt_cur_install_target=install

    info "package ${_tt_cur_name} seems complete ..."
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
    local pkg=( ${pkginfo} )
    IFS=${sIFS}

    _tt_cur_name="${pkg[0]}"
    _tt_cur_version="${pkg[1]}"
    _tt_cur_flags="${pkg[2]}"
    _tt_cur_build_target="${pkg[3]}"
    _tt_cur_install_target="${pkg[4]}"
    _tt_cur_src="$(tt_find_source)"
    _tt_cur_bdir="${_tt_bdir}/${_tt_cur_name}"

    tt_assure_complete
}

tt_parse_cl "$@"
if [[ ${_tt_download} == true ]]; then
    tt_download
fi

if [[ ${_tt_preflight} == true ]]; then
    for package in "${tools[@]}"; do
        [[ -n ${_tt_pkg} && ${package} != ${_tt_pkg}* ]] && continue
        tt_use_package "${package}"
    done
fi

#
# TODO: find a better way to do this...?
#
export LDFLAGS="-L${_tt_prefix}/lib -Wl,-rpath,${_tt_prefix}/lib"
export CFLAGS="-I${_tt_prefix}/include"
export CXXFLAGS="${CFLAGS}"

for package in "${tools[@]}"; do
    [[ -n ${_tt_pkg} && ${package} != ${_tt_pkg}* ]] && continue
    tt_use_package "${package}"

    _tfbase="${_tt_cur_bdir}/"

    [[ ${_tt_clean} == true ]] && rm -rf "${_tt_cur_bdir}"

    tt_build_dir || fatal "cannot create build directory!"

    [[ -f ${_tfbase}.unpacked ]] \
        || tt_unpack || fatal "failed to unpack ${_tt_cur_name}-${_tt_cur_version}"
    cd ${_tt_cur_name}* || fatal "missing expected directory"
    touch ${_tfbase}.unpacked
    [[ -f ${_tfbase}.configured ]] \
        || tt_configure || fatal "failed to configure ${_tt_cur_name}-${_tt_cur_version}"
    touch ${_tfbase}.configured
    [[ -f ${_tfbase}.built ]] \
        || tt_build || fatal "failed to build ${_tt_cur_name}-${_tt_cur_version}"
    touch ${_tfbase}.built
    [[ -f ${_tfbase}.installed ]] \
        || tt_install || fatal "failed to install ${_tt_cur_name}-${_tt_cur_version}"
    touch ${_tfbase}.installed
done

if [[ ${_tt_clean} == true ]]; then
    info "cleaning up ..."
    [[ -n "${_tt_bdir}" && -e ${_tt_bdir} ]] \
        && rm -rf "${_tt_bdir}"
fi

info "generating environment to ${_tt_env}"
echo "export PATH=${_tt_prefix}/bin:${_tt_prefix}/sbin:\${PATH}" > "${_tt_env}"

info "generating environment to ${_tt_mkenv}"
echo "export PATH := ${_tt_prefix}/bin:${_tt_prefix}/sbin:\$(PATH)" > "${_tt_mkenv}"

