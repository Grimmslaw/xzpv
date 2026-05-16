#!/usr/bin/env bash

set -euo pipefail

#--------------#
# Global State #
#--------------#

programname="$(basename "$0")"

COMP_LEVEL=6
VERBOSE=0
DEST=""
DELETE=0
TARGET=""

#-------------------#
# Utility Functions #
#-------------------#

vprint() {
    if [[ "$#" -gt 0 ]]; then
        local -r message="$1"
        if [[ "$VERBOSE" -eq 1 ]]; then
            printf "%s\n" "$message"
        fi
    fi
}

check_getopt() {
    getopt -T >/dev/null 2>&1
    if [ $? -ne 4 ]; then
        echo "To run this script, getopt(1) must be installed and earlier in the path that getopt(3)."
        exit 1
    fi
}

usage_exit() {
    local status=1
    if [[ $# -gt 0 ]] && [[ "$1" = "0" ]]; then
        status=0
    fi

    usage
    exit "$status"
}

#-------------------#
# Primary Functions #
#-------------------#

usage() {
    cat <<EOF
Usage: $programname [options] <target>

Options:
  -c <level>, --compression <level>     Level of compression (see \`man xz\`) to be used to archive the files.
  -d <dest>, --destination <dest>       Path (or name relative to cwd) of the archive file.
  -v, --verbose                         Verbose output.
  -x, --delete                          Delete the source directory/files after archiving.
EOF
}

archive() {
    local -r compression_level=$([ "$COMP_LEVEL" -gt 0 ] && echo "-$COMP_LEVEL" || echo "")
    tar cf - "$TARGET" | pv -s $(($(du -sk "$TARGET" | awk '{print $1}') * 1024)) | xz "$compression_level" >"$ARCHIVE"
    if [ -f "$ARCHIVE" ]; then
        echo "Failed to create archive $ARCHIVE"
        exit 1
    fi
}

cleanup() {
    if [[ $DELETE -eq 1 ]]; then
        rm -rf "$TARGET"
    fi
}

#----------------#
# Parse Args ==> #
#----------------#

SHORT_OPTS="hc:d:vx"
LONG_OPTS="help,compression:,destination:,verbose,delete"
INITIAL_PARSED=$(getopt -o "$SHORT_OPTS" --long "$LONG_OPTS" -n "$programname" -- "$@")
if [ $? -ne 0 ]; then
    vprint "Error parsing option."
    usage_exit 1
fi

eval set -- "$INITIAL_PARSED"

while true; do
    case "$1" in
        -h|--help)
            usage_exit 0
            ;;
        -c|--compression)
            if [[ ("$2" -lt 1) || ("$2" -gt 9) ]]; then
                echo "Error: Argument for option -$1 must be between 1 and 9, inclusive."
                usage_exit 1
            fi

            COMP_LEVEL="$2"
            shift 2
            ;;
        -d|--destination)
            DEST="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -x|--delete)
            DELETE=1
            shift
            ;;
        --)
            shift;
            break;
            ;;
        \?)
            echo "Error: Option -$1 requires an argument."
            usage_exit 1
            ;;
        *)
            echo "Error: Unknown option -$1."
            usage_exit 1
            ;;
    esac
done

if [[ "$#" -lt 1 ]]; then
    echo "A <target> positional parameter is required."
    usage_exit 1
elif [[ "$#" -gt 1 ]]; then
    echo "Ignoring extraneous arguments: " "${@:2}"
fi

TARGET="${1}"
ARCHIVE="$DEST".tar.xz

#--------------#
# Program Flow #
#--------------#

check_getopt
archive
cleanup
