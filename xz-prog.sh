#!/usr/bin/env bash
#
# Archive a target directory/file, displaying a visual progress bar.

set -euo pipefail

#--------------------#
# /* Global State */ #
#--------------------#

programname="$(basename "$0")"

COMP_LEVEL=6
VERBOSE=0
DEST=""
DELETE=0
TARGET=""

#-------------------------#
# /* Utility Functions */ #
#-------------------------#

##########################################
# Print a message (or messages) to stderr.
# Globals:
#   None
# Arguments:
#   Any number of messages
# Outputs:
#   Writes the message(s) to stderr
##########################################
err() {
    echo "fatal: $*" >&2
}

#########################################################################
# Print to stdout if (and only if) the script is running in verbose mode.
# Globals:
#   VERBOSE
# Arguments:
#   The message to print
#########################################################################
vprint() {
    if [[ "$#" -gt 0 ]]; then
        local -r message="$1"
        if [[ "$VERBOSE" -eq 1 ]]; then
            printf "%s\n" "$message"
        fi
    fi
}

###############################################################################################
# Check whether the current device/environment calls GNU getopt(1) for `getopt`.
# Globals:
#   None
# Arguments:
#   None
# Outputs:
#   Nothing if the `getopt` is getopt(1), otherwise writes an error message and non-zero exits.
###############################################################################################
check_getopt() {
    getopt -T >/dev/null 2>&1
    if [ $? -ne 4 ]; then
        err "To run this script, getopt(1) must be installed and earlier in the path than getopt(3)."
        exit 1
    fi
}

#######################################################################################
# Write the usage (help) text and exits the script.
# Globals:
#   None
# Arguments:
#   "0" for a successful exit, or anything else (including nothing) for a non-zero exit
# Outputs:
#   The usage text
#######################################################################################
usage_exit() {
    local status=1
    if [[ $# -gt 0 ]] && [[ "$1" = "0" ]]; then
        status=0
    fi

    usage
    exit "$status"
}

#-------------------------#
# /* Primary Functions */ #
#-------------------------#

######################################################
# Write the usage/help text for this script to stdout.
# Globals:
#   None
# Arguments:
#   None
######################################################
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

#########################################################################################
# Archive (using `xz`) the target, writing an (in-place) updating progress bar to stdout.
# Globals:
#   COMP_LEVEL
#   TARGET
#   ARCHIVE
# Arguments:
#   None
# Outputs:
#   Writes an in-place updating progress bar to stdout
#   Writes an error to stderr if the archiving fails
# Returns:
#   A non-zero exit if the archiving fails
#########################################################################################
archive() {
    local -r compression_level=$([ "$COMP_LEVEL" -gt 0 ] && echo "-$COMP_LEVEL" || echo "")
    tar cf - "$TARGET" | pv -s $(($(du -sk "$TARGET" | awk '{print $1}') * 1024)) | xz "$compression_level" >"$ARCHIVE"
    if [ -f "$ARCHIVE" ]; then
        err "Failed to create archive $ARCHIVE"
        exit 1
    fi
}

#####################################################################################################
# Delete the original target file/directory if in "delete" mode.
# Globals:
#   DELETE
#   TARGET
# Arguments:
#   None
# Outputs:
#   Nothing
# Returns:
#   0 if the target was successfully delete or the script is not in "delete" mode, non-zero on error.
#####################################################################################################
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
